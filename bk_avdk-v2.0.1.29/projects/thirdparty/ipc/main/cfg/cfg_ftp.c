/*!
*****************************************************************************

*****************************************************************************
*/

#include "sdk_log.h"
#include "cfg_ftp.h"

SDK_NET_FTP_CFG runFtpCfg;

SDK_CFG_MAP ftpMap[] = {
    {"enableFTP", &(runFtpCfg.enableFTP), SDK_CFG_DATA_TYPE_S32,     "0",       "rw", 0, 1,               NULL},
    {"address",   &(runFtpCfg.address),   SDK_CFG_DATA_TYPE_STRING,  "",  "rw", 1, MAX_STR_LEN_128, NULL},
    {"port",      &(runFtpCfg.port),      SDK_CFG_DATA_TYPE_S32,     "",      "rw", 0, 65536,           NULL},
    {"userName",  &(runFtpCfg.userName),  SDK_CFG_DATA_TYPE_STRING,  "",    "rw", 1, MAX_STR_LEN_64,  NULL},
    {"password",  &(runFtpCfg.password),  SDK_CFG_DATA_TYPE_STRING,  "",    "rw", 1, MAX_STR_LEN_64,  NULL},
    {"datapath",  &(runFtpCfg.datapath),  SDK_CFG_DATA_TYPE_STRING,  "MDSnapshot",   "rw", 1, MAX_STR_LEN_128,  NULL},
    {"filename",  &(runFtpCfg.filename),  SDK_CFG_DATA_TYPE_STRING,  "image.jpg",   "rw", 1, MAX_STR_LEN_128,  NULL},
    {"interval",  &(runFtpCfg.interval),  SDK_CFG_DATA_TYPE_S32,     "30",   "rw", 1, MAX_STR_LEN_128,  NULL},
    {NULL,},
};



void FtpCfgPrint()
{
    CfgPrintMap(ftpMap);
}


int FtpCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "ftp", ftpMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(FTP_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", FTP_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int FtpCfgLoadDefValue()
{
    CfgLoadDefValue(ftpMap);


    return 0;
}

int FtpCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(FTP_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", FTP_CFG_FILE);
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

    CfgParseCjson(json, "ftp", ftpMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    FtpCfgLoadDefValue();
    FtpCfgSave();
    return 0;
}
cJSON * FtpCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(FTP_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", FTP_CFG_FILE);
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
int FtpJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(FTP_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", FTP_CFG_FILE);
        return -1;
    }

    free(out);
    FtpCfgLoad();
    return 0;
}

