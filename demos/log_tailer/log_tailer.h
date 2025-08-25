//
// A tool like 'tail -F'

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef void* log_tailer_handle;
typedef void (*log_tailer_cb)(const char *line, size_t size);
log_tailer_handle log_tailer_open(const char *path, log_tailer_cb cb);
int log_tailer_run_process(log_tailer_handle handle);
int log_tailer_close(log_tailer_handle handle);

#ifdef __cplusplus
}
#endif // __cplusplus
