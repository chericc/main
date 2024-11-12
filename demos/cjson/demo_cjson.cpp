#include <xlog.hpp>

#include "cjson/cJSON.h"
// #include "cjson/cJSON_Utils.h"

void demo1()
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "deviceId", cJSON_CreateString("mzzzzzzz"));
    // char *json = cJSON_Print(root);
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    xlog_dbg("json: \n%s\n", json);
    cJSON_free(json);
}

void demo2()
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "deviceId", cJSON_CreateString("mzzzzzzz"));
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    xlog_dbg("json: \n%s\n", json);
    cJSON_free(json);
}

void demo3()
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "total", 50);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < 2; ++i) {
        cJSON *sub = cJSON_CreateObject();
        cJSON_AddNumberToObject(sub, "name", i);
        cJSON_AddItemToArray(arr, sub);
    }
    cJSON_AddItemToObject(root, "names", arr);

    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    xlog_dbg("json: \n%s\n", json);
    cJSON_free(json);
}

int main()
{
    demo3();
    return 0;
}