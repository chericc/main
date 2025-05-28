import json
from enum import Enum

import serial
from numpy.matlib import empty
import time

from hub import UsbSerialCmd
from mysystem import MySystem
from mysystem import PortsDevInfo
from myconfig import MyConfig
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
    Init = 0
    AllPowerUp = 1
    HandlePorts = 2

class PortStateType(Enum):
    Init = 1
    WaitInsert = 2
    WaitVolumeMount = 3
    Upgrade = 4
    Upgraded = 5

class PortState:
    def __init__(self):
        self.port_state = PortStateType.Init
    port_state: PortStateType

class PortDeviceType(Enum):
    Camera = 1
    Screen = 2
    Unknown = 3

class PortTypeHelper:
    type: PortDeviceType
    version_file_path: str
    update_file_dst_path: str
    update_file_src_path: str
    # volume should like: 'E:'
    def __init__(self, port_dev_type: PortDeviceType = PortDeviceType.Unknown, volume: str = None):
        self.type : PortDeviceType = port_dev_type
        if self.type == PortDeviceType.Camera:
            self.version_file_path = os.path.join(volume, MyConfig.camera_version_file_path)
            self.update_file_dst_path = os.path.join(volume, MyConfig.camera_update_dst_file_path)
            self.update_file_src_path = os.path.join(volume, MyConfig.camera_update_file_from_path)
        elif self.type == PortDeviceType.Screen:
            self.version_file_path = os.path.join(volume, MyConfig.screen_version_file_path)
            self.update_file_dst_path = os.path.join(volume, MyConfig.screen_update_dst_file_path)
            self.update_file_src_path = os.path.join(volume, MyConfig.screen_update_file_from_path)
        else:
            logging.error('unknown port dev type')
    def get_version_src(self) -> str:
        try:
            content = None
            with open(self.version_file_path, 'r') as f:
                content = f.read()
            if self.type == PortDeviceType.Camera:
                json_begin = content.find('{')
                json_end = content.rfind('}') + 1
                if json_begin == -1 or json_end == 0:
                    logging.error(f'locate json failed: {content}')
                    return None
                json_str = content[json_begin:json_end]
                logging.debug(f'json: {json_str}')
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
            logging.error('get version failed')
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

class StatusChecker:
    def __init__(self, serial_name):
        self.__running_state = RunningState.Init
        self.__serial_name = serial_name
        self.__port_ctl = UsbSerialCmd(self.__serial_name)
        self.__system = MySystem()
        self.__port_states: list[PortState] = []
        if len(self.__port_states) == 0:
            for i in range(MyConfig.hub_port_count):
                port_state = PortState()
                self.__port_states.append(port_state)

    def get_port_state(self):
        return copy.deepcopy(self.__port_states)
    
    def run_init(self):
        # 最开始，把所有端口上电
        self.__port_ctl.control_all_port(open=True)
        logging.debug('into AllPowerUp state')
        self.__running_state = RunningState.AllPowerUp
    def run_all_powerup(self):
        # 所有端口上电后，等待设备开机
        for count in range(MyConfig.wait_port_seconds):
            time.sleep(1)
            all_ports_info = self.__system.get_all_ports_dev_info()
            if len(all_ports_info) > 0:
                logging.debug('some port inserted')
                break
            logging.debug('wait some port insert')
            continue
        # 有端口上线，开始处理
        logging.debug('into HandlePorts state')
        self.__running_state = RunningState.HandlePorts
    def run_handle_ports(self):
        port_num = MyConfig.hub_port_count
        all_ports_info = self.__system.get_all_ports_dev_info()
        for i in range(MyConfig.hub_port_count):
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
            state.port_state = PortStateType.WaitInsert
            logging.debug(f'port[{port_info.port_number}] is not inserted, into {state.port_state} state')
        elif len(port_info.volume) == 0:
            state.port_state = PortStateType.WaitVolumeMount
            logging.debug(f'port[{port_info.port_number}] is not mounted, into {state.port_state} state')
        else:
            state.port_state = PortStateType.Upgrade
            logging.debug(f'port[{port_info.port_number}] is mounted, into {state.port_state} state')

    def run_handle_one_port_wait_insert(self, state: PortState, port_info: PortsDevInfo):
        if len(port_info.dev_id) == 0:
            logging.debug(f'port[{port_info.port_number}] is not inserted, wait')
        else:
            state.port_state = PortStateType.WaitVolumeMount
            logging.debug(f'port[{port_info.port_number}] is inserted, into {state.port_state} state')

    def run_handle_one_port_wait_volume_mount(self, state: PortState, port_info: PortsDevInfo):
        if len(port_info.volume) == 0:
            logging.debug(f'port[{port_info.port_number}] is not mounted, wait')
        else:
            state.port_state = PortStateType.Upgrade
            logging.debug(f'port[{port_info.port_number}] is mounted, into {state.port_state} state')

    def check_port_type(self, volume: str):
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

    def do_camera_upgrade(self, helper: PortTypeHelper, state: PortState, port_info: PortsDevInfo):
        # check version
        version_dst = helper.get_version_dst()
        version_now = helper.get_version_src()
        if len(version_dst) == 0 or len(version_now) == 0:
            logging.error(f'get version failed: dst={version_dst}, now={version_now}')
            return False
        if int(version_now) < int(version_dst):
            logging.debug(f'port[{port_info.port_number}] need update: {version_now} < {version_dst}')
            logging.debug(f'port[{port_info.port_number}] cp ota file: {helper.update_file_src_path} --> {helper.update_file_dst_path}')
            shutil.copyfile(helper.update_file_src_path, helper.update_file_dst_path)
            time.sleep(0.2)
            logging.debug(f'port[{port_info.port_number}] power off]')
            self.__port_ctl.control_usb_port(port_info.port_number, False)
            time.sleep(0.2)
            logging.debug(f'port[{port_info.port_number}] power on]')
            self.__port_ctl.control_usb_port(port_info.port_number, True)
            state.port_state = PortStateType.WaitVolumeMount
        else:
            logging.debug(f'port[{port_info.port_number}] need not update: {version_now} >= {version_dst}')
            state.port_state = PortStateType.Upgraded
        return True

    def run_handle_one_port_upgrade(self, state: PortState, port_info: PortsDevInfo):
        logging.debug(f'port[{port_info.port_number}] in upgrade state({port_info.volume})')
        if len(port_info.dev_id) == 0 or len(port_info.volume) == 0:
            state.port_state = PortStateType.Init
            logging.debug(f'port[{port_info.port_number}] removed, into {state.port_state} state')
            return
        port_type_helper = self.check_port_type(port_info.volume)
        if port_type_helper is None:
            return
        else:
            self.do_camera_upgrade(port_type_helper, state, port_info)

    def run_handle_one_port_upgraded(self, state: PortState, port_info: PortsDevInfo):
        logging.debug(f'port[{port_info.port_number}] in upgraded state({port_info.volume}), wait unplugging')
        if len(port_info.dev_id) == 0 or len(port_info.volume) == 0:
            state.port_state = PortStateType.Init
            logging.debug(f'port[{port_info.port_number}] removed, into {state.port_state} state')
            return

    def run_port(self, state: PortState, port_info: PortsDevInfo):
        if state.port_state == PortStateType.Init:
            self.run_handle_one_port_init(state, port_info)
        elif state.port_state == PortStateType.WaitInsert:
            self.run_handle_one_port_wait_insert(state, port_info)
        elif state.port_state == PortStateType.WaitVolumeMount:
            self.run_handle_one_port_wait_volume_mount(state, port_info)
        elif state.port_state == PortStateType.Upgrade:
            self.run_handle_one_port_upgrade(state, port_info)
        elif state.port_state == PortStateType.Upgraded:
            self.run_handle_one_port_upgraded(state, port_info)
        else:
            logging.error(f'port[{port_info.port_number}] state invalid: {state.port_state}')
            pass

    def run(self):
        if self.__running_state == RunningState.Init:
            self.run_init()
        elif self.__running_state == RunningState.AllPowerUp:
            self.run_all_powerup()
        elif self.__running_state == RunningState.HandlePorts:
            self.run_handle_ports()

if __name__ == "__main__":
    logging.info('begin')
    checker = StatusChecker('COM14')
    while True:
        checker.run()
        time.sleep(3)
    logging.info('end')