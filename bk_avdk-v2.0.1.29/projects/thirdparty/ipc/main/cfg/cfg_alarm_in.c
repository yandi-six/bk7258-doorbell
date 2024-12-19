/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_alarm_in.h"

SDK_NET_ALARM_IN_CFG runAlarmInCfg;

SDK_CFG_MAP alarmInMap[] = {
    {"alarmIn.channel",      &(runAlarmInCfg.channel),            SDK_CFG_DATA_TYPE_S32,   "0", "rw", 0, 3,    NULL},
    {"alarmIn.alarmName",    &(runAlarmInCfg.alarmInName),        SDK_CFG_DATA_TYPE_STRING,"alarm", "rw", 1, MAX_STR_LEN_32, NULL},
    {"alarmIn.defaultState", &(runAlarmInCfg.defaultState),       SDK_CFG_DATA_TYPE_S8,    "0", "rw", 0, 1, "0-low 1-high"},
    {"alarmIn.activeState",  &(runAlarmInCfg.activeState),        SDK_CFG_DATA_TYPE_S8,    "0", "rw", 0, 1, "0-low 1-high"},
    {NULL,},
};



void AlarmInCfgPrint()
{
    webcam_debug("*************** Alarm **************");

    webcam_debug("alarm in:");
    CfgPrintMap(alarmInMap);

    webcam_debug("*************** Alarm **************");
}


int AlarmInCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "alarmin", alarmInMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(ALARMIN_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", ALARMIN_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int AlarmInCfgLoadDefValue()
{
    CfgLoadDefValue(alarmInMap);


    return 0;
}

int AlarmInCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(ALARMIN_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", ALARMIN_CFG_FILE);
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

    CfgParseCjson(json, "alarmin", alarmInMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    AlarmInCfgLoadDefValue();
    AlarmInCfgSave();
    return 0;
}
cJSON * AlarmInCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(ALARMIN_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", ALARMIN_CFG_FILE);
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
int AlarmInLoadActionsDefValue(SDK_ACTION *action){
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
int AlarmInJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(ALARMIN_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", ALARMIN_CFG_FILE);
        return -1;
    }

    free(out);
    AlarmInCfgLoad();
    return 0;
}

