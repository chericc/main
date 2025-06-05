#include "log_tailer.h"

#include <stdio.h>

struct log_tailer_ctx {
    FILE *fp;

};

log_tailer_handle log_tailer_open(const char *path, log_tailer_cb cb)
{
    
}

int log_tailer_run_process(log_tailer_handle handle)
{

}

int log_tailer_close(log_tailer_handle handle)
{

}