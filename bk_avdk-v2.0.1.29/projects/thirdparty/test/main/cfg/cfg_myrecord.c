/*!
*****************************************************************************

*****************************************************************************
*/

#include "cfg_myrecord.h"
// #include "sdk_log.h"
SDK_NET_MYRECORD_CFG runMyRecordCfg;


SDK_CFG_MAP myrecordMap[] = {
    {"auto", &runMyRecordCfg.enable, SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1, NULL},
    {"start_time", &runMyRecordCfg.start_time, SDK_CFG_DATA_TYPE_STRING, "00:00:00", "rw", 1, MAX_STR_LEN_16, NULL},
    {"end_time", &runMyRecordCfg.end_time, SDK_CFG_DATA_TYPE_STRING, "23:59:59", "rw", 1, MAX_STR_LEN_16, NULL},
    {"resolution_index", &runMyRecordCfg.resolution_index, SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 10, NULL},
    {"record_time", &runMyRecordCfg.record_time, SDK_CFG_DATA_TYPE_S32, "60", "rw", 0, 1000, NULL},
    {"record_interval", &runMyRecordCfg.record_interval, SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 3, NULL},
    {"cover", &runMyRecordCfg.cover, SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1, NULL},
    // {"recycleRecord",  &runMyRecordCfg.recycleRecord,        SDK_CFG_DATA_TYPE_S32,   "1",   "rw", 0, 1,   NULL},
    // {"scheduleTime",     &(runMyRecordCfg.scheduleTime[0][0]),  SDK_CFG_DATA_TYPE_STIME, "0",   "rw", 0, 0,  NULL},
    {
        NULL,
    },
};

void MyRecordCfgPrint()
{
    // printf("********** Record *********\n");
    CfgPrintMap(myrecordMap);
    // printf("********** Record *********\n\n");
}

int MyRecordCfgSave()
{
    int ret = CfgSave(MYRECORD_CFG_FILE, "myrecord", myrecordMap);
    if (ret != 0)
    {
        // webcam_error("CfgSave %s error.", MYRECORD_CFG_FILE);
        bk_printf("MyRecordCfgSave error\n");
        return -1;
    }
    return 0;
}


// int MyRecordCfgSave()
// {
//     cJSON *root;
//     // cJSON *wifiLink = NULL;
//     char *out;

//     root = cJSON_CreateObject();//\B4\B4\BD\A8\CF\EEĿ

//     CfgAddCjson(root, "myrecord", myrecordMap);



//     out = cJSON_Print(root);

//     int ret = CfgWriteToFile(MYRECORD_CFG_FILE, out);
//     if (ret != 0) {
//         os_printf("CfgWriteToFile %s error.\n", MYRECORD_CFG_FILE);
//         return -1;
//     }

//     os_free(out);
//     cJSON_Delete(root);

//     return 0;
// }

int MyRecordCfgLoad()
{
    int ret = CfgLoad(MYRECORD_CFG_FILE, "myrecord", myrecordMap);
    if (ret != 0)
    {
        // webcam_error("CfgLoad %s error.", MYRECORD_CFG_FILE);
        return -1;
    }
    return 0;
}

cJSON *myRecordCfgLoadJson()
{
    char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(MYRECORD_CFG_FILE);
    if (data == NULL)
    {
        // ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        // webcam_debug("load %s error, so to load default cfg param.\n", MYRECORD_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json)
    {
        // ŽÓÅäÖÃÎÄŒþœâÎöcjsonÊ§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        // webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    free(data);
    return json;

err:
    if (data)
    {
        free(data);
    }
    return NULL;
}

int MyRecordCfgLoadDefValue()
{
    CfgLoadDefValue(myrecordMap);

    return 0;
}
int MyRecordJsonSaveCfg(cJSON *json)
{
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(MYRECORD_CFG_FILE, out);
    if (ret != 0)
    {
        // webcam_error("CfgWriteToFile %s error.", MYRECORD_CFG_FILE);
        return -1;
    }

    free(out);
    MyRecordCfgLoad();
    return 0;
}
