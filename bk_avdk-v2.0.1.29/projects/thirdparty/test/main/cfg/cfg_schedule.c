/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_schedule.h"
#include "sdk_log.h"
SDK_NET_SCHEDULE_CFG runScheduleCfg;



SDK_CFG_MAP scheduleMap[] = {
    {"schedule.enable",            &(runScheduleCfg.enable),        SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    //{"schedule.scheduleTime",      &(runScheduleCfg.scheduleTime[0][0]),    SDK_CFG_DATA_TYPE_STIME, "0", "rw", 0, 0, NULL},
    //{"schedule.actions", &(runScheduleCfg.actions[0]), SDK_CFG_DATA_TYPE_ACTION, "0", "rw", 0, 84, NULL},
    {NULL,},

};


void ScheduleCfgPrint()
{
   // printf("********** Pir *********\n");
    CfgPrintMap(scheduleMap);
   // printf("********** Pir *********\n\n");
}

int ScheduleCfgSave()
{
    int ret = CfgSave(SCHEDULE_CFG_FILE, "schedule", scheduleMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", SCHEDULE_CFG_FILE);
        return -1;
    }

    return 0;
}

int ScheduleCfgLoad()
{
    int ret = CfgLoad(SCHEDULE_CFG_FILE, "schedule", scheduleMap);
    if (ret != 0) {
        webcam_error("CfgLoad %s error.", SCHEDULE_CFG_FILE);
        return -1;
    }

    return 0;
}

int ScheduleCfgLoadDefValue()
{
    CfgLoadDefValue(scheduleMap);

    return 0;
}
int ScheduleLoadActionsDefValue(SDK_ACTION *action){
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
cJSON* ScheduleCfgLoadJson(){
    char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(SCHEDULE_CFG_FILE);
    if (data == NULL) {
        webcam_debug("load %s error, so to load default cfg param.\n", SCHEDULE_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //ŽÓÅäÖÃÎÄŒþœâÎöcjsonÊ§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
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
int ScheduleJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(SCHEDULE_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", SCHEDULE_CFG_FILE);
        return -1;
    }

    free(out);
    ScheduleCfgLoad();
    return 0;
}
