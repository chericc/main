#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef int(*mwu_disk_id_cb)(const char* disk_id);
int mwu_get_disk_id_with_dev_id(mwu_disk_id_cb cb);

#ifdef __cplusplus
}
#endif // __cplusplus