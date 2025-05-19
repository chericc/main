import wmi




def test():
    w = wmi.WMI()

    print('logical disks: ')
    logical_disks = w.Win32_LogicalDisk()
    for ld in logical_disks:
        print(f'{ld}')

    print('disk partitions: ')
    disk_partitions = w.Win32_DiskPartition()
    for par in disk_partitions:
        print(f'{par}')

    print('disk drives: ')
    disk_drives = w.Win32_DiskDrive()
    for dr in disk_drives:
        print(f'{dr}')

    usb_hubs = w.Win32_USBHub()
    for hub in usb_hubs:
        print(f'{hub}')

    pnp_ent = w.Win32_PnPEntity()
    for pnp in pnp_ent:
        print(f'${pnp}')

    usb_ctl = w.Win32_USBControllerDevice()
    for uc in usb_ctl:
        print(f'${uc}')

if __name__ == "__main__":
    print("begin")
    test()
    print("end")