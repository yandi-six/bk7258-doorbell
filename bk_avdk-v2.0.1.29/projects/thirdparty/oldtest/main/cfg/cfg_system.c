
#include <stdio.h>
#include <ctype.h>
#include "cfg_system.h"


SDK_NET_SYSTEM_CFG runSystemCfg;
SDK_SYSTEM_INFO g_systemInfo;



SDK_CFG_MAP deviceInfoMap[] = {
    {"deviceName",        &(runSystemCfg.deviceInfo.deviceName),        SDK_CFG_DATA_TYPE_STRING, "WebCam",    "rw", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"description",       &(runSystemCfg.deviceInfo.manufacturer),      SDK_CFG_DATA_TYPE_STRING, "WebRTC Camera",     "rw", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"deviceType",        &(runSystemCfg.deviceInfo.deviceType),        SDK_CFG_DATA_TYPE_STRING, "WebCam",   "rw", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"sensorType",        &(runSystemCfg.deviceInfo.sensorType),        SDK_CFG_DATA_TYPE_S32,    "0",        "rw", 0, 10, "0: IMX222; 1 OV9710 2 ..."},
    {"languageType",      &(runSystemCfg.deviceInfo.languageType),      SDK_CFG_DATA_TYPE_S32,    "0",        "rw", 0, 18, "0: chiness; 1 english 2 ..."},
    {"playsound",        &(runSystemCfg.deviceInfo.playSound),          SDK_CFG_DATA_TYPE_S32,    "0",        "rw", 1, 10, "0; 1"},
    {"initString",        &(runSystemCfg.deviceInfo.initString),        SDK_CFG_DATA_TYPE_STRING,  "", "ro", 1, MAX_STR_LEN_256, NULL},
    {"serverAddress",      &(runSystemCfg.deviceInfo.serverAddress),        SDK_CFG_DATA_TYPE_STRING,  "", "ro", 1, MAX_STR_LEN_128, NULL},
    {"serialNumber",        &(runSystemCfg.deviceInfo.serialNumber),        SDK_CFG_DATA_TYPE_STRING,  "", "ro", 1, MAX_STR_LEN_64, NULL},
    {"softwareVersion",     &(runSystemCfg.deviceInfo.softwareVersion),     SDK_CFG_DATA_TYPE_STRING,  "S-1.230.586",     "ro", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"softwareBuildDate",   &(runSystemCfg.deviceInfo.softwareBuildDate),   SDK_CFG_DATA_TYPE_STRING,  "20150928", "ro", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"firmwareVersion",     &(runSystemCfg.deviceInfo.firmwareVersion),     SDK_CFG_DATA_TYPE_STRING,  "F-1.0",     "ro", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"firmwareReleaseDate", &(runSystemCfg.deviceInfo.firmwareReleaseDate), SDK_CFG_DATA_TYPE_STRING,  "20150928", "ro", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"hardwareVersion",     &(runSystemCfg.deviceInfo.hardwareVersion),     SDK_CFG_DATA_TYPE_STRING,  "H-1.1.1",     "ro", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"hardwareBuildDate",   &(runSystemCfg.deviceInfo.hardwareBuildDate),   SDK_CFG_DATA_TYPE_STRING,  "20150928", "ro", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"webVersion",          &(runSystemCfg.deviceInfo.webVersion),          SDK_CFG_DATA_TYPE_STRING,  "W-1.0",     "ro", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"webBuildDate",        &(runSystemCfg.deviceInfo.webBuildDate),        SDK_CFG_DATA_TYPE_STRING,  "20150928", "ro", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {NULL,},
};

SDK_CFG_MAP timezoneCfgMap[] = {
    {"timeZone", &(runSystemCfg.timezoneCfg.timezone), SDK_CFG_DATA_TYPE_S32, "480", "rw", -720, 720,  NULL},
    {NULL,},
};

SDK_CFG_MAP netDstCfgCfgMap[] = {
    {"netDstCfg.enableDST",  &(runSystemCfg.netDstCfg.enableDST),  SDK_CFG_DATA_TYPE_S32,     "0",         "rw", 0, 1,  NULL},
    {"netDstCfg.dSTBias",    &(runSystemCfg.netDstCfg.dSTBias),    SDK_CFG_DATA_TYPE_S32,     "0",         "rw", 0, 360,  NULL},
    {"netDstCfg.beginTime",  &(runSystemCfg.netDstCfg.beginTime),  SDK_CFG_DATA_TYPE_STRING,  "2015.8.30", "rw", 1, MAX_TIME_STR_SIZE,  NULL},
    {"netDstCfg.endTime",    &(runSystemCfg.netDstCfg.endTime),    SDK_CFG_DATA_TYPE_STRING,  "2015.8.30", "rw", 1, MAX_TIME_STR_SIZE,  NULL},
    {NULL,},
};

SDK_CFG_MAP ntpCfgMap[] = {
    {"ntpCfg_enable",        &(runSystemCfg.ntpCfg.enable),        SDK_CFG_DATA_TYPE_S32,     "1",               "rw", 0, 1,  NULL},
    {"ntpCfg_serverDomain",  &(runSystemCfg.ntpCfg.serverDomain),  SDK_CFG_DATA_TYPE_STRING,  "time-a.nist.gov", "rw", 1, MAX_URL_STR_SIZE, NULL},
    {NULL,},
};


SDK_CFG_MAP maintainCfgMap[] = {
    {"enable",   &(runSystemCfg.maintainCfg.enable),  SDK_CFG_DATA_TYPE_U8,    "0",  "rw", 0, 1,  NULL},
    {"index",    &(runSystemCfg.maintainCfg.index),   SDK_CFG_DATA_TYPE_U8,    "2",  "rw", 0, 7,  NULL},
    {"hour",     &(runSystemCfg.maintainCfg.hour),    SDK_CFG_DATA_TYPE_U8,    "3",  "rw", 0, 23,   NULL},
    {"minute",   &(runSystemCfg.maintainCfg.minute),  SDK_CFG_DATA_TYPE_U8,    "0",  "rw", 0, 59,   NULL},
    {"second",   &(runSystemCfg.maintainCfg.second),  SDK_CFG_DATA_TYPE_U8,    "0",  "rw", 0, 59,   NULL},
    {NULL,},
};


void SystemCfgPrint()
{
    //printf("*************** System **************\n");

   // printf("device info:\n");
    CfgPrintMap(deviceInfoMap);
    //printf("\n");

    //printf("timezone:\n");
    CfgPrintMap(timezoneCfgMap);
    //printf("\n");

    //printf("netDST:\n");
    CfgPrintMap(netDstCfgCfgMap);
   // printf("\n");

   // printf("ntp:\n");
    CfgPrintMap(ntpCfgMap);
   // printf("\n");

	//printf("maintain:\n");
    CfgPrintMap(maintainCfgMap);
   // printf("\n");

    //printf("*************** System **************\n\n");
}

/*
  add by heyong, for load special system info.
*/




SDK_CFG_MAP systemInfoMap[] = {
    {"chip_type",        &(g_systemInfo.chip_type),        SDK_CFG_DATA_TYPE_STRING, "bk_7258",      "rw", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"sensor_type",       &(g_systemInfo.sensor_type),     SDK_CFG_DATA_TYPE_STRING, "gc",     "rw", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"svn_version",       &(g_systemInfo.svn_version),     SDK_CFG_DATA_TYPE_STRING, "1110",   "rw", 1, MAX_SYSTEM_STR_SIZE, NULL},
    {"make_date",        &(g_systemInfo.make_date),        SDK_CFG_DATA_TYPE_STRING,    __DATE__,        "rw", 0, MAX_SYSTEM_STR_SIZE, NULL},
    {NULL,},
};
int LoadSystemInfo()
{
    char *data = NULL;
    data = CfgReadFromFile(SYSTEM_INFO_FILE);
    if (data == NULL) {
        os_printf("load %s error, so to load default cfg param.\n", SYSTEM_INFO_FILE);
        return -1;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        return -1;
    }

    CfgParseCjson(json, "sys_info", systemInfoMap);

    cJSON_Delete(json);
    free(data);

    //printf(">>>>>>sys_info:\n");
    CfgPrintMap(systemInfoMap);
    //printf("\n");

    return 0;
}

int SystemInfoCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();

    CfgAddCjson(root, "sys_info", systemInfoMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(SYSTEM_INFO_FILE, out);
    if (ret != 0) {
        os_printf("CfgWriteToFile %s error.", SYSTEM_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

static void y_toupper(char* s)
{
   int i;
   char* p = (char*)s;

   for (i = 0; i < strlen(s); i++)
   {
           p[i] = toupper(p[i]);
   }
}
void loadSpecialInfo()
{
    sprintf(runSystemCfg.deviceInfo.softwareVersion,"V1.1.0.%s",g_systemInfo.svn_version);
    sprintf(runSystemCfg.deviceInfo.hardwareVersion,"%s_%s",g_systemInfo.chip_type,g_systemInfo.sensor_type);
    y_toupper(runSystemCfg.deviceInfo.hardwareVersion);
    sprintf(runSystemCfg.deviceInfo.softwareBuildDate,"%s",g_systemInfo.make_date);
}
/*
  add end.
*/

int SystemCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//������Ŀ

    CfgAddCjson(root, "deviceinfo", deviceInfoMap);
    CfgAddCjson(root, "timezone", timezoneCfgMap);
    CfgAddCjson(root, "netDST", netDstCfgCfgMap);
    CfgAddCjson(root, "NTP", ntpCfgMap);
    CfgAddCjson(root, "Maintain", maintainCfgMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(SYSTEM_CFG_FILE, out);
    if (ret != 0) {
        os_printf("CfgWriteToFile %s error.", SYSTEM_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int SystemCfgLoadDefValue()
{
    CfgLoadDefValue(deviceInfoMap);
    CfgLoadDefValue(timezoneCfgMap);
    CfgLoadDefValue(netDstCfgCfgMap);
    CfgLoadDefValue(ntpCfgMap);
    CfgLoadDefValue(maintainCfgMap);
    CfgLoadDefValue(systemInfoMap);
    loadSpecialInfo();
    return 0;
}


int SystemCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(SYSTEM_CFG_FILE);
    if (data == NULL) {
        os_printf("load %s error, so to load default cfg param.\n", SYSTEM_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);//将读取到的字符串数据解析为 cJSON 对象
    if (!json){
        os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    CfgParseCjson(json, "deviceinfo", deviceInfoMap);
    CfgParseCjson(json, "timezone", timezoneCfgMap);
    CfgParseCjson(json, "netDST", netDstCfgCfgMap);
    CfgParseCjson(json, "NTP", ntpCfgMap);
    CfgParseCjson(json, "Maintain", maintainCfgMap);
    CfgLoadDefValue(systemInfoMap);

    loadSpecialInfo();

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    SystemCfgLoadDefValue();
    SystemCfgSave();
    return 0;
}

cJSON * SystemCfgLoadJson()
{
    char *data = NULL;
    data = CfgReadFromFile(SYSTEM_CFG_FILE);
    if (data == NULL) {
        os_printf("load %s error, so to load default cfg param.\n", SYSTEM_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
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


cJSON *SystemCfgGetNTPJsonSting()
{
     cJSON *json = CfgDataToCjsonByMap(ntpCfgMap);
    return json;
}


char *SytemCfgGetCjsonString()
{
    cJSON *json = NULL;
    char *buf;
    json = CfgDataToCjsonByMap(deviceInfoMap);
    buf = cJSON_Print(json);
    cJSON_Delete(json);

    return buf;
}
