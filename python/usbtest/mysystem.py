import subprocess
import os
import xml.etree.cElementTree as ET
import copy
import wmi

class PortsDevInfo:
    dev_id: str
    port_number: int
    volume: str

class MySystem:
    def __init__(self):
        self.__default_xml_file = "output.xml"

    def get_usb_tree_view_as_xml(self, output_file: ''):
        try:
            command = ['d:/code/usbview.exe', '/q', '/f', f'/savexml:{output_file}']
            result = subprocess.run(command,
                                    check=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    text=True)
            print(f'USBTreeView executed ok. Output saved to {output_file}')
            return True
        except subprocess.CalledProcessError as e:
            print(f'USBTreeView failed with return code {e.returncode}')
            return False
        except FileNotFoundError:
            print(f'USBTreeView could not find executable')
            return False
    def parse_usb_xml(self, xml_file: str):

        tree = ET.parse(xml_file)
        root = tree.getroot()

        parent_node = None

        # locate hub's node
        found_flag = False
        ext_hubs = root.findall('.//ExternalHub')
        for ext_hub in ext_hubs:
            dev_id = ext_hub.get('DeviceId')
            if dev_id is None:
                continue
            if not (('PID_5411' in dev_id) and ('VID_0BDA' in dev_id)):
                continue
            ext_sub_hubs = ext_hub.findall('.//ExternalHub')
            for ext_sub_hub in ext_sub_hubs:
                dev_id = ext_sub_hub.get('DeviceId')
                if dev_id is None:
                    continue
                if not (('PID_5411' in dev_id) and ('VID_0BDA' in dev_id)):
                    continue
                parent_node = ext_hub
                found_flag = True
                break
            if found_flag:
                break

        if not found_flag:
            return False

        port_infos = []

        # traverse hub's sub hub
        hubs_under_hub = parent_node.findall('ExternalHub')
        port_index = 0
        for hub in hubs_under_hub:
            dev_id = hub.get('DeviceId')
            if dev_id is None:
                continue
            print('hub.dev_id:', dev_id)
            usb_devices = hub.findall('UsbDevice')
            for usb_device in usb_devices:
                prot = usb_device.get('UsbProtocol')  # USB 2.0
                port_num = usb_device.get('UsbPortNumber')  # 2 [1,2,3,4...]
                name = usb_device.get('DeviceName')  # USB 大容量存储设备
                dev_id = usb_device.get('DeviceId')  # USB\VID_302E&amp;PID_041B\0000000000000000
                port_num_in_hub = port_index + int(port_num)
                print(f'port_num_in_hub: {port_num_in_hub}, dev_id: {dev_id}')
                port_info = PortsDevInfo()
                port_info.port_number = int(port_num_in_hub)
                port_info.dev_id = dev_id
                port_infos.append(port_info)
            port_index += 4

        return port_infos
    def complete_volume(self, port_infos: list[PortsDevInfo]):
        for port_info in port_infos:
            pass
    def get_all_ports_dev_info(self) -> list[PortsDevInfo]:
        self.get_usb_tree_view_as_xml(self.__default_xml_file)
        dev_infos = self.parse_usb_xml(self.__default_xml_file)
        self.complete_volume(dev_infos)
        return dev_infos

if __name__ == "__main__":
    print("begin")
    sys = MySystem()
    dev_infos = sys.get_all_ports_dev_info()
    for dev_info in dev_infos:
        print(f'devinfo: port={dev_info.port_number}, id={dev_info.dev_id}')
    print("end")