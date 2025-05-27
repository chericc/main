#include <stdio.h>

#include "my_win_usb_wrapper.h"

int main()
{
	printf("hello\n");
	const char* dev_id = "USB\\VID_14CD&PID_1212\\121220160204";
	const char *disk_id = mwu_get_disk_id_with_dev_id_wrap(dev_id);
	printf("devid: %s\n", dev_id);
	printf("disk_id: %s\n", disk_id);
	return 0;
}