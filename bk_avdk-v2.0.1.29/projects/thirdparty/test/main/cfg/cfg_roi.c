/*!
*****************************************************************************

*****************************************************************************
*/

#include "cfg_roi.h"
// #include "sdk_log.h"
SDK_NET_ROI_CFG runRoiCfg;




SDK_CFG_MAP roiMap[] = {
    {"whole_region", &runRoiCfg.whole_region, SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1, NULL},
    {"start_x", &runRoiCfg.start_x, SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 2000, NULL},
    {"start_y", &runRoiCfg.start_y, SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 2000, NULL},
    {"end_x", &runRoiCfg.end_x, SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 2000, NULL},
    {"end_y", &runRoiCfg.end_y, SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 2000, NULL},

    // {"recycleRecord",  &runRoiCfg.recycleRecord,        SDK_CFG_DATA_TYPE_S32,   "1",   "rw", 0, 1,   NULL},
    // {"scheduleTime",     &(runRoiCfg.scheduleTime[0][0]),  SDK_CFG_DATA_TYPE_STIME, "0",   "rw", 0, 0,  NULL},
    {
        NULL,
    },
};

void RoiCfgPrint()
{
    // printf("********** Record *********\n");
    CfgPrintMap(roiMap);
    // printf("********** Record *********\n\n");
}

int RoiCfgSave()
{
    int ret = CfgSave(ROI_CFG_FILE, "roi", roiMap);
    if (ret != 0)
    {
        // webcam_error("CfgSave %s error.", ROI_CFG_FILE);
        bk_printf("RoiCfgSave error\n");
        return -1;
    }
    return 0;
}




int RoiCfgLoad()
{
    int ret = CfgLoad(ROI_CFG_FILE, "roi", roiMap);
    if (ret != 0)
    {
        // webcam_error("CfgLoad %s error.", ROI_CFG_FILE);
        return -1;
    }
    return 0;
}

cJSON *RoiCfgLoadJson()
{
    char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(ROI_CFG_FILE);
    if (data == NULL)
    {
        // ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        // webcam_debug("load %s error, so to load default cfg param.\n", ROI_CFG_FILE);
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

int RoiCfgLoadDefValue()
{
    CfgLoadDefValue(roiMap);

    return 0;
}
int RoiJsonSaveCfg(cJSON *json)
{
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(ROI_CFG_FILE, out);
    if (ret != 0)
    {
        // webcam_error("CfgWriteToFile %s error.", ROI_CFG_FILE);
        return -1;
    }

    free(out);
    RoiCfgLoad();
    return 0;
}
