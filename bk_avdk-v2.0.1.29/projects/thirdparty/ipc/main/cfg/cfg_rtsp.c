/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_rtsp.h"

SDK_NET_RTSP_CFG runRtspCfg;

SDK_CFG_MAP rtspMap[] = {
    {"enable",      &(runRtspCfg.enable),            SDK_CFG_DATA_TYPE_S8,     "1",     "rw", 0, 1,  NULL},
    {"port",  &(runRtspCfg.port),        SDK_CFG_DATA_TYPE_S32,    "554",  "rw", 0, 65536, NULL},
    {NULL,},
};



void RtspCfgPrint()
{
    CfgPrintMap(rtspMap);
}


int RtspCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "rtsp", rtspMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(RTSP_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", RTSP_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int RtspCfgLoadDefValue()
{
    CfgLoadDefValue(rtspMap);


    return 0;
}

int RtspCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(RTSP_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", RTSP_CFG_FILE);
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

    CfgParseCjson(json, "rtsp", rtspMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    RtspCfgLoadDefValue();
    RtspCfgSave();
    return 0;
}
cJSON * RtspCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(RTSP_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", RTSP_CFG_FILE);
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
int RtspJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(RTSP_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", RTSP_CFG_FILE);
        return -1;
    }

    free(out);
    RtspCfgLoad();
    return 0;
}

