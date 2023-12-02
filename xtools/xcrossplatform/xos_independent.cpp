#include "xos_independent.hpp"

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