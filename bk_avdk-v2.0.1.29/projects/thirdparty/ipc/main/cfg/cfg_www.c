/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_www.h"

SDK_NET_WWW_CFG runWWWCfg;

SDK_CFG_MAP wwwMap[] = {
    {"httpEnable",      &(runWWWCfg.httpEnable),            SDK_CFG_DATA_TYPE_S8,   "1",       "rw", 0, 1,               NULL},
    {"httpPort",  &(runWWWCfg.httpPort),        SDK_CFG_DATA_TYPE_S32,    "80",   "rw", 0, 65536, NULL},
    {"httpsEnable",      &(runWWWCfg.httpsEnable),            SDK_CFG_DATA_TYPE_S8,   "1",       "rw", 0, 1,               NULL},
    {"httpsPort",  &(runWWWCfg.httpsPort),        SDK_CFG_DATA_TYPE_S32,   "443",   "rw", 0, 65536, NULL},
    {NULL,},
};



void WebCfgPrint()
{
    CfgPrintMap(wwwMap);
}


int WebCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "www", wwwMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(WWW_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", WWW_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int WebCfgLoadDefValue()
{
    CfgLoadDefValue(wwwMap);


    return 0;
}

int WebCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(WWW_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", WWW_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    CfgParseCjson(json, "www", wwwMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    WebCfgLoadDefValue();
    WebCfgSave();
    return 0;
}
cJSON * WebCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(WWW_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", WWW_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    free(data);
    return json;

err:
    if(data){
	free(data);
    }
    return NULL;
}


