#include <cstring>

#include "xlog.h"

#include "xtea.h"
#include "base64.h"

int main()
{
    xlog_dbg("hello\n");

    const int round = 8;
    const char *password = "Welcome to China!";

    unsigned char hello[] = "hello\n";
    char out[sizeof(hello) * 2] = {};

    xlog_dbg("input: %s\n", (char*)hello);

    base64_encode(hello, sizeof(hello), out);
    xlog_dbg("base64: %s\n", out);

    uint8_t enc[sizeof(out) + 8];
    size_t enc_size = sizeof(enc);

    uint8_t dec[sizeof(out) + 8];
    size_t dec_size = sizeof(dec);
    
    int ret = xtea_encipher_string(out, strlen(out), enc, &enc_size, password, round, 1);
    if (ret < 0) {
        xlog_err("encipher failed\n");
    }
    xtea_encipher_string(enc, enc_size, dec, &dec_size, password, round, 0);

    xlog_dbg("after: {:.{}}, size: {}\n", (char*)dec, dec_size, dec_size);

    return 0;
}