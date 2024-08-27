#include <stdio.h>
#include <time.h>

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

void test_utc()
{
    char str[] = "2024-08-27T10:12:29.791Z";

    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    sscanf(str, "%4d-%2d-%2dT%2d:%2d:%2d", 
        &year, &month, &day, &hour, &minute, &second);
    printf("scanf: %04d-%02d-%02d %02d:%02d:%02d\n", 
        year, month, day, hour, minute, second);
    
    return ;
}

int main() 
{
    test_utc();
    return 0;
}