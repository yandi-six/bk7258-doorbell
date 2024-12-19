/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_email.h"

SDK_NET_EMAIL_CFG runEmailCfg;

SDK_CFG_MAP emailMap[] = {
    {"enable",    	 &(runEmailCfg.enableEmail),    SDK_CFG_DATA_TYPE_S32,     "1",                "rw", 0, 1,      NULL},
	{"attachPicture",&(runEmailCfg.attachPicture),	SDK_CFG_DATA_TYPE_S32,	  "1",				  "rw", 0, 1,	   NULL},
    {"smtpServer",   &(runEmailCfg.smtpServer),     SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64, NULL},
	{"smtpPort",	 &(runEmailCfg.smtpPort), 		SDK_CFG_DATA_TYPE_S32,	  "25",    "rw", 1, 65535, NULL},
	{"pop3Server",	 &(runEmailCfg.pop3Server), 	SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64, NULL},
	{"pop3Port",	 &(runEmailCfg.pop3Port),		SDK_CFG_DATA_TYPE_S32,	  "110",   "rw", 1, 65535, NULL},
    {"cryptionType", &(runEmailCfg.encryptionType), SDK_CFG_DATA_TYPE_S32,     "0",                "rw",  0, 2,	   NULL},
    {"user",       	 &(runEmailCfg.eMailUser),      SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},
    {"password",	 &(runEmailCfg.eMailPass),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,	   NULL},
	{"fromAddrList", &(runEmailCfg.fromAddr),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},

	{"toAddrList0", &(runEmailCfg.toAddrList0),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList1", &(runEmailCfg.toAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList2", &(runEmailCfg.toAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList3", &(runEmailCfg.toAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList0", &(runEmailCfg.ccAddrList0), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList1", &(runEmailCfg.ccAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList2", &(runEmailCfg.ccAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList3", &(runEmailCfg.ccAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList0",&(runEmailCfg.bccAddrList0), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList1",&(runEmailCfg.bccAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList2",&(runEmailCfg.bccAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList3",&(runEmailCfg.bccAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
    {NULL,},
};



void EmailCfgPrint()
{
    CfgPrintMap(emailMap);
}


int EmailCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "email", emailMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(EMAIL_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", EMAIL_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int EmailCfgLoadDefValue()
{
    CfgLoadDefValue(emailMap);


    return 0;
}

int EmailCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(EMAIL_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", EMAIL_CFG_FILE);
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

    CfgParseCjson(json, "email", emailMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    EmailCfgLoadDefValue();
    EmailCfgSave();
    return 0;
}
cJSON * EmailCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(EMAIL_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", EMAIL_CFG_FILE);
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
int EmailJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(EMAIL_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", EMAIL_CFG_FILE);
        return -1;
    }

    free(out);
    EmailCfgLoad();
    return 0;
}


