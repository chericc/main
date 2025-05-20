import wmi

def list_usb_ports():
    w = wmi.WMI()
    # pnps = w.Win32_PnPEntity()
    # for p in pnps:
    #     # print(p.PNPDeviceID)
    #     # print(p)
    #     name = ''
    #     name = p.PNPDeviceID
    #     if 'VID_0BDA' in name.upper() and 'PID_5411' in name.upper():
    #         print(f'{name}')
    usbcontrollers = w.Win32_USBController()
    for usb in usbcontrollers:
        print(usb.PNPDeviceID)
        assos = usb.associators(wmi_result_class="Win32_PnPEntity")
        for asso in assos:
            print(f'{asso}')
    # usbcontroller_dev = w.Win32_USBControllerDevice()
    # for u_d in usbcontroller_dev:
    #     print(f'{u_d.Antecedent.PNPDeviceID}\n --{u_d.Dependent.PNPDeviceID}')



if __name__ == "__main__":
    print("begin123")
    list_usb_ports()
    print("end123")