#include "func.hpp"

#include <cstdint>

Func::Func(std::shared_ptr<UploaderInterface> uploader, int ntimes)
{
    _uploader = uploader;
    _ntimes = ntimes;
}

int Func::do_sth()
{
    uint8_t data[64]{0};

    if (_uploader->connect() < 0)
    {
        printf("connect failed\n");
        return -1;
    }

    for (int i = 0; i < _ntimes; ++i)
    {
        if (_uploader->upload(data, sizeof(data)) < 0)
        {
            printf("upload failed\n");
            break;
        }
    }

    if (_uploader->disconnect() < 0)
    {
        printf("disconnect failed\n");
        return -1;
    }
    return 0;
}

