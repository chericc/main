#include <stdio.h>

void test1()
{
    // get 357
    char str[] = "he1llo_az123_357.aac";

    int len = -1;
    int ret = sscanf(str, "%*[^_]_%*[^_]_%d", &len);
    printf("ret=%d, len=%d\n", ret, len);
}

void test2()
{
    char str[] = "ab123 78fyg\n";
    char str1[256];
    char str2[256];
    int ret = sscanf(str, "%[^ ] %[a-z0-9]", str1, str2);
    printf("ret=%d, str1=%s, str2=%s\n", ret, str1, str2);
}

int main() 
{
    test2();
    return 0;
}