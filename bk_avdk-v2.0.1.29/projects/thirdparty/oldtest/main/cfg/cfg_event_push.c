/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_event_push.h"

SDK_NET_EVENT_PUSH_CFG runEventPushCfg;

SDK_CFG_MAP eventPushMap[] = {
    {"enable",    	 &(runEventPushCfg.enableEmail),    SDK_CFG_DATA_TYPE_S32,     "1",                "rw", 0, 1,      NULL},
	{"attachPicture",&(runEventPushCfg.attachPicture),	SDK_CFG_DATA_TYPE_S32,	  "1",				  "rw", 0, 1,	   NULL},
    {"smtpServer",   &(runEventPushCfg.smtpServer),     SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64, NULL},
	{"smtpPort",	 &(runEventPushCfg.smtpPort), 		SDK_CFG_DATA_TYPE_S32,	  "25",    "rw", 1, 65535, NULL},
	{"pop3Server",	 &(runEventPushCfg.pop3Server), 	SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64, NULL},
	{"pop3Port",	 &(runEventPushCfg.pop3Port),		SDK_CFG_DATA_TYPE_S32,	  "110",   "rw", 1, 65535, NULL},
    {"cryptionType", &(runEventPushCfg.encryptionType), SDK_CFG_DATA_TYPE_S32,     "0",                "rw",  0, 2,	   NULL},
    {"user",       	 &(runEventPushCfg.eMailUser),      SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},
    {"password",	 &(runEventPushCfg.eMailPass),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,	   NULL},
	{"fromAddrList", &(runEventPushCfg.fromAddr),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},

	{"toAddrList0", &(runEventPushCfg.toAddrList0),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList1", &(runEventPushCfg.toAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList2", &(runEventPushCfg.toAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList3", &(runEventPushCfg.toAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList0", &(runEventPushCfg.ccAddrList0), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList1", &(runEventPushCfg.ccAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList2", &(runEventPushCfg.ccAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList3", &(runEventPushCfg.ccAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList0",&(runEventPushCfg.bccAddrList0), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList1",&(runEventPushCfg.bccAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList2",&(runEventPushCfg.bccAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList3",&(runEventPushCfg.bccAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
    {NULL,},
};



void EventPushCfgPrint()
{
    CfgPrintMap(eventPushMap);
}


int EventPushCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "eventpush", eventPushMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(EVENT_PUSH_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", EVENT_PUSH_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int EventPushCfgLoadDefValue()
{
    CfgLoadDefValue(eventPushMap);


    return 0;
}

int EventPushCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(EVENT_PUSH_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", EVENT_PUSH_CFG_FILE);
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

    CfgParseCjson(json, "eventpush", eventPushMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    EventPushCfgLoadDefValue();
    EventPushCfgSave();
    return 0;
}
cJSON * EventPushCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(EVENT_PUSH_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", EVENT_PUSH_CFG_FILE);
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
int EventPushJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(EVENT_PUSH_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", EVENT_PUSH_CFG_FILE);
        return -1;
    }

    free(out);
    EventPushCfgLoad();
    return 0;
}

