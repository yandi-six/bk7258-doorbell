/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_alarm_out.h"

SDK_NET_ALARM_OUT_CFG runAlarmOutCfg;



SDK_CFG_MAP alarmOutMap[] = {
    {"alarmOut.channel",       &(runAlarmOutCfg.channel),            SDK_CFG_DATA_TYPE_S32,   "0", "rw", 0, 3,    NULL},
    {"alarmOut.alarmOutName",  &(runAlarmOutCfg.alarmOutName),       SDK_CFG_DATA_TYPE_S8,    "0", "rw", 1, MAX_STR_LEN_32, NULL},
    {"alarmOut.defaultState",  &(runAlarmOutCfg.defaultState),       SDK_CFG_DATA_TYPE_S8,    "0", "rw", 0, 1, "0-low 1-high"},
    {"alarmOut.activeState",   &(runAlarmOutCfg.activeState),        SDK_CFG_DATA_TYPE_S8,    "0", "rw", 0, 1, "0-low 1-high"},
    {"alarmOut.powerOnState",  &(runAlarmOutCfg.powerOnState),       SDK_CFG_DATA_TYPE_S8,    "0", "rw", 0, 1, "0-pulse 1-continuous"},
    {"alarmOut.pulseDuration", &(runAlarmOutCfg.pulseDuration),      SDK_CFG_DATA_TYPE_S32,   "5000", "rw", 1000, 10000, NULL},
    {NULL,},
};



void AlarmOutCfgPrint()
{
    webcam_debug("*************** Alarm **************");

    webcam_debug("alarm out:");
    CfgPrintMap(alarmOutMap);

    webcam_debug("*************** Alarm **************");
}


int AlarmOutCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目


    CfgAddCjson(root, "alarmout", alarmOutMap);


    out = cJSON_Print(root);

    int ret = CfgWriteToFile(ALARMOUT_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", ALARMOUT_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int AlarmOutCfgLoadDefValue()
{

    CfgLoadDefValue(alarmOutMap);


    return 0;
}

int AlarmOutCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(ALARMOUT_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", ALARMOUT_CFG_FILE);
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

 
    CfgParseCjson(json, "alarmout", alarmOutMap);


    cJSON_Delete(json);
    free(data);
    return 0;

err:
    AlarmOutCfgLoadDefValue();
    AlarmOutCfgSave();
    return 0;
}
cJSON * AlarmOutCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(ALARMOUT_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", ALARMOUT_CFG_FILE);
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
int AlarmOutJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(ALARMOUT_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", ALARMOUT_CFG_FILE);
        return -1;
    }

    free(out);
    AlarmOutCfgLoad();
    return 0;
}
