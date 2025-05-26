import wmi
import win32file
import sys

def get_physical_drive_from_letter(drive_letter):
    # 使用Win32 API获取盘符对应的设备路径
    path = r"\\.\%s" % drive_letter.rstrip("\\")
    try:
        handle = win32file.CreateFile(
            path,
            0,
            win32file.FILE_SHARE_READ | win32file.FILE_SHARE_WRITE,
            None,
            win32file.OPEN_EXISTING,
            0,
            None
        )
        device = win32file.DeviceIoControl(handle, 0x00070000, None, 1024)
        win32file.CloseHandle(handle)
        return device
    except Exception as e:
        print(f"Error getting physical device: {e}")
        return None

def get_usb_device_path(drive_letter):
    c = wmi.WMI()
    
    # 获取逻辑盘到分区的映射
    for logical_disk in c.Win32_LogicalDisk():
        if logical_disk.DeviceID.upper() == drive_letter.upper():
            # 找出分区
            for partition in c.Win32_DiskPartition():
                for logical_disk_link in partition.associators("Win32_LogicalDiskToPartition"):
                    if logical_disk_link.DeviceID.upper() == drive_letter.upper():
                        # 找出物理磁盘
                        for disk_drive in c.Win32_DiskDrive():
                            for partition_link in disk_drive.associators("Win32_DiskDriveToDiskPartition"):
                                if partition_link.DeviceID == partition.DeviceID:
                                    # 检查是否是USB接口
                                    if 'USB' in disk_drive.PNPDeviceID:
                                        print(f"U盘盘符 {drive_letter} 对应物理设备：")
                                        print(f"  设备ID: {disk_drive.DeviceID}")
                                        print(f"  USB设备路径: {disk_drive.PNPDeviceID}")
                                        return disk_drive.PNPDeviceID
    print(f"未找到与盘符 {drive_letter} 对应的USB设备。")
    return None

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("用法: python find_usb_device.py <盘符>")
        print("例如: python find_usb_device.py E:")
    else:
        drive_letter = sys.argv[1]
        get_usb_device_path(drive_letter)
