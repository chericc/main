import subprocess
import os
import xml.etree.cElementTree as ET
import copy
import wmi

def get_usb_tree_view_as_xml(output_file: ''):
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

def parse_xml_file(xml_file: str):

    # xml_file = 'test.xml'

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

    # traverse hub's sub hub
    hubs_under_hub = parent_node.findall('ExternalHub')
    for hub in hubs_under_hub:
        dev_id = hub.get('DeviceId')
        if dev_id is None:
            continue
        print('dev_id:', dev_id)
        usb_devices = hub.findall('UsbDevice')
        for usb_device in usb_devices:
            prot = usb_device.get('UsbProtocol')  # USB 2.0
            port_num = usb_device.get('UsbPortNumber') # 2 [1,2,3,4...]
            name = usb_device.get('DeviceName') # USB 大容量存储设备
            dev_id = usb_device.get('DeviceId') # USB\VID_302E&amp;PID_041B\0000000000000000
            print(f'prot: {prot}, port_num: {port_num}, port: {port_num}, name: {name}, dev_id: {dev_id}')

    return True

if __name__ == "__main__":
    print("begin")
    xml_file='output.xml'
    # xml_file = 'test.xml'
    ret = get_usb_tree_view_as_xml(xml_file)
    if not ret:
        print(f'USBTreeView could not find executable')
        exit(-1)
    parse_xml_file(xml_file)

    print("end")