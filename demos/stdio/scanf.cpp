#include <stdio.h>

int main()
{
    char str[] = "he1llo_az123_357.aac";

    int len = -1;
    int ret = sscanf(str, "%*[^_]_%*[^_]_%d", &len);
    printf("ret=%d, len=%d\n", ret, len);
    return 0;
}