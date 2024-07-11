#include <xlog.hpp>

#include "cjson/cJSON.h"
#include "cjson/cJSON_Utils.h"

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

int main()
{
    demo1();
    return 0;
}