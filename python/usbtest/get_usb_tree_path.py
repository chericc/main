import wmi
import sys

def get_drive_usb_device_pnpid(drive_letter):
    c = wmi.WMI()
    
    for logical_disk in c.Win32_LogicalDisk():
        if logical_disk.DeviceID.upper() == drive_letter.upper():
            for partition in c.Win32_DiskPartition():
                for logical_disk_link in partition.associators("Win32_LogicalDiskToPartition"):
                    if logical_disk_link.DeviceID.upper() == drive_letter.upper():
                        for disk_drive in c.Win32_DiskDrive():
                            for partition_link in disk_drive.associators("Win32_DiskDriveToDiskPartition"):
                                if partition_link.DeviceID == partition.DeviceID:
                                    if 'USB' in disk_drive.PNPDeviceID:
                                        return disk_drive.PNPDeviceID
    return None

def find_device_tree_path(pnp_device_id):
    c = wmi.WMI(namespace="root\\cimv2")
    devices = list(c.Win32_PnPEntity())
    dev_map = {dev.PNPDeviceID: dev for dev in devices if dev.PNPDeviceID}

    path = []
    current_id = pnp_device_id

    while current_id in dev_map:
        dev = dev_map[current_id]
        path.append((dev.Name, dev.PNPDeviceID))
        
        # 查找父设备
        parent_id = None
        try:
            assoc_query = f"ASSOCIATORS OF {{Win32_PnPEntity.DeviceID='{current_id.replace('\\', '\\\\')}'}} WHERE AssocClass=Win32_PnPEntity"
            parents = c.query(assoc_query)
            if parents:
                parent_id = parents[0].PNPDeviceID
        except:
            break

        if not parent_id or parent_id == current_id:
            break
        current_id = parent_id

    return path

def main(drive_letter):
    print(f"查找盘符 {drive_letter} 对应的USB设备树路径...")
    pnp_id = get_drive_usb_device_pnpid(drive_letter)
    if not pnp_id:
        print("未找到该盘符对应的USB设备。")
        return

    print(f"\n✅ 找到PNPDeviceID: {pnp_id}\n")

    tree = find_device_tree_path(pnp_id)
    print("📦 USB设备树路径（从子设备到主控制器）:")
    for level, (name, dev_id) in enumerate(tree):
        indent = "  " * level
        print(f"{indent}- {name}")
        print(f"{indent}  PNPDeviceID: {dev_id}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("用法: python find_usb_tree.py <盘符>")
        print("例如: python find_usb_tree.py E:")
    else:
        main(sys.argv[1].upper())
