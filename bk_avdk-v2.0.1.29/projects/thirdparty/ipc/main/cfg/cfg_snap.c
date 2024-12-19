/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_snap.h"
#include "sdk_log.h"
SDK_NET_SNAP_CFG runSnapCfg;

SDK_CFG_MAP snapTimerMap[] = {
    {"enable",            &runSnapCfg.timer_snap.enable,               SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1, NULL},
    {"scheduleTime",      &(runSnapCfg.timer_snap.scheduleTime[0][0]), SDK_CFG_DATA_TYPE_STIME, "0",   "rw", 0, 28, NULL},
    {"interval",          &runSnapCfg.timer_snap.interval,             SDK_CFG_DATA_TYPE_S32,   "10000",   "rw", 0, 1000000, NULL},
    {"nums",              &runSnapCfg.timer_snap.nums,                 SDK_CFG_DATA_TYPE_S32,   "1",   "rw", 1, 30,   NULL},
    {"pictureQuality",    &runSnapCfg.timer_snap.pictureQuality,       SDK_CFG_DATA_TYPE_S32,   "2",   "rw", 0, 4, NULL},
    {"imageSize",         &runSnapCfg.timer_snap.imageSize,            SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 4, "0: 1080p 1: 720p 2 D1 3：CIF，4：QCIF"},
    {"snapShotImageType", &(runSnapCfg.timer_snap.snapShotImageType),  SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1, "0 JPEG；1 bmp"},
    {"storagerMode",      &(runSnapCfg.timer_snap.storagerMode),       SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 2, "0 local；1 ftp; 2 local|ftp"},
    {"channelID",         &(runSnapCfg.timer_snap.channelID),          SDK_CFG_DATA_TYPE_S32,   "3",   "rw", 3, 3, "channel id for snapshort"},
    {NULL,},
};

SDK_CFG_MAP snapEventMap[] = {
	{"enable",            &runSnapCfg.event_snap.enable,               SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1, NULL},
    {"interval",          &runSnapCfg.event_snap.interval,             SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1000, NULL},
    {"nums",              &runSnapCfg.event_snap.nums,                 SDK_CFG_DATA_TYPE_S32,   "1",   "rw", 1, 30,   NULL},
    {"pictureQuality",    &runSnapCfg.event_snap.pictureQuality,       SDK_CFG_DATA_TYPE_S32,   "100", "rw", 0, 100, NULL},
    {"imageSize",         &runSnapCfg.event_snap.imageSize,            SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 4, "0: 1080p 1: 720p 2 D1 3：CIF，4：QCIF"},
    {"snapShotImageType", &(runSnapCfg.event_snap.snapShotImageType),  SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1, "0 JPEG；1 bmp"},
    {"storagerMode",      &(runSnapCfg.event_snap.storagerMode),       SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 2, "0 local；1 ftp; 2 local|ftp"},
    {"channelID",         &(runSnapCfg.event_snap.channelID),          SDK_CFG_DATA_TYPE_S32,   "3",   "rw", 3, 3, "channel id for snapshort"},
    {NULL,},
};

void SnapCfgPrint()
{
    //printf("**********Timer Snap *********\n");
    CfgPrintMap(snapTimerMap);
    //printf("**********Timer Snap *********\n\n");

	//printf("**********Event Snap *********\n");
    CfgPrintMap(snapEventMap);
    //printf("**********Event Snap *********\n\n");
}

int SnapCfgSave()
{
	cJSON *root;
    char *out;

	root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "timerSnap", snapTimerMap);
    CfgAddCjson(root, "eventSnap", snapEventMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(SNAP_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", SNAP_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);
    return 0;
}

int SnapCfgLoad()
{
	char *data = NULL;
    data = CfgReadFromFile(SNAP_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", SNAP_CFG_FILE);
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

    CfgParseCjson(json, "timerSnap", snapTimerMap);
    CfgParseCjson(json, "eventSnap", snapEventMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    SnapCfgLoadDefValue();
    SnapCfgSave();
    return 0;
}

int SnapCfgLoadDefValue()
{
    CfgLoadDefValue(snapTimerMap);
	CfgLoadDefValue(snapEventMap);
    return 0;
}
cJSON * SnapCfgLoadJson()
{
    char *data = NULL;
    data = CfgReadFromFile(SNAP_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", SNAP_CFG_FILE);
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

    CfgParseCjson(json, "timerSnap", snapTimerMap);
    CfgParseCjson(json, "eventSnap", snapEventMap);


    free(data);
    return json;

err:
    return NULL;
}
int SnapJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(SNAP_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", SNAP_CFG_FILE);
        return -1;
    }

    free(out);
    SnapCfgLoad();
    return 0;
}
