/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_ptz.h"
#include "sdk_log.h"

SDK_NET_PTZ_CFG     runPtzCfg;
SDK_NET_DECODERCFG  runPtzDecCfg;

SDK_NET_PRESET_INFO runPresetCfg;
SDK_NET_CRUISE_CFG  runCruiseCfg;

SDK_CFG_MAP ptzMap[] = {
    {"id",           &runPtzCfg.id,           SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 10, NULL},
    {"serialPortID", &runPtzCfg.serialPortID, SDK_CFG_DATA_TYPE_S32, "2", "rw", 0, 10, NULL},
    {"videoInputID", &runPtzCfg.videoInputID, SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 10, NULL},
    {"duplexMode",   &runPtzCfg.duplexMode,   SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1,  "0-half, 1-full"},
    {"controlType",  &runPtzCfg.controlType,  SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1,  "0- controlType, 1- external"},
    {"protocol",     &runPtzCfg.protocol,     SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 10, "0 pelco-d, 1 pelco-p"},
    {"address",      &runPtzCfg.address,      SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 10, NULL},

    {"channel",        &runPtzDecCfg.channel,        SDK_CFG_DATA_TYPE_S32,    "0",    "rw",  0, 255,  NULL},
    {"baudRate",       &runPtzDecCfg.baudRate,       SDK_CFG_DATA_TYPE_S32, "1200",    "rw", 600, 115200, NULL},
    {"dataBit",        &runPtzDecCfg.dataBit,        SDK_CFG_DATA_TYPE_U8,     "3",    "rw",  0,   3,  NULL},
    {"stopBit",        &runPtzDecCfg.stopBit,        SDK_CFG_DATA_TYPE_U8,     "0",    "rw",  0,   2,  NULL},
    {"parity",         &runPtzDecCfg.parity,         SDK_CFG_DATA_TYPE_U8,     "0",    "rw",  0,   2,  NULL},
    {"flowcontrol",    &runPtzDecCfg.flowcontrol,    SDK_CFG_DATA_TYPE_U8,     "0",    "rw",  0,   2,  NULL},
    {"workMode",       &runPtzDecCfg.workMode,       SDK_CFG_DATA_TYPE_S32,    "2",    "rw",  0,   2,  NULL},
    {"decoderAddress", &runPtzDecCfg.decoderAddress, SDK_CFG_DATA_TYPE_U16,    "0",    "rw",  0, 255,  NULL},
    {"speedH",         &runPtzDecCfg.speedH,         SDK_CFG_DATA_TYPE_U16,    "2",    "rw",  0,  64,  NULL},
    {"speedV",         &runPtzDecCfg.speedV,         SDK_CFG_DATA_TYPE_U16,    "2",    "rw",  0,  64,  NULL},
    {"watchPos",       &runPtzDecCfg.watchPos,       SDK_CFG_DATA_TYPE_U16,    "0",    "rw",  0, 255,  NULL},
    {"decoderType",    &runPtzDecCfg.decoderType,    SDK_CFG_DATA_TYPE_STRING, "pelco-d",  "rw",  1, MAX_STR_LEN_32,  NULL},

    {NULL,},
};


void PtzCfgPrint()
{
    //printf("********** Ptz *********\n");
    CfgPrintMap(ptzMap);
    //printf("********** Ptz *********\n\n");
}

int PtzCfgSave()
{
    int ret = CfgSave(PTZ_CFG_FILE, "ptz", ptzMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", PTZ_CFG_FILE);
        return -1;
    }

    return 0;
}

int PtzCfgLoad()
{
    int ret = CfgLoad(PTZ_CFG_FILE, "ptz", ptzMap);
    if (ret != 0) {
        webcam_error("CfgLoad %s error.", PTZ_CFG_FILE);
        return -1;
    }

    return 0;
}

cJSON * PtzCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(PTZ_CFG_FILE);
    if (data == NULL) {
        //ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        webcam_debug("load %s error, so to load default cfg param.\n", PTZ_CFG_FILE);
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


int PtzCfgLoadDefValue()
{
    CfgLoadDefValue(ptzMap);

    return 0;
}


int PresetCruiseCfgDefault()
{
    int i, j;

    memset(&runPresetCfg, 0, sizeof(runPresetCfg));
    memset(&runCruiseCfg, 0, sizeof(runCruiseCfg));

    for (i = 0; i < PTZ_MAX_CRUISE_GROUP_NUM; i++)
    {
        runCruiseCfg.struCruise[i].byPointNum    = 0;
        runCruiseCfg.struCruise[i].byCruiseIndex = i;
        for (j = 0; j < PTZ_MAX_CRUISE_POINT_NUM; j++)
        {
            runCruiseCfg.struCruise[i].struCruisePoint[j].byPointIndex = 0;
            runCruiseCfg.struCruise[i].struCruisePoint[j].byPresetNo   = 0;
            runCruiseCfg.struCruise[i].struCruisePoint[j].byRemainTime = 0;
            runCruiseCfg.struCruise[i].struCruisePoint[j].bySpeed      = 0;
        }
    }

    return 0;
}


int PresetCruiseCfgSave()
{
    int ret = 0;
    int fd  = -1;
    int val = 0;
    char filename[128]={0};
    snprintf(filename,128,"%s%s",CFG_DIR,PTZ_CRUISE_FILE);
    if (access(filename, F_OK) != 0)
    {
        webcam_error("File:%s don't exist! creat new file. \n", filename);
        fd = open(filename, (O_CREAT|O_RDWR|O_TRUNC));
        if (fd < 0)
        {
            webcam_error("open %s ERROR! %s\n", filename, strerror(errno));
            return -1;
        }
    }
    else
    {
        fd = open(filename, O_RDWR);
        if (fd < 0)
        {
            webcam_error("open %s ERROR! %s\n", filename, strerror(errno));
            return -1;
        }
    }

    //val = mtd_crc32(0,   &runPresetCfg, sizeof(runPresetCfg));
    //val = mtd_crc32(val, &runCruiseCfg, sizeof(runCruiseCfg));

    //PRINT_INFO("FUN[%s]  LINE[%d]  crc_val:0x%x \n", __FUNCTION__, __LINE__, val);

    ret = write(fd, &val, sizeof(val));
    if (ret <= 0)
    {
        webcam_error("write CRC_val error! %s\n", strerror(errno));
        close(fd);
    }

    ret = write(fd, &runPresetCfg, sizeof(runPresetCfg));
    if (ret <= 0)
    {
        webcam_error("write runPresetCfg error! %s\n", strerror(errno));
        close(fd);
    }

    ret = write(fd, &runCruiseCfg, sizeof(runCruiseCfg));
    if (ret <= 0)
    {
        webcam_error("write runCruiseCfg error! %s\n", strerror(errno));
        close(fd);
    }

    close(fd);
    return 0;
}


int PresetCruiseCfgLoad()
{
    int ret = 0;
    int crc = 0;
    int val = 0;
    int fd  = -1;
    char filename[128]={0};
    snprintf(filename,128,"%s%s",CFG_DIR,PTZ_CRUISE_FILE);
    if (access(filename, F_OK) != 0)
    {
        webcam_error("File:%s don't exist! creat new file. \n", filename);
        PresetCruiseCfgDefault();
        PresetCruiseCfgSave();
        return 0;
    }
    else
    {
        fd = open(filename, O_RDWR);
        if (fd < 0)
        {
            webcam_error("open %s ERROR! %s\n", filename, strerror(errno));
            return -1;
        }
    }

    ret = read(fd, &crc, sizeof(crc));
    if (ret <= 0)
    {
        webcam_error("read CRC_val error! %s\n", strerror(errno));
        close(fd);
    }

    ret = read(fd, &runPresetCfg, sizeof(runPresetCfg));
    if (ret <= 0)
    {
        webcam_error("read runPresetCfg error! %s\n", strerror(errno));
        close(fd);
    }


    ret = read(fd, &runCruiseCfg, sizeof(runCruiseCfg));
    if (ret <= 0)
    {
        webcam_error("read runCruiseCfg error! %s\n", strerror(errno));
        close(fd);
    }
    close(fd);

    //val = mtd_crc32(0,   &runPresetCfg, sizeof(runPresetCfg));
    //val = mtd_crc32(val, &runCruiseCfg, sizeof(runCruiseCfg));

    if (val != crc)
    {
        webcam_error("Get check crc error! val:0x%x  crc:0x%x\n", val, crc);
        PresetCruiseCfgDefault();
        PresetCruiseCfgSave();
        return 0;
    }

    return 0;
}
int PtzJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(PTZ_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", PTZ_CFG_FILE);
        return -1;
    }

    free(out);
    PtzCfgLoad();
    return 0;
}
