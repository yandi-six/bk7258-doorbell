/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_webrtc.h"

SDK_NET_WEBRTC_CFG runWebRTCCfg;

SDK_CFG_MAP webrtcMap[] = {
    {"stunport",      &(runWebRTCCfg.stunport),            SDK_CFG_DATA_TYPE_S32,    "3678",   "rw", 0, 65536, NULL},
    {"activeState",  &(runWebRTCCfg.activeState),        SDK_CFG_DATA_TYPE_S8,    "1",       "rw", 0, 1,  NULL},
    {NULL,},
};



void WebRTCCfgPrint()
{
    CfgPrintMap(webrtcMap);
}


int WebRTCCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "webrtc", webrtcMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(WEBRTC_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", WEBRTC_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int WebRTCCfgLoadDefValue()
{
    CfgLoadDefValue(webrtcMap);


    return 0;
}

int WebRTCCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(WEBRTC_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", WEBRTC_CFG_FILE);
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

    CfgParseCjson(json, "webrtc", webrtcMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    WebRTCCfgLoadDefValue();
    WebRTCCfgSave();
    return 0;
}
cJSON * WebRTCCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(WEBRTC_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", WEBRTC_CFG_FILE);
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
int WebRTCJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(WEBRTC_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", WEBRTC_CFG_FILE);
        return -1;
    }

    free(out);
    WebRTCCfgLoad();
    return 0;
}


