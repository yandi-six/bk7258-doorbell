/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_md.h"
#include "sdk_log.h"
SDK_NET_MD_CFG runMdCfg;

#define MAX_MD_GRID_NUM 4


 SDK_CFG_MAP mdMap[] = {
    {"md.channel",           &(runMdCfg.channel),       SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 3, NULL},
    {"md.enable",            &(runMdCfg.enable),        SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    {"md.sensitive",         &(runMdCfg.sensitive),     SDK_CFG_DATA_TYPE_S32, "50", "rw", 0, 100, "0-100"},
    {"md.compensation",      &(runMdCfg.compensation),  SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1, "0，1"},
    {"md.detectionType",     &(runMdCfg.detectionType), SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, "0 grid, 1 region"},

    {"md.grid.row",          &(runMdCfg.mdGrid.row),         SDK_CFG_DATA_TYPE_S32,  "15", "rw", 0, 100, NULL},
    {"md.grid.column",       &(runMdCfg.mdGrid.column),      SDK_CFG_DATA_TYPE_S32,  "22", "rw", 0, 100, NULL},
    {"md.grid.granularity",  &(runMdCfg.mdGrid.granularity), SDK_CFG_DATA_TYPE_S8,   "0",  "rw", 1, SDK_NET_MD_GRID_ROW * SDK_NET_MD_GRID_COLUMN + 1, NULL},

    {"md.region0.enable",    &(runMdCfg.mdRegion[0].enable),    SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1,    NULL},
    {"md.region0.x",         &(runMdCfg.mdRegion[0].x),         SDK_CFG_DATA_TYPE_S32, "0", "rw", 1, 1920, NULL},
    {"md.region0.y",         &(runMdCfg.mdRegion[0].y),         SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1280, NULL},
    {"md.region0.width",     &(runMdCfg.mdRegion[0].width),     SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1920, NULL},
    {"md.region0.height",    &(runMdCfg.mdRegion[0].height),    SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1280, NULL},

    {"md.region1.enable",    &(runMdCfg.mdRegion[1].enable),    SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1,    NULL},
    {"md.region1.x",         &(runMdCfg.mdRegion[1].x),         SDK_CFG_DATA_TYPE_S32, "0", "rw", 1, 1920, NULL},
    {"md.region1.y",         &(runMdCfg.mdRegion[1].y),         SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1280, NULL},
    {"md.region1.width",     &(runMdCfg.mdRegion[1].width),     SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1920, NULL},
    {"md.region1.height",    &(runMdCfg.mdRegion[1].height),    SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1280, NULL},

    {"md.region2.enable",    &(runMdCfg.mdRegion[2].enable),    SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1,    NULL},
    {"md.region2.x",         &(runMdCfg.mdRegion[2].x),         SDK_CFG_DATA_TYPE_S32, "0", "rw", 1, 1920, NULL},
    {"md.region2.y",         &(runMdCfg.mdRegion[2].y),         SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1280, NULL},
    {"md.region2.width",     &(runMdCfg.mdRegion[2].width),     SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1920, NULL},
    {"md.region2.height",    &(runMdCfg.mdRegion[2].height),    SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1280, NULL},

    {"md.region3.enable",    &(runMdCfg.mdRegion[3].enable),    SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1,    NULL},
    {"md.region3.x",         &(runMdCfg.mdRegion[3].x),         SDK_CFG_DATA_TYPE_S32, "0", "rw", 1, 1920, NULL},
    {"md.region3.y",         &(runMdCfg.mdRegion[3].y),         SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1280, NULL},
    {"md.region3.width",     &(runMdCfg.mdRegion[3].width),     SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1920, NULL},
    {"md.region3.height",    &(runMdCfg.mdRegion[3].height),    SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1280, NULL},

    {"md.schedule_mode",     &(runMdCfg.schedule_mode),         SDK_CFG_DATA_TYPE_S32,   "1", "rw", 0, 3, NULL},
    //{"md.scheduleTime",      &(runMdCfg.scheduleTime[0][0]),    SDK_CFG_DATA_TYPE_STIME, "0", "rw", 0, 0, NULL},
    //{"md.actions", &(runAlarmCfg.alarmIn.actions[0]), SDK_CFG_DATA_TYPE_ACTION, "0", "rw", 0, 84, NULL},


    {NULL,},

};


void MdCfgPrint()
{
   // printf("********** Md *********\n");
    CfgPrintMap(mdMap);
   // printf("********** Md *********\n\n");
}

int MdCfgSave()
{
    int ret = CfgSave(MD_CFG_FILE, "motiondetect", mdMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", MD_CFG_FILE);
        return -1;
    }

    return 0;
}

int MdCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(MD_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", MD_CFG_FILE);
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

    CfgParseCjson(json, "motiondetect", mdMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    MdCfgLoadDefValue();
    MdCfgSave();
    return 0;
}

int MdCfgLoadDefValue()
{
    CfgLoadDefValue(mdMap);

    return 0;
}
cJSON * MdCfgLoadJson(){
   char *data = NULL;
    data = CfgReadFromFile(MD_CFG_FILE);
    if (data == NULL) {
        webcam_debug("load %s error, so to load default cfg param.\n", MD_CFG_FILE);
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
int MdLoadActionsDefValue(SDK_ACTION *action){
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
int MdJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(MD_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", MD_CFG_FILE);
        return -1;
    }

    free(out);
    MdCfgLoad();
    return 0;
}

