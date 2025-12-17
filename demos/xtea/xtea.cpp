#include "xtea.h"

#include <string.h>

#include "city.hpp"
#include "xlog.h"

/* take 64 bits of data in v[0] and v[1] and 128 bits of key[0] - key[3] */

void xtea_encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
    unsigned int i;
    uint32_t v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
    for (i=0; i < num_rounds; i++) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
    }
    v[0]=v0; v[1]=v1;
}

void xtea_decipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
    unsigned int i;
    uint32_t v0=v[0], v1=v[1], delta=0x9E3779B9, sum=delta*num_rounds;
    for (i=0; i < num_rounds; i++) {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
        sum -= delta;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
    }
    v[0]=v0; v[1]=v1;
}

static unsigned int align_up_generic(unsigned int value, unsigned int n) 
{
    return ((value + n - 1) / n) * n;
}

static void calc_key(const char *key, uint32_t key_array[4])
{
    uint128 hash = CityHash128(key, strlen(key));
    auto *key_64 = reinterpret_cast<uint64_t*>(key_array);
    key_64[0] = hash.first;
    key_64[1] = hash.second;

    return ;
}

int xtea_encipher_string(const void *string, size_t size, 
    void *out, size_t *out_size, const char *key, unsigned int rounds, int encipher)
{
    // 
    size_t size_align_up_8 = align_up_generic(size, 8);
    size_t total_blocks = size_align_up_8 / 8;

    if (*out_size < size_align_up_8) {
        xlog_dbg("buf too small({}, {})", (int)*out_size, (int)size_align_up_8);
        return -1;
    }

    uint32_t key_array[4] = {};
    calc_key(key, key_array);

    memcpy(out, string, size);
    memset((uint8_t*)out + size, 0, size_align_up_8 - size);

    auto array = reinterpret_cast<uint64_t*>(out);
    for (size_t ib = 0; ib < total_blocks; ++ib) {
        if (0 != encipher) {
            xtea_encipher(rounds, reinterpret_cast<uint32_t *>(&array[ib]), key_array);
        } else {
            xtea_decipher(rounds, reinterpret_cast<uint32_t *>(&array[ib]), key_array);
        }
    }
    
    *out_size = size_align_up_8;
    
    return 0;
}