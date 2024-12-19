/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_SYSTEM_H__
#define _SDK_CFG_SYSTEM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/******************************************/
/*** device_info, time, zone, dst, NTP  ***/

/******************************************/

#define INVALID_TIEM_ZONE  0XFFFF

typedef enum {
    SDK_PAL = 0,
    SDK_NTSC,
} SDK_VIDEO_STANDARD_E;

typedef struct {
    SDK_S8    deviceName[MAX_SYSTEM_STR_SIZE];
    SDK_S8    manufacturer[MAX_SYSTEM_STR_SIZE];
    SDK_S8    deviceType[MAX_SYSTEM_STR_SIZE];
    SDK_S32   sensorType; // 0: IMX222; 1 OV9710 2 ...
    SDK_S32   languageType;   // 0: 中文; 1 英文 2 ...
    SDK_S32   playSound;
    SDK_S8    serialNumber[MAX_STR_LEN_64];//序列号
    SDK_S8    initString[MAX_STR_LEN_256];
    SDK_S8    serverAddress[MAX_STR_LEN_128];
    SDK_S8    softwareVersion[MAX_SYSTEM_STR_SIZE]; //软件版本号，高16位是主版本，低16位是次版本
    SDK_S8    softwareBuildDate[MAX_SYSTEM_STR_SIZE]; //软件生成日期，0xYYYYMMDD
    SDK_S8    firmwareVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    firmwareReleaseDate[MAX_SYSTEM_STR_SIZE];
    SDK_S8    hardwareVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    hardwareBuildDate[MAX_SYSTEM_STR_SIZE];
    SDK_S8    webVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    webBuildDate[MAX_SYSTEM_STR_SIZE];
} SDK_NET_DEVICE_INFO;

/*时区和夏时制参数*/
typedef struct  {
    SDK_U32 month;    //月 0-11表示1-12个月
    SDK_U32 weekNo;   //第几周：0－第1周；1－第2周；2－第3周； 3－第4周；4－最后一周
    SDK_U32 weekDate; //星期几：0－星期日；1－星期一；2－星期二；3－星期三；4－星期四；5－星期五；6－星期六
    SDK_U32 hour;     //小时，开始时间取值范围0－23； 结束时间取值范围1－23
    SDK_U32 min;      //分0－59
} SDK_NET_TIMEPOINT;

typedef enum {
    SDK_GMT_NEG_12 = -12,
    SDK_GMT_NEG_11 = -11,
    SDK_GMT_NEG_10 = -10,
    SDK_GMT_NEG_9  = -9,
    SDK_GMT_NEG_8  = -8,
    SDK_GMT_NEG_7  = -7,
    SDK_GMT_NEG_6  = -6,
    SDK_GMT_NEG_5  = -5,
    SDK_GMT_NEG_4  = -4,
    SDK_GMT_NEG_3  = -3,
    SDK_GMT_NEG_2  = -2,
    SDK_GMT_NEG_1  = -1,
    SDK_GMT_0      = 0,
    SDK_GMT_POS_1  = 1,
    SDK_GMT_POS_2  = 2,
    SDK_GMT_POS_3  = 3,
    SDK_GMT_POS_4  = 4,
    SDK_GMT_POS_5  = 5,
    SDK_GMT_POS_6  = 6,
    SDK_GMT_POS_7  = 7,
    SDK_GMT_POS_8  = 8,
    SDK_GMT_POS_9  = 9,
    SDK_GMT_POS_10 = 10,
    SDK_GMT_POS_11 = 11,
    SDK_GMT_POS_12 = 12,
}SDK_TimeZone_E;

typedef struct {
    int     timezone;   //-720, +720  时区 分钟,

} SDK_NET_ZONE, *LPSDK_NET_ZONE;

typedef struct {
    SDK_S32    enableDST;       //是否启用夏时制 0－不启用 1－启用
    SDK_S32    dSTBias;         //夏令时偏移值，30min, 60min, 90min, 120min, 以分钟计，传递原始数值
    SDK_S8     beginTime[MAX_TIME_STR_SIZE]; //夏时制开始时间
    SDK_S8     endTime[MAX_TIME_STR_SIZE];   //夏时制停止时间
} SDK_NET_DST, *LPSDK_NET_DST;

typedef struct {
    SDK_S32   enable;
    SDK_S8    serverDomain[MAX_URL_STR_SIZE];
} SDK_NTP_CFG;

typedef struct {
    SDK_U8      enable;
    SDK_U8      index; /* 0 星期天 ... 6 星期六 7 每天*/
    SDK_U8      hour;
    SDK_U8      minute;
    SDK_U8      second;
} SDK_MAINTAIN_CFG;

typedef struct {
    SDK_NET_DEVICE_INFO deviceInfo;
    SDK_NET_ZONE timezoneCfg;
    SDK_NET_DST netDstCfg;
    SDK_NTP_CFG ntpCfg;
    SDK_MAINTAIN_CFG maintainCfg;
} SDK_NET_SYSTEM_CFG;
typedef struct {
    SDK_S8    chip_type[MAX_SYSTEM_STR_SIZE];
    SDK_S8    sensor_type[MAX_SYSTEM_STR_SIZE];
    SDK_S8    svn_version[MAX_SYSTEM_STR_SIZE]; //YYYYMMDD
    SDK_S8    make_date[MAX_SYSTEM_STR_SIZE];
} SDK_SYSTEM_INFO;
extern int LoadSystemInfo();
extern int SystemCfgSave();
extern int SystemCfgLoad();
extern void SystemCfgPrint();
extern int SystemCfgLoadDefValue();
extern char *SytemCfgGetCjsonString();
extern cJSON *SystemCfgGetNTPJsonSting();
extern int SystemInfoCfgSave();
extern cJSON * SystemCfgLoadJson();

#define SYSTEM_INFO_FILE "sys_info.cjson"
#define SYSTEM_CFG_FILE "system_cfg.cjson"

extern SDK_NET_SYSTEM_CFG runSystemCfg;
extern SDK_SYSTEM_INFO g_systemInfo;
extern SDK_CFG_MAP deviceInfoMap[];
extern SDK_CFG_MAP timezoneCfgMap[];
extern SDK_CFG_MAP netDstCfgCfgMap[];
extern SDK_CFG_MAP ntpCfgMap[];

#ifdef __cplusplus
}
#endif
#endif

