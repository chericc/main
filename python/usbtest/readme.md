# win-usb

python -m pip install wmi pywin32

# wmi

https://timgolden.me.uk/python/wmi/tutorial.html#the-basics

https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/win32-logicaldisktopartition

## Win32_LogicalDisk

```bash
instance of Win32_LogicalDisk
{
        Access = 0;
        Caption = "Z:";
        Compressed = FALSE;
        CreationClassName = "Win32_LogicalDisk";
        Description = "网络连接";
        DeviceID = "Z:";
        DriveType = 4;
        FileSystem = "NTFS";
        FreeSpace = "735566393344";
        MaximumComponentLength = 255;
        MediaType = 0;
        Name = "Z:";
        ProviderName = "\\\\10.0.0.3\\share";
        Size = "982820896768";
        SupportsDiskQuotas = FALSE;
        SupportsFileBasedCompression = FALSE;
        SystemCreationClassName = "Win32_ComputerSystem";
        SystemName = "DESKTOP-8CJTJC3";
        VolumeName = "share";
        VolumeSerialNumber = "0CB43C3D";
};
```

## Win32_DiskPartition

```bash

```

## usbview.deviceid

https://groups.google.com/g/microsoft.public.development.device.drivers/c/1Xo8NAcoJVk?pli=1

SetupDiGetDeviceInstanceId

deviceid.
USB\VID_0781&PID_5581\A20054F50D478A23
USB\VID_0781&PID_5581\A20054F50D478A23

disk.
USBSTOR\DISK&VEN_SANDISK&PROD_SANDISK_ULTRA&REV_PMAP\A20054F50D478A23&0
USBSTOR\DISK&VEN_SANDISK&PROD_SANDISK_ULTRA&REV_PMAP\A20054F50D478A23&0

Get-WmiObject Win32_DiskDrive | Select-Object Model,DeviceID,PNPDeviceID,InterfaceType
Get-WmiObject Win32_DiskPartition | Select-Object Name,DeviceID,PNPDeviceID
Get-WmiObject Win32_PnPEntity | Select-Object *

设备管理器：
设备实例路径 -- device id from pnp
父系 -- usb devid


## USBSTOR\DISK&VEN_SANDISK&PROD_SANDISK_ULTRA&REV_PMAP\A20054F50D478A23&0


PSComputerName              : DESKTOP-5Q0I681
__GENUS                     : 2
__CLASS                     : Win32_PnPEntity
__SUPERCLASS                : CIM_LogicalDevice
__DYNASTY                   : CIM_ManagedSystemElement
__RELPATH                   : Win32_PnPEntity.DeviceID="USBSTOR\\DISK&VEN_SANDISK&PROD_SANDISK_ULTRA&REV_PMAP\\A20054F50D478A23&0"
__PROPERTY_COUNT            : 26
__DERIVATION                : {CIM_LogicalDevice, CIM_LogicalElement, CIM_ManagedSystemElement}
__SERVER                    : DESKTOP-5Q0I681
__NAMESPACE                 : root\cimv2
__PATH                      : \\DESKTOP-5Q0I681\root\cimv2:Win32_PnPEntity.DeviceID="USBSTOR\\DISK&VEN_SANDISK&PROD_SANDISK_ULTRA&REV_PMAP\\A20054F50D478A23&0"
Availability                : 
Caption                     : SanDisk Sandisk Ultra USB Device
ClassGuid                   : {4d36e967-e325-11ce-bfc1-08002be10318}
CompatibleID                : {USBSTOR\Disk, USBSTOR\RAW, GenDisk}
ConfigManagerErrorCode      : 0
ConfigManagerUserConfig     : False
CreationClassName           : Win32_PnPEntity
Description                 : 磁盘驱动器
DeviceID                    : USBSTOR\DISK&VEN_SANDISK&PROD_SANDISK_ULTRA&REV_PMAP\A20054F50D478A23&0
ErrorCleared                : 
ErrorDescription            : 
HardwareID                  : {USBSTOR\DiskSanDisk_Sandisk_Ultra___PMAP, USBSTOR\DiskSanDisk_Sandisk_Ultra___, USBSTOR\DiskSanDisk_, USBSTOR\SanDisk_Sandisk_Ultra___P...}
InstallDate                 : 
LastErrorCode               : 
Manufacturer                : (标准磁盘驱动器)
Name                        : SanDisk Sandisk Ultra USB Device
PNPClass                    : DiskDrive
PNPDeviceID                 : USBSTOR\DISK&VEN_SANDISK&PROD_SANDISK_ULTRA&REV_PMAP\A20054F50D478A23&0
PowerManagementCapabilities : 
PowerManagementSupported    : 
Present                     : True
Service                     : disk
Status                      : OK
StatusInfo                  : 
SystemCreationClassName     : Win32_ComputerSystem
SystemName                  : DESKTOP-5Q0I681
Scope                       : System.Management.ManagementScope
Path                        : \\DESKTOP-5Q0I681\root\cimv2:Win32_PnPEntity.DeviceID="USBSTOR\\DISK&VEN_SANDISK&PROD_SANDISK_ULTRA&REV_PMAP\\A20054F50D478A23&0"
Options                     : System.Management.ObjectGetOptions
ClassPath                   : \\DESKTOP-5Q0I681\root\cimv2:Win32_PnPEntity
Properties                  : {Availability, Caption, ClassGuid, CompatibleID...}
SystemProperties            : {__GENUS, __CLASS, __SUPERCLASS, __DYNASTY...}
Qualifiers                  : {dynamic, Locale, provider, UUID}
Site                        : 
Container        

## USB\VID_0781&PID_5581\A20054F50D478A23


PSComputerName              : DESKTOP-5Q0I681
__GENUS                     : 2
__CLASS                     : Win32_PnPEntity
__SUPERCLASS                : CIM_LogicalDevice
__DYNASTY                   : CIM_ManagedSystemElement
__RELPATH                   : Win32_PnPEntity.DeviceID="USB\\VID_0781&PID_5581\\A20054F50D478A23"
__PROPERTY_COUNT            : 26
__DERIVATION                : {CIM_LogicalDevice, CIM_LogicalElement, CIM_ManagedSystemElement}
__SERVER                    : DESKTOP-5Q0I681
__NAMESPACE                 : root\cimv2
__PATH                      : \\DESKTOP-5Q0I681\root\cimv2:Win32_PnPEntity.DeviceID="USB\\VID_0781&PID_5581\\A20054F50D478A23"
Availability                : 
Caption                     : USB 大容量存储设备
ClassGuid                   : {36fc9e60-c465-11cf-8056-444553540000}
CompatibleID                : {USB\Class_08&SubClass_06&Prot_50, USB\Class_08&SubClass_06, USB\Class_08}
ConfigManagerErrorCode      : 0
ConfigManagerUserConfig     : False
CreationClassName           : Win32_PnPEntity
Description                 : USB 大容量存储设备
DeviceID                    : USB\VID_0781&PID_5581\A20054F50D478A23
ErrorCleared                : 
ErrorDescription            : 
HardwareID                  : {USB\VID_0781&PID_5581&REV_0100, USB\VID_0781&PID_5581}
InstallDate                 : 
LastErrorCode               : 
Manufacturer                : 兼容 USB 存储设备
Name                        : USB 大容量存储设备
PNPClass                    : USB
PNPDeviceID                 : USB\VID_0781&PID_5581\A20054F50D478A23
PowerManagementCapabilities : 
PowerManagementSupported    : 
Present                     : True
Service                     : USBSTOR
Status                      : OK
StatusInfo                  : 
SystemCreationClassName     : Win32_ComputerSystem
SystemName                  : DESKTOP-5Q0I681
Scope                       : System.Management.ManagementScope
Path                        : \\DESKTOP-5Q0I681\root\cimv2:Win32_PnPEntity.DeviceID="USB\\VID_0781&PID_5581\\A20054F50D478A23"
Options                     : System.Management.ObjectGetOptions
ClassPath                   : \\DESKTOP-5Q0I681\root\cimv2:Win32_PnPEntity
Properties                  : {Availability, Caption, ClassGuid, CompatibleID...}
SystemProperties            : {__GENUS, __CLASS, __SUPERCLASS, __DYNASTY...}
Qualifiers                  : {dynamic, Locale, provider, UUID}
Site                        : 
Container                   : 