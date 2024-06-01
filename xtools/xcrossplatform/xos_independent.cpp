#include "xos_independent.hpp"
#include <cstring>

int x_fseek64(FILE *stream, int64_t offset, int whence)
{
    int ret = 0;

#if defined(X_PLATFORM_MSVC)
    ret = _fseeki64(stream, offset, whence);
#elif defined(X_PLATFORM_CLANG)
    static_assert(sizeof(off_t) == sizeof(int64_t), "");
    ret = fseeko(stream, offset, whence);
#else 
    ret = fseeko64(stream, offset, whence);
#endif

    return ret;
}

int64_t x_ftell64(FILE *stream)
{
    int64_t ret = 0;

#if defined(X_PLATFORM_MSVC)
    ret = _ftelli64(stream);
#elif defined(X_PLATFORM_CLANG)
    static_assert(sizeof(off_t) == sizeof(int64_t), "");
    ret = ftello(stream);
#else 
    ret = ftello64(stream);
#endif 

    return ret;
}

int x_strerror(int errnum, char* buf, size_t buflen)
{
    int ret = 0;
#if defined(X_PLATFORM_MSVC)
    ret = strerror_s(buf, buflen, errnum);
#else 

#if ((_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE) || defined (X_PLATFORM_CLANG)
    ret = strerror_r(errnum, buf, buflen);
#else 
    char *res = strerror_r(errnum, buf, buflen);
    if (nullptr == res)
    {
        ret = -1;
    }
    else 
    {
        strncpy(buf, res, buflen);
        buf[buflen - 1] = 0;
    }
#endif 

#endif
    return ret;
}