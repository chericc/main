
#pragma once

#include <cstddef>

class UploaderInterface
{
public:
    virtual ~UploaderInterface() = default;
    virtual int connect() = 0;
    virtual int disconnect() = 0;
    virtual int upload(void *data, size_t size) = 0;
};