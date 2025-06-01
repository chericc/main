import json
from enum import Enum

import serial
from numpy.matlib import empty
import time

from hub import UsbSerialCmd
from mysystem import MySystem
from mysystem import PortsDevInfo
from myconfig import g_config
import copy
import os
import shutil
import logging

# 直接返回10个端口的状态，上层直接显示

class Status(Enum):
    UNKNOWN = 0
    DETECTING = 1
    REBOOTING = 2
    OK = 3
    FAILED = 4

class RunningState(Enum):
    INIT = 0
    ALLPOWERUP = 1
    HANDLEPORTS = 2

class PortStateType(Enum):
    INIT = 1
    WAITINSERT = 2
    WAITVOLUMEMOUNT = 3
    UPGRADEPREPARE = 4
    UPGRADING = 5
    CHECKVERSION = 6
    UPGRADED = 7
    ERROR = 8

class PortState:
    def __init__(self):
        self.__port_state = PortStateType.INIT
        self.__state_update_time: float = time.time()
        self.__port_info = ''
    def get_port_state(self) -> PortStateType:
        return self.__port_state
    def set_port_state(self, port_state: PortStateType, volume: str = '', info: str = ''):
        self.__port_state = port_state
        self.__state_update_time = time.time()
        self.__port_info = ''
        if len(volume):
            self.__port_info += f'{volume}'
        if len(info):
            if len(self.__port_info):
                self.__port_info += '/'
            self.__port_info += f'{info}'
    def get_port_info(self) -> str:
        return self.__port_info
    def state_update_duration(self) -> float:
        return time.time() - self.__state_update_time

class PortDeviceType(Enum):
    Camera = 1
    Screen = 2
    Unknown = 3

class PortTypeHelper:
    # volume should like: 'E:'
    def __init__(self, port_dev_type: PortDeviceType = PortDeviceType.Unknown, volume: str = None):
        self.type : PortDeviceType = port_dev_type
        self.version_file_path: str = ''
        self.update_file_dst_path: str = ''
        self.update_file_src_path: str = ''
        if self.type == PortDeviceType.Camera:
            self.version_file_path = os.path.join(volume, g_config.camera_version_file_path)
            self.update_file_dst_path = os.path.join(volume, g_config.camera_update_dst_file_path)
            self.update_file_src_path = os.path.join(volume, g_config.camera_update_file_from_path)
        elif self.type == PortDeviceType.Screen:
            self.version_file_path = os.path.join(volume, g_config.screen_version_file_path)
            self.update_file_dst_path = os.path.join(volume, g_config.screen_update_dst_file_path)
            self.update_file_src_path = os.path.join(volume, g_config.screen_update_file_from_path)
        else:
            logging.error('unknown port dev type')
    def get_version_src(self) -> str:
        try:
            content = None
            with open(file=self.version_file_path, mode='r', encoding='utf8') as f:
                content = f.read()
            if self.type == PortDeviceType.Camera:
                json_begin = content.find('{')
                json_end = content.rfind('}') + 1
                if json_begin == -1 or json_end == 0:
                    logging.error('locate json failed: %s', content)
                    return None
                json_str = content[json_begin:json_end]
                logging.debug('json: %s', json_str)
                json_obj = json.loads(json_str)
                logging.debug('json load ok')
                date = json_obj['date']
                return date
            elif self.type == PortDeviceType.Screen:
                date = content
                date = date.strip('\n')
                return date
            else:
                return ''
        except Exception as e:
            logging.error('get version failed: %s', e)
            return ''
    def get_version_dst(self) -> str:
        if self.type == PortDeviceType.Camera or self.type == PortDeviceType.Screen:
            file_name_list = self.update_file_src_path.split('.')
            if len(file_name_list) >= 3:
                return file_name_list[1]
            else:
                logging.error('invalid filename')
                return ''
        else:
            return ''
    def get_wifi_version_dst(self) -> str:
        if self.type == PortDeviceType.Camera:
            return g_config.wifi_dst_version
        elif self.type == PortDeviceType.Screen:
            return 'default'
        else:
            return ''
    def get_wifi_version_now(self) -> str:
        if self.type == PortDeviceType.Camera:
            content = None
            with open(file=self.version_file_path, mode='r', encoding='utf8') as f:
                content = f.read()
            lines = content.split('\n')
            for line in lines:
                if g_config.wifi_version_keyword in line:
                    words = line.split(':')
                    if len(words) < 2:
                        logging.error('get wifi version failed: version line invalid')
                        return ''
                    return words[1].strip()
            logging.error('get wifi version failed: version line not found')
            return ''
        elif self.type == PortDeviceType.Screen:
            logging.error('get wifi version failed: screen not support')
            return ''

class StatusChecker:
    def __init__(self, serial_name):
        self.__running_state = RunningState.INIT
        self.__serial_name = serial_name
        self.__port_ctl = UsbSerialCmd(self.__serial_name)
        self.__system = MySystem()
        self.__port_states: list[PortState] = []
        if len(self.__port_states) == 0:
            for i in range(g_config.hub_port_count):
                port_state = PortState()
                self.__port_states.append(port_state)

    def get_port_state(self):
        return copy.deepcopy(self.__port_states)
    
    def run_init(self):
        # 最开始，把所有端口上电
        self.__port_ctl.control_all_port(open=True)
        logging.debug('into AllPowerUp state')
        self.__running_state = RunningState.ALLPOWERUP
    def run_all_powerup(self):
        # 所有端口上电后，等待设备开机
        for count in range(g_config.wait_port_seconds):
            time.sleep(1)
            all_ports_info = self.__system.get_all_ports_dev_info()
            if len(all_ports_info) > 0:
                logging.debug('some port inserted')
                break
            logging.debug('wait some port insert: %s/%s', count, g_config.wait_port_seconds)
            continue
        # 有端口上线，开始处理
        logging.debug('into HandlePorts state')
        self.__running_state = RunningState.HANDLEPORTS

    def logging_ports_debug(self, state: PortState, port_info: PortsDevInfo, *args):
        msg = ''
        if args:
            msg = args[0] % args[1:]
            logging.debug('port[%s] state[%s]: %s ', port_info.port_number, \
                        state.get_port_state(), msg, stacklevel=2)
        else:
            logging.debug('port[%s] state[%s]', port_info.port_number, \
                        state.get_port_state(), stacklevel=2)
        return
    def logging_ports_info(self, state: PortState, port_info: PortDeviceType, *args):
        msg = ''
        if args:
            msg = args[0] % args[1:]
            logging.info('port[%s] state[%s]: %s ', port_info.port_number, \
                        state.get_port_state(), msg, stacklevel=2)
        else:
            logging.info('port[%s] state[%s]', port_info.port_number, \
                        state.get_port_state(), stacklevel=2)
        return
    def logging_ports_error(self, state: PortState, port_info: PortDeviceType, *args):
        msg = ''
        if args:
            msg = args[0] % args[1:]
            logging.error('port[%s] state[%s]: %s ', port_info.port_number, \
                        state.get_port_state(), msg, stacklevel=2)
        else:
            logging.error('port[%s] state[%s]', port_info.port_number, \
                        state.get_port_state(), stacklevel=2)
        return 

    def run_handle_ports(self):
        all_ports_info = self.__system.get_all_ports_dev_info()
        for i in range(g_config.hub_port_count):
            found_flag = False
            for k in all_ports_info:
                if k.port_number == (i + 1):
                    found_flag = True
                    
                    self.run_port(self.__port_states[i], k)
                    break
            if not found_flag:
                port_info = PortsDevInfo()
                port_info.port_number = i + 1
                self.run_port(self.__port_states[i], port_info)

    def run_handle_one_port_init(self, state: PortState, port_info: PortsDevInfo):
        # check if port exist
        if len(port_info.dev_id) == 0:
            state.set_port_state(PortStateType.WAITINSERT)
            self.logging_ports_debug(state, port_info, 'into state')
        elif len(port_info.volume) == 0:
            state.set_port_state(PortStateType.WAITVOLUMEMOUNT)
            self.logging_ports_debug(state, port_info, 'into state')
        else:
            state.set_port_state(PortStateType.UPGRADEPREPARE, port_info.volume)
            self.logging_ports_debug(state, port_info, 'into state')

    def run_handle_one_port_wait_insert(self, state: PortState, port_info: PortsDevInfo):
        if len(port_info.dev_id) == 0:
            self.logging_ports_debug(state, port_info, 'wait')
        else:
            state.set_port_state(PortStateType.WAITVOLUMEMOUNT)
            self.logging_ports_debug(state, port_info, 'into state')

    def run_handle_one_port_wait_volume_mount(self, state: PortState, port_info: PortsDevInfo):
        if len(port_info.volume) == 0:
            self.logging_ports_debug(state, port_info, 'wait')
        else:
            state.set_port_state(PortStateType.UPGRADEPREPARE, port_info.volume)
            self.logging_ports_debug(state, port_info, 'into state')

    def gen_port_helper(self, volume: str):
        # check version file
        port_file_helper_camera = PortTypeHelper(PortDeviceType.Camera, volume)
        port_file_helper_screen = PortTypeHelper(PortDeviceType.Screen, volume)
        if os.path.exists(port_file_helper_camera.version_file_path):
            logging.debug('camera version file detected')
            return port_file_helper_camera
        elif os.path.exists(port_file_helper_screen.version_file_path):
            logging.debug('screen version file detected')
            return port_file_helper_screen
        else:
            return None

    def check_need_update_with_version(self, version_now: str, version_dst: str):
        if g_config.update_to_bigger_version_only:
            return int(version_now) < int(version_dst)
        else:
            return int(version_now) != int(version_dst)
        
    def do_camera_upgrade_prepare(self, helper: PortTypeHelper, state: PortState, port_info: PortsDevInfo):
        # check version
        version_dst = helper.get_version_dst()
        version_now = helper.get_version_src()
        if len(version_dst) == 0 or len(version_now) == 0:
            state.set_port_state(PortStateType.ERROR, volume=port_info.volume, info = 'get version failed')
            self.logging_ports_debug(state, port_info, 'get version failed: dst=%s, now=%s', version_dst, version_now)
            return
        if self.check_need_update_with_version(version_now, version_dst):
            self.logging_ports_debug(state, port_info, 'need update: %s --> %s', version_now, version_dst)
            self.logging_ports_debug(state, port_info, 'cp ota file: %s --> %s', helper.update_file_src_path, helper.update_file_dst_path)
            shutil.copyfile(helper.update_file_src_path, helper.update_file_dst_path)
            time.sleep(0.2)
            self.logging_ports_debug(state, port_info, 'power off')
            self.__port_ctl.control_usb_port(port_info.port_number, False)
            time.sleep(0.2)
            self.logging_ports_debug(state, port_info, 'power on')
            self.__port_ctl.control_usb_port(port_info.port_number, True)
            state.set_port_state(PortStateType.UPGRADING, port_info.volume)
        else:
            state.set_port_state(PortStateType.CHECKVERSION, port_info.volume)
            self.logging_ports_debug(state, port_info, 'need not update: dst=%s, now=%s', version_dst, version_now)
        return

    def run_handle_one_port_upgrade_prepare(self, state: PortState, port_info: PortsDevInfo):
        self.logging_ports_debug(state, port_info, 'in state')
        if len(port_info.dev_id) == 0 or len(port_info.volume) == 0:
            state.set_port_state(PortStateType.INIT)
            self.logging_ports_debug(state, port_info, 'in state')
            return
        port_type_helper = self.gen_port_helper(port_info.volume)
        if port_type_helper is None:
            state.set_port_state(PortStateType.ERROR, volume=port_info.volume, info='not support device')
            self.logging_ports_debug(state, port_info, 'not support device')
            return
        else:
            self.do_camera_upgrade_prepare(port_type_helper, state, port_info)

    def run_handle_one_port_upgrading(self, state: PortState, port_info: PortsDevInfo):
        self.logging_ports_debug(state, port_info, 'in state')

        if state.state_update_duration() > g_config.maximum_upgrade_duration_sec:
            self.logging_ports_error(state, port_info, 'upgrade timeout')
            state.set_port_state(PortStateType.INIT)
            return

        if len(port_info.dev_id) == 0 or len(port_info.volume) == 0:
            self.logging_ports_error(state, port_info, 'upgrade: wait restart: %s/%s', \
                                     state.state_update_duration(), g_config.maximum_upgrade_duration_sec)
            return
        port_helper = self.gen_port_helper(port_info.volume)

        version_dst = port_helper.get_version_dst()
        version_now = port_helper.get_version_src()
        if len(version_dst) == 0 or len(version_now) == 0:
            self.logging_ports_error(state, port_info, 'get version failed: dst=%s, now=%s', version_dst, version_now)
            return
        if int(version_now) != int(version_dst):
            self.logging_ports_debug(state, port_info, 'upgrade: version not equal, wait: %s/%s', \
                                    state.state_update_duration(), g_config.maximum_upgrade_duration_sec)
            return
        self.logging_ports_info(state, port_info, 'upgrade successful')
        state.set_port_state(PortStateType.CHECKVERSION, port_info.volume)

    def run_handle_one_port_check_version(self, state: PortState, port_info: PortsDevInfo):
        self.logging_ports_debug(state, port_info, 'in state')
        if len(port_info.dev_id) == 0 or len(port_info.volume) == 0:
            state.set_port_state(PortStateType.ERROR, info='check version failed')
            self.logging_ports_debug(state, port_info, 'in state')
            return
        port_helper = self.gen_port_helper(port_info.volume)
        if port_helper is None:
            state.set_port_state(PortStateType.ERROR, volume=port_info.volume, info='not support device')
            self.logging_ports_error(state, port_info, 'not support device')
            return
        # re-check version
        version_dst = port_helper.get_version_dst()
        version_now = port_helper.get_version_src()
        if len(version_dst) == 0 or len(version_now) == 0:
            state.set_port_state(PortStateType.ERROR, volume=port_info.volume, info = 'get version failed')
            self.logging_ports_error(state, port_info, 'get version failed: dst=%s, now=%s', version_dst, version_now);
            return
        if self.check_need_update_with_version(version_now, version_dst):
            state.set_port_state(PortStateType.ERROR, volume=port_info.volume, info = f'version still needs upgrade(now={version_now}, dst={version_dst})')
            self.logging_ports_error(state, port_info, 'upgrade failed(version still needs upgrade)')
            return 
        # check wifi version
        if port_helper.type == PortDeviceType.Camera:
            wifi_version_now = port_helper.get_wifi_version_now()
            wifi_version_dst = port_helper.get_wifi_version_dst()
            if g_config.wifi_check_version:
                if len(wifi_version_now) == 0 or len(wifi_version_dst) == 0:
                    state.set_port_state(PortStateType.ERROR, volume=port_info.volume, info = 'upgrade failed')
                    self.logging_ports_error(state, port_info, 'get wifi version failed')
                    return 
                if wifi_version_now != wifi_version_dst:
                    state.set_port_state(PortStateType.ERROR, volume=port_info.volume, info = 'wifi version not match')
                    self.logging_ports_error(state, port_info, 'wifi version not match: now=%s, dst=%s', 
                                             wifi_version_now, wifi_version_dst)
                    return 
                self.logging_ports_debug(state, port_info, 'wifi version matched: now=%s, dst=%s', 
                                             wifi_version_now, wifi_version_dst)
        self.logging_ports_debug(state, port_info, 'version check passed')
        state.set_port_state(PortStateType.UPGRADED, volume=port_info.volume)
        return 

    def run_handle_one_port_upgraded(self, state: PortState, port_info: PortsDevInfo):
        self.logging_ports_debug(state, port_info, 'in state')
        if len(port_info.dev_id) == 0 or len(port_info.volume) == 0:
            state.set_port_state(PortStateType.INIT)
            self.logging_ports_debug(state, port_info, 'in state')
            return
    def run_handle_one_port_error(self, state: PortState, port_info: PortsDevInfo):
        self.logging_ports_debug(state, port_info, 'in state')
        if len(port_info.dev_id) == 0 or len(port_info.volume) == 0:
            state.set_port_state(PortStateType.INIT)
            self.logging_ports_debug(state, port_info, 'in state')
            return

    def run_port(self, state: PortState, port_info: PortsDevInfo):
        try:
            if state.get_port_state() == PortStateType.INIT:
                self.run_handle_one_port_init(state, port_info)
            elif state.get_port_state() == PortStateType.WAITINSERT:
                self.run_handle_one_port_wait_insert(state, port_info)
            elif state.get_port_state() == PortStateType.WAITVOLUMEMOUNT:
                self.run_handle_one_port_wait_volume_mount(state, port_info)
            elif state.get_port_state() == PortStateType.UPGRADEPREPARE:
                self.run_handle_one_port_upgrade_prepare(state, port_info)
            elif state.get_port_state() == PortStateType.UPGRADING:
                self.run_handle_one_port_upgrading(state, port_info)
            elif state.get_port_state() == PortStateType.CHECKVERSION:
                self.run_handle_one_port_check_version(state, port_info)
            elif state.get_port_state() == PortStateType.UPGRADED:
                self.run_handle_one_port_upgraded(state, port_info)
            elif state.get_port_state() == PortStateType.ERROR:
                self.run_handle_one_port_error(state, port_info)
            else:
                self.logging_ports_error(state, port_info, 'state invalid')
        except Exception as e:
            self.logging_ports_error(state, port_info, 'state process error')
            state.set_port_state(PortStateType.ERROR, volume=f'{port_info.volume}', info=f'Error{e}')
            self.logging_ports_error(state, port_info, 'into state')


    def run(self):
        if self.__running_state == RunningState.INIT:
            self.run_init()
        elif self.__running_state == RunningState.ALLPOWERUP:
            self.run_all_powerup()
        elif self.__running_state == RunningState.HANDLEPORTS:
            self.run_handle_ports()

if __name__ == "__main__":
    logging.info('begin')
    checker = StatusChecker('COM14')
    while True:
        checker.run()
        time.sleep(3)
    logging.info('end')