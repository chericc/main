#include <stdio.h>

#include "my_win_usb.h"

int main()
{
	printf("hello\n");
	mwu_get_disk_id_with_dev_id(NULL, NULL, 0);
	return 0;
}