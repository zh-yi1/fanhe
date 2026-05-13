#include "alipay_common.h"
#include "vendor_file.h"
#include "include.h"

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN

#define ALIPAY_TEST_LOG(format, ...) \
    printf("[ALIPAY_TEST]"#format"\n", ##__VA_ARGS__)

#define TEST_CASE(func_name) \
    bool TEST_##func_name()

#define TEST_TAG ALIPAY_TEST_LOG("[ALIPAY_TEST]===================TEST:[%s]\n", __func__)

#define TEST_IT(func)                                         \
do {                                                            \
    ALIPAY_TEST_LOG("==============BEGIN %s", #func);           \
    ret = TEST_##func(); \
    if (ret == true) ALIPAY_TEST_LOG("SUCCESS: %s", #func); \
    else ALIPAY_TEST_LOG("FAILURE: %s",#func); \
    ALIPAY_TEST_LOG("==============END %s", #func); \
} while(0)

//test quick_js
EXTERNC bool js_engine_init(void);
EXTERNC void js_engine_destroy(void);
EXTERNC bool js_engine_hasPrepared(void);
EXTERNC bool js_engine_eval_script(char* eval_pack,uint32_t len_eval_pack);

static char* BRIDGECODE = "function callNative(method, param) {\n"
"    if (!method || typeof(method) != \"string\" || method.length == 0) { console.log(\"method invalid\");  return null; }\n"
"    if (!param) { param = {}; }\n"
"    if (typeof(param) != 'object') { console.log(\"param type invalid\"); return null; }\n"
"    paramStr = JSON.stringify(param);\n"
"    var result = dpe_method_router.methodRouter(method, paramStr);\n"
"    return result;\n"
"};";

static char* CALLCODE = "try {\n"
"    var param = JSON.parse( \"{}\" );var result = getCode(param);\n"
"    if (result) { re_callback.returnResultToRTos(result.toString());} else { re_callback.returnErrorToRTos(\"getCode return null\");}\n"
"} catch (error) {re_callback.returnErrorToRTos(\"getCode got error\" + error.message) ;}";

#define TEST_FILE_NAME      "test_file_name_long_long_long_long"
#define TEST_FILE_DATA      "qwertyuiopasdfghjklzxcvbnm1234567890mnbvcxzlkjhgfdsaqwertyuiop"
TEST_CASE(file_read){
    TEST_TAG;

    void* fd = NULL;
    uint8_t buf_temp[128] = {0,};
    uint32_t len_buf_temp = sizeof(buf_temp);

    char file_name[128];
    strncpy(file_name, TEST_FILE_NAME, sizeof(file_name) - 1);
    file_name[127] = '\0'; // 确保字符串以'\0'结尾

    if((int)(fd = alipay_open_rsvd_part(file_name)) <= 0){
        ALIPAY_TEST_LOG("fail to open file");
        return false;
    }

    if((alipay_read_rsvd_part(fd, buf_temp, &len_buf_temp) != 0)){
        ALIPAY_TEST_LOG("should fail to read[%d]", len_buf_temp);
        alipay_close_rsvd_part(fd);
        return false;
    }

    if(alipay_close_rsvd_part(fd) < 0){
        ALIPAY_TEST_LOG("fail to close file");
        return false;
    }

    return true;
}

int my_file_read(void)
{
    printf("%s\n", __func__);
    char file_name[128];
    strncpy(file_name, TEST_FILE_NAME, sizeof(file_name) - 1);
    file_name[127] = '\0'; // 确保字符串以'\0'结尾

    void* fd = NULL;
    uint8_t buf_temp[128] = {0,};
    uint32_t len_buf_temp = sizeof(buf_temp);

    if((int)(fd = alipay_open_rsvd_part(file_name)) <= 0){
        printf("fail to open file\n");
        return false;
    }

    if((alipay_read_rsvd_part(fd, buf_temp, &len_buf_temp) != 0)){
        printf("should fail to read[%d]\n", len_buf_temp);
        alipay_close_rsvd_part(fd);
        return false;
    }

    if(alipay_close_rsvd_part(fd) < 0){
        printf("fail to close file\n");
        return false;
    }
    printf("---->%s OK\n", __func__);
    return true;
}

TEST_CASE(file_write_and_read){
    TEST_TAG;

    char file_name[128];
    strncpy(file_name, TEST_FILE_NAME, sizeof(file_name) - 1);
    file_name[127] = '\0'; // 确保字符串以'\0'结尾

    void* fd = NULL;
    if((int)(fd = alipay_open_rsvd_part(file_name)) <= 0){
        ALIPAY_TEST_LOG("fail to open file");
        return false;
    }

    if(alipay_write_rsvd_part(fd, TEST_FILE_DATA, strlen(TEST_FILE_DATA)+1) != 0){
        ALIPAY_TEST_LOG("fail to write");
        return false;
    }

    if(alipay_close_rsvd_part(fd) != 0){
        ALIPAY_TEST_LOG("fail to close");
        return false;
    }

    fd = NULL;

    uint8_t buf_temp[128] = {0,};
    uint32_t len_buf_temp = sizeof(buf_temp);
    if((int)(fd = alipay_open_rsvd_part(file_name)) <= 0){
        ALIPAY_TEST_LOG("fail to open file");
        return false;
    }
    if((alipay_read_rsvd_part(fd, buf_temp, &len_buf_temp) != 0) || (len_buf_temp == 0) || (len_buf_temp != (strlen(TEST_FILE_DATA)+1))){
        ALIPAY_TEST_LOG("fail to read[%d]", len_buf_temp);
        return false;
    }

    if(alipay_clear_rsvd_part() < 0){
        ALIPAY_TEST_LOG("alipay_clear_rsvd_part error");
        return false;
    }

    if(alipay_access_rsvd_part(file_name) != 0){
        ALIPAY_TEST_LOG("fail to clear file");
        return false;
    }

    return true;
}

int my_file_write_and_read(void)
{
    printf("%s\n", __func__);
    char file_name[128];
    strncpy(file_name, TEST_FILE_NAME, sizeof(file_name) - 1);
    file_name[127] = '\0'; // 确保字符串以'\0'结尾

    void* fd = NULL;
    if((int)(fd = alipay_open_rsvd_part(file_name)) <= 0){
        printf("fail to open file\n");
        return false;
    }

    if(alipay_write_rsvd_part(fd, TEST_FILE_DATA, strlen(TEST_FILE_DATA)+1) != 0){
        printf("fail to write\n");
        return false;
    }

    if(alipay_close_rsvd_part(fd) != 0){
        printf("fail to close\n");
        return false;
    }

    fd = NULL;

    uint8_t buf_temp[128] = {0,};
    uint32_t len_buf_temp = sizeof(buf_temp);
    if((int)(fd = alipay_open_rsvd_part(file_name)) <= 0){
        printf("fail to open file\n");
        return false;
    }
    if((alipay_read_rsvd_part(fd, buf_temp, &len_buf_temp) != 0) || (len_buf_temp == 0) || (len_buf_temp != (strlen(TEST_FILE_DATA)+1))){
        printf("fail to read[%d]\n", len_buf_temp);
        return false;
    }

    if(alipay_clear_rsvd_part() < 0){
        printf("alipay_clear_rsvd_part error\n");
        return false;
    }

    if(alipay_access_rsvd_part(file_name) != 0){
        ALIPAY_TEST_LOG("fail to clear file\n");
        return false;
    }
    printf("---->%s OK\n", __func__);
    return true;
}

TEST_CASE(file_delete_and_access){
    TEST_TAG;

    printf("%s\n", __func__);
    char file_name[128];
    strncpy(file_name, TEST_FILE_NAME, sizeof(file_name) - 1);
    file_name[127] = '\0'; // 确保字符串以'\0'结尾

    void* fd = NULL;
    if((int)(fd = alipay_open_rsvd_part(file_name)) <= 0){
        ALIPAY_TEST_LOG("fail to open file");
        return false;
    }
    if(alipay_write_rsvd_part(fd, TEST_FILE_DATA, strlen(TEST_FILE_DATA)+1) != 0){
        ALIPAY_TEST_LOG("fail to write");
        return false;
    }
    if(alipay_close_rsvd_part(fd) != 0){
        ALIPAY_TEST_LOG("fail to close");
        return false;
    }

    if(alipay_access_rsvd_part(file_name) != 1){
        ALIPAY_TEST_LOG("alipay_access_rsvd_part error");
        return false;
    }

    if(alipay_remove_rsvd_part(file_name) < 0){
        ALIPAY_TEST_LOG("fail to remove file");
        return false;
    }

    if(alipay_access_rsvd_part(file_name) != 0){
        ALIPAY_TEST_LOG("fail to access file");
        return false;
    }

    return true;
}

int my_file_delete_and_access(void)
{
    printf("%s\n", __func__);
    char file_name[128];
    strncpy(file_name, TEST_FILE_NAME, sizeof(file_name) - 1);
    file_name[127] = '\0'; // 确保字符串以'\0'结尾

    void* fd = NULL;
    if((int)(fd = alipay_open_rsvd_part(file_name)) <= 0){
        printf("fail to open file\n");
        return false;
    }
    if(alipay_write_rsvd_part(fd, TEST_FILE_DATA, strlen(TEST_FILE_DATA)+1) != 0){
        printf("fail to write\n");
        return false;
    }
    if(alipay_close_rsvd_part(fd) != 0){
        printf("fail to close\n");
        return false;
    }

    if(alipay_access_rsvd_part(file_name) != 1){
        printf("alipay_access_rsvd_part error\n");
        return false;
    }

    if(alipay_remove_rsvd_part(file_name) < 0){
        printf("fail to remove file\n");
        return false;
    }

    if(alipay_access_rsvd_part(file_name) != 0){
        printf("fail to access file\n");
        return false;
    }

    printf("---->%s OK\n", __func__);
    return true;
}

int initializeFileSystem(void);

int test_file(void)
{
    bool ret;
    initializeFileSystem();
    TEST_IT(file_read);
    if(ret == false){
        return -1;
    }

    TEST_IT(file_write_and_read);
    if(ret == false){
        return -2;
    }
    TEST_IT(file_delete_and_access);
    if(ret == false){
        return -3;
    }
    printf("---->%s OK\n", __func__);
    return 0;

}

void test_quick_js(void)
{
    bool res = js_engine_init();
    printf("res1:%d\n", res);

    // char* eval_buf = "function getCode(params) {\n   callNative(\'log\', {\'tag\':\'ANX\', \'message\':\'genTime:\'});\n}";
//     char* eval_buf = "function dateFmt(date) {\r\n    var yyyy = \'00000\' + date.getFullYear() ;\r\n    var MM = \'00000\' + (date.getMonth() + 1);\r\n    var dd = \'00000\' + date.getDate();\r\n    var hh = \'00000\' + date.getHours();\r\n    var mm = \'00000\' + date.getMinutes();\r\n    var ss = \'00000\' + date.getSeconds();\r\n    return (yyyy.substr(-2) + MM.substr(-2)+  dd.substr(-2)+ hh.substr(-2)+ mm.substr(-2) + ss.substr(-2));\r\n}\r\n\r\nfunction getCode(params) {\r\n    var genTime = dateFmt(new Date(parseInt(callNative(\'utcTime\'))));\r\n   callNative(\'log\', {\'tag\':\'ANX\', \'message\':\'genTime:\'+genTime});\r\n}";
//     char* eval_buf = "function dateFmt(date) {\r\n    var yyyy = \'00000\' + date.getFullYear() ;\r\n    var MM = \'00000\' + (date.getMonth() + 1);\r\n    var dd = \'00000\' + date.getDate();\r\n    var hh = \'00000\' + date.getHours();\r\n    var mm = \'00000\' + date.getMinutes();\r\n    var ss = \'00000\' + date.getSeconds();\r\n    return (yyyy.substr(-2) + MM.substr(-2)+  dd.substr(-2)+ hh.substr(-2)+ mm.substr(-2) + ss.substr(-2));\r\n}\r\n\r\nfunction getCode(params) {\r\n    var genTime = dateFmt(new Date(parseInt(\'1715090896123\')));\r\n   callNative(\'log\', {\'tag\':\'ANX\', \'message\':\'genTime:\'+genTime});\r\n}";
//     char* eval_buf = "function dateFmt(date) {\r\n    var yyyy = \'00000\' + date.getFullYear() ;\r\n    var MM = \'00000\' + (date.getMonth() + 1);\r\n    var dd = \'00000\' + date.getDate();\r\n    var hh = \'00000\' + date.getHours();\r\n    var mm = \'00000\' + date.getMinutes();\r\n    var ss = \'00000\' + date.getSeconds();\r\n    return (yyyy.substr(-2) + MM.substr(-2)+  dd.substr(-2)+ hh.substr(-2)+ mm.substr(-2) + ss.substr(-2));\r\n}\r\n\r\nfunction getCode(params) {\r\n    var genTime = dateFmt(new Date(1715090896123));\r\n   callNative(\'log\', {\'tag\':\'ANX\', \'message\':\'genTime:\'+genTime});\r\n}";
    char* eval_buf = "function getCode(params) {\r\n    var genTime = new Date(1715090896123).toString();\r\n   callNative(\'log\', {\'tag\':\'ANX\', \'message\':\'genTime:\'+genTime});\r\n}";
//     char* eval_buf = "function getCode(params) {\r\n   var now = parseInt(\'1696845072123\' \/ 1000);\r\n   callNative(\'log\', {\'tag\':\'ANX\', \'message\':\'now:\'+now});\r\n}";

    char *eval_package = ab_malloc(4096);
    memset(eval_package, 0, 4096);
    strcpy(eval_package,BRIDGECODE);
    strcat(eval_package,";");
    strcat(eval_package,eval_buf);
    strcat(eval_package,CALLCODE);
    res = js_engine_eval_script(eval_package, strlen(eval_package));
    printf("res2:%d\n", res);
    js_engine_destroy();

    ab_free(eval_package);
    while(1) {
        WDT_CLR();
    }
}

#endif
