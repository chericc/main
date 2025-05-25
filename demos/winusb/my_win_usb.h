#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int mwu_get_disk_id_with_dev_id(const char *devid, char *disk_id, size_t disk_id_size);

#ifdef __cplusplus
}
#endif // __cplusplus