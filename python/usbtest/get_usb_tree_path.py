import win32com.client
import win32api
import win32con
import ctypes
from ctypes import wintypes

# Load cfgmgr32.dll for device tree traversal
cfgmgr32 = ctypes.WinDLL("cfgmgr32")

# CM_Get_Child / CM_Get_Sibling constants
CM_LOCATE_DEVNODE_NORMAL = 0

# Define CM functions
CM_Get_Child = cfgmgr32.CM_Get_Child
CM_Get_Child.argtypes = [ctypes.POINTER(ctypes.c_ulong), ctypes.c_ulong, ctypes.c_ulong]
CM_Get_Child.restype = ctypes.c_ulong

CM_Get_Sibling = cfgmgr32.CM_Get_Sibling
CM_Get_Sibling.argtypes = [ctypes.POINTER(ctypes.c_ulong), ctypes.c_ulong, ctypes.c_ulong]
CM_Get_Sibling.restype = ctypes.c_ulong

CM_Get_Parent = cfgmgr32.CM_Get_Parent
CM_Get_Parent.argtypes = [ctypes.POINTER(ctypes.c_ulong), ctypes.c_ulong, ctypes.c_ulong]
CM_Get_Parent.restype = ctypes.c_ulong

CM_Get_Device_ID = cfgmgr32.CM_Get_Device_IDW
CM_Get_Device_ID.argtypes = [ctypes.c_ulong, ctypes.c_wchar_p, ctypes.c_ulong, ctypes.c_ulong]
CM_Get_Device_ID.restype = ctypes.c_ulong

MAX_DEVICE_ID_LEN = 200

def get_device_id(dev_inst):
    buffer = ctypes.create_unicode_buffer(MAX_DEVICE_ID_LEN)
    if CM_Get_Device_ID(dev_inst, buffer, MAX_DEVICE_ID_LEN, 0) == 0:
        return buffer.value
    return None

def walk_device_tree(dev_inst, depth=0):
    dev_id = get_device_id(dev_inst)
    indent = "  " * depth
    if dev_id:
        print(f"{indent}- {dev_id}")

    # 遍历子设备
    child = ctypes.c_ulong()
    if CM_Get_Child(ctypes.byref(child), dev_inst, 0) == 0:
        walk_device_tree(child.value, depth + 1)

        # 遍历兄弟设备
        sibling = ctypes.c_ulong()
        while CM_Get_Sibling(ctypes.byref(sibling), child.value, 0) == 0:
            walk_device_tree(sibling.value, depth + 1)
            child = sibling

def main():
    # 使用 WMI 获取所有 USB 控制器
    wmi = win32com.client.GetObject("winmgmts:")
    usb_controllers = wmi.InstancesOf("Win32_USBController")

    for controller in usb_controllers:
        print(f"\n[USB Controller] {controller.Name}")
        # 获取 Device Instance Handle（DevInst）
        pnp_device_id = controller.PNPDeviceID

        # 获取根 DevInst（通过 Config Manager）
        dev_inst = ctypes.c_ulong()
        if cfgmgr32.CM_Locate_DevNodeW(ctypes.byref(dev_inst), ctypes.c_wchar_p(pnp_device_id), CM_LOCATE_DEVNODE_NORMAL) == 0:
            walk_device_tree(dev_inst.value)

if __name__ == "__main__":
    main()
