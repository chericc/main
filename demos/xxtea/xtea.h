#ifndef __XTEA_H__
#define __XTEA_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif 

void xtea_encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);
void xtea_decipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);

void xtea_encipher_simple(const char *string, size_t size, 
    uint8_t *out, size_t *out_size, const char *key, unsigned int rounds);

#ifdef __cplusplus
}
#endif 

#endif // __XTEA_H__