#include "xos_independent.hpp"

int x_fseek64(FILE *stream, int64_t offset, int whence)
{
    int ret = 0;

    ret = fseeko64(stream, offset, whence);

    return ret;
}

int64_t x_ftell64(FILE *stream)
{
    int64_t ret = 0;

    ret = ftello64(stream);

    return ret;
}