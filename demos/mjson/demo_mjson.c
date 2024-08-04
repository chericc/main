#include <stdio.h>

#include "mjson.h"

static int my_json_snprintf_pretty(char *buf, size_t len, const char *src, size_t len_src)
{
    struct mjson_fixedbuf fb = {buf, (int) len, 0};
    mjson_pretty(src, len_src, " ", mjson_print_fixed_buf, &fb);
    return fb.len;
}

/*

cmd:

{
 "cmd": "test",
 "a": {
  "a": 1,
  "b": "test"
 },
 "b": {
  "number": 0
 }
}


*/

static const char *type_name(int type)
{
    switch (type) {
        case MJSON_TOK_STRING: return "string";
        case MJSON_TOK_NUMBER: return "number";
        case MJSON_TOK_TRUE: return "true";
        case MJSON_TOK_FALSE: return "false";
        case MJSON_TOK_NULL: return "null";
        case MJSON_TOK_ARRAY: return "array";
        case MJSON_TOK_OBJECT: return "object";
        default: return "unknown";
    }
}

int main()
{
    char cmd_a[64];
    char cmd_b[3][64];
    char cmd[256];
    mjson_snprintf(cmd_a, sizeof(cmd_a), "{%Q:%d,%Q:%Q}", 
        "a",1,"b","test");
    printf("cmd_a:\n%s\n", (char*)cmd_a);
    for (int i = 0; i < 3; ++i) {
        mjson_snprintf(cmd_b[i], sizeof(cmd_b[i]), "{%Q:%d}", "number", i);
        printf("cmd_b[%d]:\n%s\n", i, (char*)cmd_b[i]);
    }
    mjson_snprintf(cmd, sizeof(cmd), "{%Q:%Q,%Q:%s,%Q:%s}",
        "cmd", "test", "a", cmd_a, "b", cmd_b);
    printf("cmd:\n%s\n", (char*)cmd);

    char cmd_pretty[256] = { 0 };
    my_json_snprintf_pretty(cmd_pretty, sizeof(cmd_pretty), cmd, sizeof(cmd));
    printf("cmd_pretty: \n%s\n", (char*)cmd_pretty);

    if (1)
    {
        int koff, klen, voff, vlen, vtype, off;

        for (off = 0; (off = mjson_next(cmd, strlen(cmd), off, &koff, &klen,
            &voff, &vlen, &vtype)) != 0; )
        {
            printf("key: %.*s, value: %.*s, type: %s\n", klen, cmd + koff, vlen, cmd + voff, 
                type_name(vtype));
        }
    }

    return 0;
}