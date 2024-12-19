/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_facerecognition.h"
#include "sdk_log.h"
SDK_NET_FR_CFG runFrCfg;



SDK_CFG_MAP FrMap[] = {
    {"enable",            &(runFrCfg.enable),        SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    {"sensitive",         &(runFrCfg.sensitive),     SDK_CFG_DATA_TYPE_S32, "50", "rw", 0, 100, NULL},
    {"compensation",      &(runFrCfg.compensation),  SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1, NULL},
    {"detectionType",     &(runFrCfg.detectionType), SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    {NULL,},

};


void FrCfgPrint()
{
   // printf("********** Fr *********\n");
    CfgPrintMap(FrMap);
   // printf("********** Fr *********\n\n");
}

int FrCfgSave()
{
    int ret = CfgSave(FR_CFG_FILE, "motiondetect", FrMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", FR_CFG_FILE);
        return -1;
    }

    return 0;
}

int FrCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(FR_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", FR_CFG_FILE);
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

    CfgParseCjson(json, "motiondetect", FrMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    FrCfgLoadDefValue();
    FrCfgSave();
    return 0;
}

int FrCfgLoadDefValue()
{
    CfgLoadDefValue(FrMap);

    return 0;
}
cJSON * FrCfgLoadJson(){
   char *data = NULL;
    data = CfgReadFromFile(FR_CFG_FILE);
    if (data == NULL) {
        webcam_debug("load %s error, so to load default cfg param.\n", FR_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
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
int FrLoadActionsDefValue(SDK_ACTION *action){
	SDK_ACTION *temp = action;
	int i = 0;
	for(i =0;i< MAX_ACTIONS;i++){
	   memset(temp,0,sizeof(SDK_ACTION));
	   temp++;
        }
	action->selected = 1;
        action->index = 0;
	snprintf(action->lable,MAX_STR_LEN_32,"%s","record");
        return 1;

}
int FrJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(FR_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", FR_CFG_FILE);
        return -1;
    }

    free(out);
    FrCfgLoad();
    return 0;
}
