/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_record.h"
#include "sdk_log.h"
SDK_NET_RECORD_CFG runRecordCfg;

SDK_CFG_MAP recordMap[] = {
    {"enable",           &runRecordCfg.enable,                SDK_CFG_DATA_TYPE_S32,   "1",   "rw", 0, 1,   NULL},
    {"stream_no",        &runRecordCfg.stream_no,             SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 3,   "0-3"},
    {"recordMode",       &runRecordCfg.recordMode,            SDK_CFG_DATA_TYPE_S32,   "3",   "rw", 0, 3,   NULL},
    {"preRecordTime",  &runRecordCfg.preRecordTime,        SDK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 10,  NULL},
    {"audioRecEnable", &runRecordCfg.audioRecEnable,       SDK_CFG_DATA_TYPE_S32,   "1",   "rw", 0, 1,   NULL},
    {"recAudioType",   &runRecordCfg.recAudioType,         SDK_CFG_DATA_TYPE_S32,   "2",   "rw", 0, 3,   NULL},
    {"recordLen",      &runRecordCfg.recordLen,            SDK_CFG_DATA_TYPE_S32,   "5",  "rw", 1, 1440, NULL},
    {"recycleRecord",  &runRecordCfg.recycleRecord,        SDK_CFG_DATA_TYPE_S32,   "1",   "rw", 0, 1,   NULL},
    {"scheduleTime",     &(runRecordCfg.scheduleTime[0][0]),  SDK_CFG_DATA_TYPE_STIME, "0",   "rw", 0, 0,  NULL},
    {NULL,},
};

void RecordCfgPrint()
{
    //printf("********** Record *********\n");
    CfgPrintMap(recordMap);
    //printf("********** Record *********\n\n");
}

int RecordCfgSave()
{
    int ret = CfgSave(RECORD_CFG_FILE, "record", recordMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", RECORD_CFG_FILE);
        return -1;
    }
    return 0;
}

int RecordCfgLoad()
{
    int ret = CfgLoad(RECORD_CFG_FILE, "record", recordMap);
    if (ret != 0) {
        webcam_error("CfgLoad %s error.", RECORD_CFG_FILE);
        return -1;
    }
    return 0;
}

cJSON * RecordCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(RECORD_CFG_FILE);
    if (data == NULL) {
        //ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        webcam_debug("load %s error, so to load default cfg param.\n", RECORD_CFG_FILE);
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


int RecordCfgLoadDefValue()
{
    CfgLoadDefValue(recordMap);

    return 0;
}
int RecordJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(RECORD_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", RECORD_CFG_FILE);
        return -1;
    }

    free(out);
    RecordCfgLoad();
    return 0;
}
