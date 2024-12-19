/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_other.h"
// #include "sdk_log.h"
SDK_NET_OTHER_CFG runOtherCfg;

SDK_CFG_MAP otherMap[] = {
    {"mirror",          &runOtherCfg.mirror,             SDK_CFG_DATA_TYPE_S32,     "0",   "rw", 0, 1,   NULL},
    {"vertical",        &runOtherCfg.vertical,           SDK_CFG_DATA_TYPE_S32,     "0",   "rw", 0, 1,   NULL},
    {"led_statue",      &runOtherCfg.led_statue,         SDK_CFG_DATA_TYPE_S32,     "0",   "rw", 0, 1,   NULL},
    {"senser_type",     &runOtherCfg.senser_type,        SDK_CFG_DATA_TYPE_STRING,  "GC0328",   "rw", 1, MAX_STR_LEN_16,   NULL},
    {NULL,},
};

void OtherCfgPrint()
{
    //printf("********** Other *********\n");
    CfgPrintMap(otherMap);
    //printf("********** Other *********\n\n");
}

int OtherCfgSave()
{
    int ret = CfgSave(OTHER_CFG_FILE, "other", otherMap);
    if (ret != 0) {
        // webcam_error("CfgSave %s error.", OTHER_CFG_FILE);
        return -1;
    }
    return 0;
}

int OtherCfgLoad()
{
    int ret = CfgLoad(OTHER_CFG_FILE, "other", otherMap);
    if (ret != 0) {
        // webcam_error("CfgLoad %s error.", OTHER_CFG_FILE);
        return -1;
    }
    return 0;
}

cJSON * OtherCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(OTHER_CFG_FILE);
    if (data == NULL) {
        //ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        // webcam_debug("load %s error, so to load default cfg param.\n", OTHER_CFG_FILE);
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


int OtherCfgLoadDefValue()
{
    CfgLoadDefValue(otherMap);

    return 0;
}
int OtherJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(OTHER_CFG_FILE, out);
    if (ret != 0) {
        // webcam_error("CfgWriteToFile %s error.", OTHER_CFG_FILE);
        return -1;
    }

    free(out);
    OtherCfgLoad();
    return 0;
}
