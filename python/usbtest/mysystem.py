import subprocess
import os
import xml.etree.cElementTree as ET
import copy
import wmi
import ctypes
import pythoncom
from contextlib import contextmanager
import logging
import myresource
import time

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
        lib_path = myresource.resource_path('libs/my_win_usb.dll')
        self.__mylib = ctypes.CDLL(lib_path)# Define the function prototype
        self.__mylib.mwu_get_disk_id_with_dev_id_wrap.argtypes = [ctypes.c_char_p]  # Input: const char*
        self.__mylib.mwu_get_disk_id_with_dev_id_wrap.restype = ctypes.c_char_p     # Output: const char*

    def get_usb_tree_view_as_xml(self, output_file: str):
        try:
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            startupinfo.wShowWindow = subprocess.SW_HIDE

            bin_path=myresource.resource_path('bins/usbview.exe')
            command = [bin_path, '/q', '/f', f'/savexml:{output_file}']

            result = subprocess.run(command,
                                    check=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    text=True,
                                    startupinfo=startupinfo)
            logging.debug('USBTreeView executed ok. Output saved to %s', output_file)
            return True
        except subprocess.CalledProcessError as e:
            logging.error('USBTreeView failed with return code %s', e.returncode)
            return False
        except FileNotFoundError:
            logging.error('USBTreeView could not find executable')
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
            logging.debug('hub.dev_id: %s', dev_id)
            usb_devices = hub.findall('UsbDevice')
            for usb_device in usb_devices:
                prot = usb_device.get('UsbProtocol')  # USB 2.0
                port_num = usb_device.get('UsbPortNumber')  # 2 [1,2,3,4...]
                name = usb_device.get('DeviceName')  # USB 大容量存储设备
                dev_id = usb_device.get('DeviceId')  # USB\VID_302E&amp;PID_041B\0000000000000000
                port_num_in_hub = port_index + int(port_num)   # in [1,2,3,...]
                logging.debug('port_num_in_hub: %s, dev_id: %s', port_num_in_hub, dev_id)
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
        start_time = time.time()
        self.get_usb_tree_view_as_xml(self.__default_xml_file)
        dev_infos = self.parse_usb_xml(self.__default_xml_file)
        self.complete_volume(dev_infos)
        end_time = time.time()
        logging.debug('cost time: %.2f', end_time - start_time)

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

            query = fr"SELECT Index FROM Win32_DiskDrive WHERE PNPDeviceID = '{dev_id}'"
            
            disk_drive = c.query(query)
            if len(disk_drive) > 0:
                logging.debug('found disk')
                disk_index = disk_drive[0].Index

            if disk_index is None:
                return None
            
            query = fr"SELECT DeviceID FROM Win32_DiskPartition WHERE DiskIndex = {disk_index}"
            disk_partitions = c.query(query)

            if len(disk_partitions) == 0:
                return None
            
            deviceid = disk_partitions[0].DeviceID

            query = "SELECT Dependent,Antecedent FROM Win32_LogicalDiskToPartition"
            disktoparts = c.query(query)
            for disktopart in disktoparts:
                if deviceid in str(disktopart.Antecedent):
                    drive_letters.append(disktopart.Dependent.Name)
                    break

            return drive_letters
        except Exception as e:
            logging.error("An error occurred: %s", e)
            return None

    def get_disk_with_dev_id(self, dev_id: str) -> str:
        dev_id_bytes = dev_id.encode('utf-8')
        # Example: Call the function with a device ID
        disk_id = self.__mylib.mwu_get_disk_id_with_dev_id_wrap(dev_id_bytes)

        # Convert the returned bytes to a Python string (if needed)
        disk_id_str = disk_id.decode("utf-8") if disk_id else ''
        logging.debug('Disk ID: %s', disk_id_str)
        return disk_id_str

if __name__ == "__main__":
    logging.basicConfig(
        level=logging.DEBUG,
        format='%(asctime)s %(levelname)s %(filename)s:%(lineno)s:%(funcName)s %(message)s',
    )
    logging.info("begin")
    sys = MySystem()
    dev_infos = sys.get_all_ports_dev_info()
    for dev_info in dev_infos:
        logging.info('devinfo: port=%s, id=%s, volume=%s', dev_info.port_number, dev_info.dev_id, dev_info.volume)
    logging.info("end")