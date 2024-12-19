/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_voicealarm.h"

SDK_NET_VOICEALARM_CFG runVoiceAlarmCfg;

SDK_CFG_MAP voiceAlarmMap[] = {
    {"enable",    	 &(runVoiceAlarmCfg.enableEmail),    SDK_CFG_DATA_TYPE_S32,     "1",                "rw", 0, 1,      NULL},
	{"attachPicture",&(runVoiceAlarmCfg.attachPicture),	SDK_CFG_DATA_TYPE_S32,	  "1",				  "rw", 0, 1,	   NULL},
    {"smtpServer",   &(runVoiceAlarmCfg.smtpServer),     SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64, NULL},
	{"smtpPort",	 &(runVoiceAlarmCfg.smtpPort), 		SDK_CFG_DATA_TYPE_S32,	  "25",    "rw", 1, 65535, NULL},
	{"pop3Server",	 &(runVoiceAlarmCfg.pop3Server), 	SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64, NULL},
	{"pop3Port",	 &(runVoiceAlarmCfg.pop3Port),		SDK_CFG_DATA_TYPE_S32,	  "110",   "rw", 1, 65535, NULL},
    {"cryptionType", &(runVoiceAlarmCfg.encryptionType), SDK_CFG_DATA_TYPE_S32,     "0",                "rw",  0, 2,	   NULL},
    {"user",       	 &(runVoiceAlarmCfg.eMailUser),      SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},
    {"password",	 &(runVoiceAlarmCfg.eMailPass),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,	   NULL},
	{"fromAddrList", &(runVoiceAlarmCfg.fromAddr),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},

	{"toAddrList0", &(runVoiceAlarmCfg.toAddrList0),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList1", &(runVoiceAlarmCfg.toAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList2", &(runVoiceAlarmCfg.toAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList3", &(runVoiceAlarmCfg.toAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList0", &(runVoiceAlarmCfg.ccAddrList0), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList1", &(runVoiceAlarmCfg.ccAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList2", &(runVoiceAlarmCfg.ccAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList3", &(runVoiceAlarmCfg.ccAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList0",&(runVoiceAlarmCfg.bccAddrList0), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList1",&(runVoiceAlarmCfg.bccAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList2",&(runVoiceAlarmCfg.bccAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList3",&(runVoiceAlarmCfg.bccAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
    {NULL,},
};



void VoiceAlarmCfgPrint()
{
    CfgPrintMap(voiceAlarmMap);
}


int VoiceAlarmCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "voicealarm", voiceAlarmMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(VOICEALARM_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", VOICEALARM_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int VoiceAlarmCfgLoadDefValue()
{
    CfgLoadDefValue(voiceAlarmMap);


    return 0;
}

int VoiceAlarmCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(VOICEALARM_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", VOICEALARM_CFG_FILE);
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

    CfgParseCjson(json, "voicealarm", voiceAlarmMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    VoiceAlarmCfgLoadDefValue();
    VoiceAlarmCfgSave();
    return 0;
}
cJSON * VoiceAlarmCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(VOICEALARM_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", VOICEALARM_CFG_FILE);
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
int VoiceAlarmJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(VOICEALARM_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", VOICEALARM_CFG_FILE);
        return -1;
    }

    free(out);
    VoiceAlarmCfgLoad();
    return 0;
}

