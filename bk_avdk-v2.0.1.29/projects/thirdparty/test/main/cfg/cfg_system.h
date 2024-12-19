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
    SDK_S32   languageType;   // 0: ����; 1 Ӣ�� 2 ...
    SDK_S32   playSound;
    SDK_S8    serialNumber[MAX_STR_LEN_64];//���к�
    SDK_S8    initString[MAX_STR_LEN_256];
    SDK_S8    serverAddress[MAX_STR_LEN_128];
    SDK_S8    softwareVersion[MAX_SYSTEM_STR_SIZE]; //�����汾�ţ���16λ�����汾����16λ�Ǵΰ汾
    SDK_S8    softwareBuildDate[MAX_SYSTEM_STR_SIZE]; //�����������ڣ�0xYYYYMMDD
    SDK_S8    firmwareVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    firmwareReleaseDate[MAX_SYSTEM_STR_SIZE];
    SDK_S8    hardwareVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    hardwareBuildDate[MAX_SYSTEM_STR_SIZE];
    SDK_S8    webVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8 webBuildDate[MAX_SYSTEM_STR_SIZE];
    SDK_S8 server_token[MAX_STR_LEN_64];
} SDK_NET_DEVICE_INFO;

/*ʱ������ʱ�Ʋ���*/
typedef struct  {
    SDK_U32 month;    //�� 0-11��ʾ1-12����
    SDK_U32 weekNo;   //�ڼ��ܣ�0����1�ܣ�1����2�ܣ�2����3�ܣ� 3����4�ܣ�4�����һ��
    SDK_U32 weekDate; //���ڼ���0�������գ�1������һ��2�����ڶ���3����������4�������ģ�5�������壻6��������
    SDK_U32 hour;     //Сʱ����ʼʱ��ȡֵ��Χ0��23�� ����ʱ��ȡֵ��Χ1��23
    SDK_U32 min;      //��0��59
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
    int     timezone;   //-720, +720  ʱ�� ����,
    SDK_S8  ntp[MAX_URL_STR_SIZE];
    SDK_S32 offset;
    SDK_S8  abbr[MAX_SYSTEM_STR_SIZE];
    SDK_S32 isdst;
    SDK_S8  name[MAX_SYSTEM_STR_SIZE];
    SDK_S8  text[MAX_STR_LEN_64];

} SDK_NET_ZONE, *LPSDK_NET_ZONE;

typedef struct {
    SDK_S32    enableDST;       //�Ƿ�������ʱ�� 0�������� 1������
    SDK_S32    dSTBias;         //����ʱƫ��ֵ��30min, 60min, 90min, 120min, �Է��Ӽƣ�����ԭʼ��ֵ
    SDK_S8     beginTime[MAX_TIME_STR_SIZE]; //��ʱ�ƿ�ʼʱ��
    SDK_S8     endTime[MAX_TIME_STR_SIZE];   //��ʱ��ֹͣʱ��
} SDK_NET_DST, *LPSDK_NET_DST;

typedef struct {
    SDK_S32   enable;
    SDK_S8    serverDomain[MAX_URL_STR_SIZE];
} SDK_NTP_CFG;

typedef struct {
    SDK_U8      enable;
    SDK_U8      index; /* 0 ������ ... 6 ������ 7 ÿ��*/
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

