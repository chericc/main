#include "my_print.h"

int g_enable_log = 0;

int mwu_enable_log(int enable)
{
	g_enable_log = enable;
	return 0;
}