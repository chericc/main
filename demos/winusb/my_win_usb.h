#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef int(*mwu_disk_id_cb)(const char* dev_id, const char* disk_id,  void *user_ptr);
int mwu_get_disk_id_with_dev_id(mwu_disk_id_cb cb, void *user_ptr);

#ifdef __cplusplus
}
#endif // __cplusplus