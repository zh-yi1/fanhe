#include "include.h"
#include "awk_mem.h"
#include "cjson/cJSON.h"

CJSON_PUBLIC(void) asl_cJSON_InitHooks(cJSON_Hooks* hooks)
{
    // cJSON_InitHooks(hooks);
}

CJSON_PUBLIC(cJSON *) asl_cJSON_Parse(const char *value)
{
    return cJSON_Parse(value);
}

CJSON_PUBLIC(void) asl_cJSON_Delete(cJSON *item)
{
    cJSON_Delete(item);
}

CJSON_PUBLIC(int) asl_cJSON_GetArraySize(const cJSON *array)
{
    return cJSON_GetArraySize(array);
}

CJSON_PUBLIC(cJSON *) asl_cJSON_GetArrayItem(const cJSON *array, int index)
{
    return cJSON_GetArrayItem(array, index);
}

CJSON_PUBLIC(cJSON_bool) asl_cJSON_IsString(const cJSON * const item)
{
    return cJSON_IsString(item);
}

CJSON_PUBLIC(cJSON_bool) asl_cJSON_IsObject(const cJSON * const item)
{
    return cJSON_IsObject(item);
}

CJSON_PUBLIC(cJSON_bool) asl_cJSON_IsNumber(const cJSON * const item)
{
    return cJSON_IsNumber(item);
}

CJSON_PUBLIC(cJSON_bool) asl_cJSON_IsArray(const cJSON * const item)
{
    return cJSON_IsArray(item);
}

CJSON_PUBLIC(cJSON *) asl_cJSON_CreateArray(void)
{
    return cJSON_CreateArray();
}

CJSON_PUBLIC(cJSON *) asl_cJSON_CreateObject(void)
{
    return cJSON_CreateObject();
}

CJSON_PUBLIC(char *) asl_cJSON_PrintUnformatted(const cJSON *item)
{
    void *ptr = cJSON_PrintUnformatted(item);
    // printf("awk json malloc:0x%p 0x%p\n", ptr, __builtin_return_address(0));
    return ptr;

    // char *ptr = cJSON_PrintUnformatted(item);
    // if (ptr) {
    //     int len = strlen(ptr) + 1;
    //     char *new_str = psram_heap_malloc(len);
    //     strcpy(new_str, ptr);
    //     ab_free(ptr);
    //     return new_str;
    // }
    // return NULL;
}

CJSON_PUBLIC(cJSON *) asl_cJSON_CreateNumber(double num)
{
    return cJSON_CreateNumber(num);
}

CJSON_PUBLIC(cJSON *) asl_cJSON_CreateString(const char *string)
{
    return cJSON_CreateString(string);
}

CJSON_PUBLIC(cJSON *) asl_cJSON_CreateInt(int i)
{
    return cJSON_CreateNumber(i);
}

CJSON_PUBLIC(cJSON_bool) asl_cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    return cJSON_AddItemToArray(array, item);
}

CJSON_PUBLIC(cJSON_bool) asl_cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    return cJSON_AddItemToObject(object, string, item);
}

CJSON_PUBLIC(cJSON *) asl_cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string)
{
    return cJSON_GetObjectItemCaseSensitive(object, string);
}

CJSON_PUBLIC(void) asl_cJSON_free(void *object)
{
    cJSON_free(object);
}

CJSON_PUBLIC(double) asl_cJSON_GetNumberValue(const cJSON * const item)
{
    return cJSON_GetNumberValue(item);
}

CJSON_PUBLIC(int) asl_cJSON_GetIntValue(const cJSON * const item)
{
    return item->valueint;
}
