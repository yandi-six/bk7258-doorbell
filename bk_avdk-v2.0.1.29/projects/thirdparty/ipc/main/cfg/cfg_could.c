/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_could.h"

SDK_NET_COULD_CFG runCouldCfg;

SDK_CFG_MAP couldMap[] = {
    {"enable",    	 &(runCouldCfg.enableEmail),    SDK_CFG_DATA_TYPE_S32,     "1",                "rw", 0, 1,      NULL},
	{"attachPicture",&(runCouldCfg.attachPicture),	SDK_CFG_DATA_TYPE_S32,	  "1",				  "rw", 0, 1,	   NULL},
    {"smtpServer",   &(runCouldCfg.smtpServer),     SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64, NULL},
	{"smtpPort",	 &(runCouldCfg.smtpPort), 		SDK_CFG_DATA_TYPE_S32,	  "25",    "rw", 1, 65535, NULL},
	{"pop3Server",	 &(runCouldCfg.pop3Server), 	SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64, NULL},
	{"pop3Port",	 &(runCouldCfg.pop3Port),		SDK_CFG_DATA_TYPE_S32,	  "110",   "rw", 1, 65535, NULL},
    {"cryptionType", &(runCouldCfg.encryptionType), SDK_CFG_DATA_TYPE_S32,     "0",                "rw",  0, 2,	   NULL},
    {"user",       	 &(runCouldCfg.eMailUser),      SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},
    {"password",	 &(runCouldCfg.eMailPass),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,	   NULL},
	{"fromAddrList", &(runCouldCfg.fromAddr),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},

	{"toAddrList0", &(runCouldCfg.toAddrList0),		SDK_CFG_DATA_TYPE_STRING,  "", 	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList1", &(runCouldCfg.toAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList2", &(runCouldCfg.toAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList3", &(runCouldCfg.toAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList0", &(runCouldCfg.ccAddrList0), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList1", &(runCouldCfg.ccAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList2", &(runCouldCfg.ccAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList3", &(runCouldCfg.ccAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList0",&(runCouldCfg.bccAddrList0), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList1",&(runCouldCfg.bccAddrList1), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList2",&(runCouldCfg.bccAddrList2), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList3",&(runCouldCfg.bccAddrList3), 	SDK_CFG_DATA_TYPE_STRING,  "",	   "rw", 1, MAX_STR_LEN_64,    NULL},
    {NULL,},
};



void CouldCfgPrint()
{
    CfgPrintMap(couldMap);
}


int CouldCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "could", couldMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(COULD_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", COULD_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int CouldCfgLoadDefValue()
{
    CfgLoadDefValue(couldMap);


    return 0;
}

int CouldCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(COULD_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", COULD_CFG_FILE);
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

    CfgParseCjson(json, "could", couldMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    CouldCfgLoadDefValue();
    CouldCfgSave();
    return 0;
}
cJSON * CouldCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(COULD_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", COULD_CFG_FILE);
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
int CouldJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(COULD_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", COULD_CFG_FILE);
        return -1;
    }

    free(out);
    CouldCfgLoad();
    return 0;
}

