/*

Operating system related functions.

*/

#pragma once

#include <cstdint>
#include <cstdio>

int x_fseek64(FILE* stream, int64_t offset, int whence);
int64_t x_ftell64(FILE* stream);

int x_strerror(int errnum, char* buf, size_t buflen);
int x_setthreadname(const char *name);