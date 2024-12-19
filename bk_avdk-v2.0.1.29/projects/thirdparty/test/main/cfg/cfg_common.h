/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_COMMON_H__
#define _SDK_CFG_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <sys/time.h>
#include "sdk_def.h"
#include "cJSON.h"
#if CONFIG_TRNG_SUPPORT             
#include "driver/trng.h"
#endif
#if CONFIG_VFS
#include "bk_vfs.h"
#include "bk_filesystem.h"
#else
#include "vfs_file_minor.h"
#endif
typedef enum {
	SDK_CFG_DATA_TYPE_U32 = 0,
	SDK_CFG_DATA_TYPE_U16,
	SDK_CFG_DATA_TYPE_U8,
	SDK_CFG_DATA_TYPE_S32,
    // SDK_CFG_DATA_TYPE_S64,//+++
    // SDK_CFG_DATA_TYPE_TIMET,//+++
	SDK_CFG_DATA_TYPE_S16,
	SDK_CFG_DATA_TYPE_S8,
	SDK_CFG_DATA_TYPE_FLOAT,
	//SDK_CFG_DATA_TYPE_DOUBLE,
	SDK_CFG_DATA_TYPE_STRING,
	SDK_CFG_DATA_TYPE_STIME,
	SDK_CFG_DATA_TYPE_SLICE,
    SDK_CFG_DATA_TYPE_ACTION,
    SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,
    SDK_CFG_DATA_TYPE_JSON_ARRAY_STRING,
} SDK_CFG_DATA_TYPE;

typedef struct
{
    char* 					stringName;
    void* 					dataAddress;
    SDK_CFG_DATA_TYPE		dataType;
    char*                   defaultValue;

    char* mode;
    double min; //����
    double max; //����

    char* description;
} SDK_CFG_MAP;

/*ϵͳʱ��*/
typedef struct {
    SDK_U16  year;
    SDK_U16  month;
    SDK_U16  day;
    SDK_U16  hour;
    SDK_U16  minute;
    SDK_U16  second;
} SDK_NET_TIME, *LPSDK_NET_TIME;

typedef struct {
    //��ʼʱ��
    SDK_S32 startHour;
    SDK_S32 startMin;
    //����ʱ��
    SDK_S32 stopHour;
    SDK_S32 stopMin;
} SDK_SCHEDTIME;


extern int CfgWriteDelete(const char *filename);
extern int CfgWriteToFile(const char *filename, const char *data);
extern char *CfgReadFromFile(const char *filename);
extern int CfgWriteToFile2(const char *filename, const char *data,int len);

extern int CfgPrintMap(SDK_CFG_MAP *mapArray);
extern int CfgSave(const char *filename, const char *str, SDK_CFG_MAP *mapArray);
extern int CfgLoad(const char *filename, const char *str, SDK_CFG_MAP *mapArray);
extern int CfgLoadDefValue(SDK_CFG_MAP *mapArray);

extern cJSON *CfgDataToCjsonByMap(SDK_CFG_MAP *mapArray);
extern int CfgCjsonToDataByMap(SDK_CFG_MAP *mapArray, cJSON *json);

extern cJSON* CfgAddCjson(cJSON *root, const char *str, SDK_CFG_MAP *mapArray);
extern cJSON* CfgParseCjson(cJSON *root, const char *str, SDK_CFG_MAP *mapArray);

extern int CfgGetDefByName(SDK_CFG_MAP *mapArray, const char *item_name, void *value);

extern int is_in_schedule_slice(SDK_U32 *slice);
extern int is_in_schedule_timepoint(SDK_SCHEDTIME *time_point);

#define MAX_STR_LEN_16 16
#define MAX_STR_LEN_20 20
#define MAX_STR_LEN_32 32
#define MAX_STR_LEN_64 64
#define MAX_STR_LEN_128 128
#define MAX_STR_LEN_256 256

#define MAX_SYSTEM_STR_SIZE 32
#define MAX_TIME_STR_SIZE 32
#define MAX_URL_STR_SIZE 100

#define MAX_ACTIONS 15
#define MAX_EVENTS 15
#define MAX_SCENEMODES 2

typedef struct {
    SDK_S8    selected;
    SDK_S32   index;
    SDK_S8    lable[MAX_STR_LEN_32]; 
} SDK_ACTION;

typedef struct {
    SDK_S8    enable;
    SDK_S32   index;
    SDK_S8    eventName[MAX_STR_LEN_32];
    SDK_ACTION actions[MAX_ACTIONS];
    SDK_SCHEDTIME scheduleTime[7][12]; 
} SDK_EVENT;


// #define CFG_DIR "/flash/cfg/"
#define CFG_DIR "/flash/cfg/"



#ifdef __cplusplus
}
#endif
#endif /* _SDK_CFG_COMMON_H__ */

