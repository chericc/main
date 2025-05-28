#include "my_win_usb_wrapper.h"

#include <stdio.h>

#include "my_win_usb.h"

#include "my_print.h"

static char s_disk_id_str[2048];

struct ctx {
	char disk_id[2048];
	const char* dev_id;
};

int my_mwu_disk_id_cb(const char* dev_id, const char* disk_id, void *user)
{
	struct ctx* ctx = (struct ctx*)user;
	if (strcmp(ctx->dev_id, dev_id) == 0) {
		int len = strlen(ctx->disk_id);
		int new_len = strlen(disk_id);
		if (len + new_len < sizeof(ctx->disk_id)) {
			snprintf(ctx->disk_id + len, sizeof(ctx->disk_id) - len, "%s,", disk_id);
		}
	}
	else {
		MY_LOG("not match: %s - %s\n", ctx->dev_id, dev_id);
	}
	return 0;
}

const char* mwu_get_disk_id_with_dev_id_wrap(const char* dev_id)
{
	s_disk_id_str[0] = 0;
	struct ctx ctx = {};
	ctx.dev_id = dev_id;
	mwu_get_disk_id_with_dev_id(my_mwu_disk_id_cb, &ctx);
	snprintf(s_disk_id_str, sizeof(s_disk_id_str), "%s", ctx.disk_id);
	return s_disk_id_str;
}
void mwu_enable_log_wrap(int enable)
{
	mwu_enable_log(enable);
}
