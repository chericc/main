import abc

class HostInfo:
    def __init__(self):
        self.mac = ''
        self.wifi = '0'
        self.ip = ''
        self.name = ''
        self.upspeed = ''
        self.downspeed = ''

class ListenerInterface(metaclass=abc.ABCMeta):
    def __init__(self):
        super().__init__()

    @abc.abstractclassmethod
    def Notify(self, hostinfo:HostInfo):
        pass