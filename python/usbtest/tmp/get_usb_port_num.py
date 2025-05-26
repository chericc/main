import ctypes
import ctypes.wintypes as wintypes
import sys
from ctypes import POINTER, byref, sizeof, c_ulong, c_ubyte, c_void_p

# Constants
DIGCF_PRESENT = 0x00000002
DIGCF_DEVICEINTERFACE = 0x00000010
GUID_CLASS_USBHUB = ctypes.c_byte * 16
USB_HUB_GUID = bytes.fromhex('F18A0E88C30C11D0A5D6' '28A0A4036CE3B')  # GUID for USB hubs

# Load DLLs
setupapi = ctypes.windll.LoadLibrary('setupapi')
kernel32 = ctypes.windll.kernel32

# Data types
class GUID(ctypes.Structure):
    _fields_ = [
        ("Data1", wintypes.DWORD),
        ("Data2", wintypes.WORD),
        ("Data3", wintypes.WORD),
        ("Data4", wintypes.BYTE * 8),
    ]

def string_guid_to_struct(guid_bytes):
    return GUID.from_buffer_copy(guid_bytes)

class SP_DEVICE_INTERFACE_DATA(ctypes.Structure):
    _fields_ = [
        ("cbSize", wintypes.DWORD),
        ("InterfaceClassGuid", GUID),
        ("Flags", wintypes.DWORD),
        ("Reserved", ctypes.c_ulonglong),
    ]

class SP_DEVINFO_DATA(ctypes.Structure):
    _fields_ = [
        ("cbSize", wintypes.DWORD),
        ("ClassGuid", GUID),
        ("DevInst", wintypes.DWORD),
        ("Reserved", ctypes.c_ulonglong),
    ]

class SP_DEVICE_INTERFACE_DETAIL_DATA_A(ctypes.Structure):
    _fields_ = [
        ("cbSize", wintypes.DWORD),
        ("DevicePath", ctypes.c_char * 260),
    ]

def list_usb_hub_ports():
    # Get USB Hub interface class GUID
    guid = string_guid_to_struct(USB_HUB_GUID)

    # Get device info set for USB hubs
    devinfo = setupapi.SetupDiGetClassDevsA(
        byref(guid), None, None,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    )
    if devinfo == -1:
        print("Failed to get USB hub device info list")
        return

    i = 0
    while True:
        iface_data = SP_DEVICE_INTERFACE_DATA()
        iface_data.cbSize = ctypes.sizeof(SP_DEVICE_INTERFACE_DATA)

        success = setupapi.SetupDiEnumDeviceInterfaces(devinfo, None, byref(guid), i, byref(iface_data))
        if not success:
            break

        # Get required buffer size
        required_size = wintypes.DWORD()
        setupapi.SetupDiGetDeviceInterfaceDetailA(
            devinfo, byref(iface_data), None, 0,
            byref(required_size), None
        )

        detail = SP_DEVICE_INTERFACE_DETAIL_DATA_A()
        detail.cbSize = 5  # On 64-bit, 8. On 32-bit, 5
        dev_info_data = SP_DEVINFO_DATA()
        dev_info_data.cbSize = ctypes.sizeof(SP_DEVINFO_DATA)

        success = setupapi.SetupDiGetDeviceInterfaceDetailA(
            devinfo, byref(iface_data),
            byref(detail), required_size,
            None, byref(dev_info_data)
        )

        if success:
            device_path = detail.DevicePath.decode('utf-8')
            print(f"USB Hub found at: {device_path}")
            # To actually get port details, you must open this device and call IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
            # This part requires using DeviceIoControl and a USB driver like WinUSB
        else:
            print("Failed to get device interface detail")

        i += 1

    setupapi.SetupDiDestroyDeviceInfoList(devinfo)

list_usb_hub_ports()
