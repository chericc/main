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