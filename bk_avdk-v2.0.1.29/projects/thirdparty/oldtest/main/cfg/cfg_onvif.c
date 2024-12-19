/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_onvif.h"

SDK_NET_ONVIF_CFG runOnvifCfg;

SDK_CFG_MAP OnvifMap[] = {
    {"httpEnable",      &(runOnvifCfg.httpEnable),            SDK_CFG_DATA_TYPE_S8,    "1",     "rw", 0, 1,  NULL},
    {"httpPort",  &(runOnvifCfg.httpPort),        SDK_CFG_DATA_TYPE_S32,    "8080",   "rw", 0, 65536, NULL},
    {"httpsEnable",      &(runOnvifCfg.httpsEnable),            SDK_CFG_DATA_TYPE_S8,    "1",     "rw", 0, 1,  NULL},
    {"httpsPort",  &(runOnvifCfg.httpsPort),        SDK_CFG_DATA_TYPE_S32,     "8443",   "rw", 0, 65536, NULL},
    {NULL,},
};



void OnvifCfgPrint()
{
    CfgPrintMap(OnvifMap);
}


int OnvifCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "onvif", OnvifMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(ONVIF_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", ONVIF_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int OnvifCfgLoadDefValue()
{
    CfgLoadDefValue(OnvifMap);


    return 0;
}

int OnvifCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(ONVIF_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", ONVIF_CFG_FILE);
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

    CfgParseCjson(json, "onvif", OnvifMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    OnvifCfgLoadDefValue();
    OnvifCfgSave();
    return 0;
}
cJSON * OnvifCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(ONVIF_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", ONVIF_CFG_FILE);
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
int OnvifJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(ONVIF_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", ONVIF_CFG_FILE);
        return -1;
    }

    free(out);
    OnvifCfgLoad();
    return 0;
}


