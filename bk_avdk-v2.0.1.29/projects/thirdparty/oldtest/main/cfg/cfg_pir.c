/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_pir.h"
#include "sdk_log.h"
SDK_NET_PIR_CFG runPirCfg;



SDK_CFG_MAP pirMap[] = {
    {"pir.enable",            &(runPirCfg.enable),        SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    {NULL,},

};


void PirCfgPrint()
{
   // printf("********** Pir *********\n");
    CfgPrintMap(pirMap);
   // printf("********** Pir *********\n\n");
}

int PirCfgSave()
{
    int ret = CfgSave(PIR_CFG_FILE, "pir", pirMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", PIR_CFG_FILE);
        return -1;
    }

    return 0;
}

int PirCfgLoad()
{
    int ret = CfgLoad(PIR_CFG_FILE, "pir", pirMap);
    if (ret != 0) {
        webcam_error("CfgLoad %s error.", PIR_CFG_FILE);
        return -1;
    }

    char *data = NULL;
    data = CfgReadFromFile(PIR_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", PIR_CFG_FILE);
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

    CfgParseCjson(json, "pir", pirMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    PirCfgLoadDefValue();
    PirCfgSave();
    return 0;


}

int PirCfgLoadDefValue()
{
    CfgLoadDefValue(pirMap);

    return 0;
}
cJSON* PirCfgLoadJson(){
    char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(PIR_CFG_FILE);
    if (data == NULL) {
        webcam_debug("load %s error, so to load default cfg param.\n", PIR_CFG_FILE);
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
int PirLoadActionsDefValue(SDK_ACTION *action){
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
int PirJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(PIR_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", PIR_CFG_FILE);
        return -1;
    }

    free(out);
    PirCfgLoad();
    return 0;
}
