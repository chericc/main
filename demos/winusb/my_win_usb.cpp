#include "my_win_usb.h"

#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <initguid.h>
#include <wbemidl.h>
#include <comdef.h>

#include <vector>
#include <string>
#include <iostream>

#include <stdio.h>

#include "my_print.h"

int mwu_get_disk_id_with_dev_id(mwu_disk_id_cb cb, void *user_ptr)
{
	int suc_flag = 0;
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

	do {
		hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_DISKDRIVE, nullptr, nullptr, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE) {
			MY_LOG("SetupDiGetClassDevs failed: %d\n", GetLastError());
			break;
		}

		SP_DEVINFO_DATA devInfoData = {};
		devInfoData.cbSize = sizeof(devInfoData);
		for (int i = 0; ; ++i) {
			int ret = SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData);
			if (!ret) {
				MY_LOG("iter end\n");
				break;
			}

			char instance_id[1024] = {};
			char parent_id[1024] = {};

			ret = CM_Get_Device_ID(devInfoData.DevInst, instance_id, sizeof(instance_id), 0);
			if (ret != CR_SUCCESS) {
				MY_LOG("get dev id failed\n");
				continue;
			}

			DEVINST parentDevInst = CR_INVALID_DEVINST;
			ret = CM_Get_Parent(&parentDevInst, devInfoData.DevInst, 0);
			if (ret != CR_SUCCESS) {
				MY_LOG("failed to get parent node\n");
				continue;
			}

			ret = CM_Get_Device_ID(parentDevInst, parent_id, sizeof(parent_id), 0);
			if (ret != CR_SUCCESS) {
				MY_LOG("get parent dev id failed\n");
				continue;
			}

			MY_LOG("parent: %s\n", parent_id);
			MY_LOG("node: %s\n", instance_id);
			if (cb) {
				if (cb(parent_id, instance_id, user_ptr)) {
					break;
				}
			}
		}

	} while (0);

	if (hDevInfo != INVALID_HANDLE_VALUE) {
		SetupDiDestroyDeviceInfoList(hDevInfo);
		hDevInfo = INVALID_HANDLE_VALUE;
	}

	return suc_flag ? 0 : -1;
}