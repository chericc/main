// This is an example of how to use stdint.h to write code
// that is independent from the environment.
// For example, when we are writing

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <limits>

int main() {
    // size_t ssize_t
    {
        size_t size = -1;
        ssize_t ssize = -1;
        printf("size=%zu, ssize=%zd\n", size, ssize);
    }

    // ptr intptr_t
    {
        int a = 0;
        int b = 0;
        uintptr_t ptr_a = (uintptr_t)&a;
        uintptr_t ptr_b = (uintptr_t)&b;
        intptr_t ptr_diff = (intptr_t)(&b - &a);
        printf("&a = %#" PRIxPTR "\n", ptr_a);
        printf("&b = %#" PRIxPTR "\n", ptr_b);
        printf("&b - &a = %#" PRIxPTR "\n", ptr_diff);
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
        printf("uint16=%" PRIu16 "\n", uint16);
        printf("int32=%" PRId32 "\n", int32);
        printf("uint32=%" PRIu32 "\n", uint32);
        printf("int64=%" PRId64 "\n", int64);
        printf("uint64=%" PRIu64 "\n", uint64);
    }

    // intmax_t
    {
        intmax_t big_signed_number = std::numeric_limits<intmax_t>().max();
        uintmax_t big_unsinged_number = std::numeric_limits<uintmax_t>().max();
        printf("intmax=%" PRIdMAX "\n", big_signed_number);
        printf("uintmax=%" PRIuMAX "\n", big_unsinged_number);
    }

    return 0;
}