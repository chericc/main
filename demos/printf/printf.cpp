// This is an example of how to use stdint.h to write code
// that is independent from the environment.
// For example, when we are writing

#include <stdio.h>
#include <stdint.h>

#include <inttypes.h>

int main()
{
    // size_t ssize_t
    {
        size_t size = 0;
        ssize_t ssize = 0;
        printf("size=%zu, ssize=%zd\n", size, ssize);
    }

    // ptr intptr_t
    {

    }

    // int8_t uint8_t ...
    {
        int8_t int8 = -8;
        uint8_t uint8 = 8;
        int16_t int16 = -16;
        uint16_t uint16 = 16;
        int32_t int32 = -32;
        uint32_t uint32 = 32;
        int64_t int64 = -64;
        uint64_t uint64 = 64;

        printf("int8=%" PRId8 "\n", int8);
        printf("uint8=%" PRIu8 "\n", uint8);
        printf("int16=%" PRId16 "\n", int16);
    }

    return 0;
}