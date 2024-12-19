/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_TIMEZONE_H__
#define _SDK_CFG_TIMEZONE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

    /***********************************/
    /***         timezone              ***/
    /***********************************/
    typedef struct
    {
        int timezone; //-720, +720  ʱ�� ����,
        SDK_S8 ntp[MAX_URL_STR_SIZE];
        SDK_S32 offset;
        SDK_S8 abbr[MAX_SYSTEM_STR_SIZE];
        SDK_S32 isdst;
        SDK_S8 name[MAX_SYSTEM_STR_SIZE];
        SDK_S8 text[MAX_STR_LEN_64];

    } SDK_NET_TIMEZONE_CFG, *LPSDK_NET_TIMEZONE_CFG;
    extern int TimezoneCfgSave();
    extern int TimezoneCfgLoad();
    extern void TimezoneCfgPrint();
    extern int TimezoneCfgLoadDefValue();
    extern cJSON *TimezoneCfgLoadJson();
    extern int TimezoneJsonSaveCfg(cJSON *json);

#define TIMEZONE_CFG_FILE "timezone_cfg.cjson"

    extern SDK_NET_TIMEZONE_CFG runTimezoneCfg;
    extern SDK_CFG_MAP timezoneMap[];

#ifdef __cplusplus
}
#endif
#endif
