/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_timezone.h"
// #include "sdk_log.h"
SDK_NET_TIMEZONE_CFG runTimezoneCfg;

SDK_CFG_MAP timezoneMap[] = {
    {"timeZone", &(runTimezoneCfg.timezone), SDK_CFG_DATA_TYPE_S32, "480", "rw", -720, 720,  NULL},
    {"ntp", &(runTimezoneCfg.ntp), SDK_CFG_DATA_TYPE_STRING, "time.windows.com", "rw", 1, MAX_URL_STR_SIZE,  NULL},
    {"offset", &(runTimezoneCfg.offset), SDK_CFG_DATA_TYPE_S32, "8", "rw", -12, 12,  NULL},
    {"abbr", &(runTimezoneCfg.abbr), SDK_CFG_DATA_TYPE_STRING, "CST", "rw", 1, MAX_SYSTEM_STR_SIZE,  NULL},
    {"isdst", &(runTimezoneCfg.isdst), SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1,  NULL},
    {"name", &(runTimezoneCfg.name), SDK_CFG_DATA_TYPE_STRING, "China Standard Time", "rw", 1, MAX_SYSTEM_STR_SIZE,  NULL},
    {"text", &(runTimezoneCfg.text), SDK_CFG_DATA_TYPE_STRING, "(UTC+08:00) Beijing, Chongqing, Hong Kong, Urumqi", "rw", 1, MAX_STR_LEN_64,  NULL},
    {NULL,},
};

void TimezoneCfgPrint()
{
    //printf("********** Timezone *********\n");
    CfgPrintMap(timezoneMap);
    //printf("********** Timezone *********\n\n");
}

int TimezoneCfgSave()
{
    int ret = CfgSave(TIMEZONE_CFG_FILE, "timezone", timezoneMap);
    if (ret != 0) {
        // webcam_error("CfgSave %s error.", TIMEZONE_CFG_FILE);
        return -1;
    }
    return 0;
}

int TimezoneCfgLoad()
{
    int ret = CfgLoad(TIMEZONE_CFG_FILE, "timezone", timezoneMap);
    if (ret != 0) {
        // webcam_error("CfgLoad %s error.", TIMEZONE_CFG_FILE);
        return -1;
    }
    return 0;
}

cJSON * TimezoneCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(TIMEZONE_CFG_FILE);
    if (data == NULL) {
        //ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        // webcam_debug("load %s error, so to load default cfg param.\n", TIMEZONE_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //ŽÓÅäÖÃÎÄŒþœâÎöcjsonÊ§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        // webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
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


int TimezoneCfgLoadDefValue()
{
    CfgLoadDefValue(timezoneMap);

    return 0;
}
int TimezoneJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(TIMEZONE_CFG_FILE, out);
    if (ret != 0) {
        // webcam_error("CfgWriteToFile %s error.", TIMEZONE_CFG_FILE);
        return -1;
    }

    free(out);
    TimezoneCfgLoad();
    return 0;
}
