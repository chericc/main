import subprocess
import os
import xml.etree.cElementTree as ET
import copy
import wmi
import ctypes
import pythoncom
from contextlib import contextmanager

class PortsDevInfo:
    def __init__(self):
        self.port_number = -1  # 1,2,3,...
        self.dev_id = ''
        self.volume = ''
    port_number: int
    dev_id: str
    volume: str

class MySystem:
    def __init__(self):
        self.__default_xml_file = "output.xml"
        self.__mylib = ctypes.CDLL('D:/code/main/python/usbtest/my_win_usb.dll')# Define the function prototype
        self.__mylib.mwu_get_disk_id_with_dev_id_wrap.argtypes = [ctypes.c_char_p]  # Input: const char*
        self.__mylib.mwu_get_disk_id_with_dev_id_wrap.restype = ctypes.c_char_p     # Output: const char*

    def get_usb_tree_view_as_xml(self, output_file: ''):
        try:
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            startupinfo.wShowWindow = subprocess.SW_HIDE

            command = ['d:/code/usbview.exe', '/q', '/f', f'/savexml:{output_file}']

            result = subprocess.run(command,
                                    check=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    text=True,
                                    startupinfo=startupinfo)
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
                port_num_in_hub = port_index + int(port_num)   # in [1,2,3,...]
                print(f'port_num_in_hub: {port_num_in_hub}, dev_id: {dev_id}')
                port_info = PortsDevInfo()
                port_info.port_number = int(port_num_in_hub)
                port_info.dev_id = dev_id
                port_infos.append(port_info)
            port_index += 4

        return port_infos
    def complete_volume(self, port_infos: list[PortsDevInfo]):
        for port_info in port_infos:
            dev_ids = self.get_disk_with_dev_id(port_info.dev_id)
            disk_id_list = dev_ids.split(',')
            for disk_id in disk_id_list:
                volume_name = self.get_drive_letters_from_disk_dev_id_in_thread_wrapper(disk_id)
                if volume_name is not None and len(volume_name) > 0 :
                    port_info.volume = volume_name[0]
                    break

    def get_all_ports_dev_info(self) -> list[PortsDevInfo]:
        self.get_usb_tree_view_as_xml(self.__default_xml_file)
        dev_infos = self.parse_usb_xml(self.__default_xml_file)
        self.complete_volume(dev_infos)
        sorted_infos = sorted(dev_infos, key=lambda x: x.port_number)
        return sorted_infos

    @contextmanager
    def com_initialized(self):
        pythoncom.CoInitialize()
        try:
            yield
        finally:
            pythoncom.CoUninitialize()

    def get_drive_letters_from_disk_dev_id_in_thread_wrapper(self, disk_id):
        with self.com_initialized():
            return self.get_drive_letters_from_disk_dev_id(disk_id)

    def get_drive_letters_from_disk_dev_id(self, dev_id: str):
        c = wmi.WMI()
        drive_letters = []
        try:
            disk_index = None
            disk_drives = c.Win32_DiskDrive()
            for disk_drive in disk_drives:
                print(f'disk_drive: {disk_drive.PNPDeviceID}, index: {disk_drive.Index}')
                if disk_drive.PNPDeviceID == dev_id:
                    print('found disk')
                    disk_index = disk_drive.Index
            if disk_index is None:
                return None
            disk_partitions = c.Win32_DiskPartition()
            for disk_partition in disk_partitions:
                if disk_partition.DiskIndex == disk_index:
                    print(f'found partition')
                    logical_disks = disk_partition.associators('Win32_LogicalDiskToPartition')
                    for logical_disk in logical_disks:
                        print(f'ldisk: {logical_disk.Name}')
                        drive_letters.append(logical_disk.Name)
            return drive_letters
        except Exception as e:
            print(f"An error occurred: {e}")
            return None

    def get_disk_with_dev_id(self, dev_id: str) -> str:
        dev_id_bytes = dev_id.encode('utf-8')
        # Example: Call the function with a device ID
        disk_id = self.__mylib.mwu_get_disk_id_with_dev_id_wrap(dev_id_bytes)

        # Convert the returned bytes to a Python string (if needed)
        disk_id_str = disk_id.decode("utf-8") if disk_id else None
        print("Disk ID:", disk_id_str)
        return disk_id_str

if __name__ == "__main__":
    print("begin")
    sys = MySystem()
    dev_infos = sys.get_all_ports_dev_info()
    for dev_info in dev_infos:
        print(f'devinfo: port={dev_info.port_number}, id={dev_info.dev_id}, volume={dev_info.volume}')
    print("end")