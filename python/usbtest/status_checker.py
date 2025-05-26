from enum import Enum

import serial

from device import UsbSerialCmd

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

class StatusChecker:
    def __init__(self, serial_name):
        self.__port_state = []
        for i in range(10):
            self.__port_state.append(Status.UNKNOWN)
        self.__running_state = RunningState.Init
        self.__serial_name = serial_name
        self.__port_ctl = UsbSerialCmd(self.__serial_name)

    def get_port_state(self):
        return self.__port_state
    
    def run_init(self):
        # 最开始，把所有端口上电
        self.__port_ctl.control_all_port(open=True)
        self.__running_state = RunningState.AllPowerUp
    def run_all_powerup(self):
        # 所有端口上电后，等待设备开机

    def run(self):
        if self.__running_state == RunningState.Init:
            self.run_init()
        elif self.__running_state == RunningState.AllPowerUp:
            self.run_all_powerup()