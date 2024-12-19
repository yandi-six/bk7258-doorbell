/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_OTHER_H__
#define _SDK_CFG_OTHER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

    /***********************************/
    /***         other              ***/
    /***********************************/
#include "sys/time.h"
    typedef struct
    {
        SDK_S32 mirror;
        SDK_S32 vertical;
        SDK_S32 led_statue;
        SDK_S8 senser_type[MAX_STR_LEN_16];
        SDK_S8 rtc_sec[MAX_STR_LEN_16];
        SDK_S8 rtc_usec[MAX_STR_LEN_16];
        
    } SDK_NET_OTHER_CFG, *LPSDK_NET_OTHER_CFG;
    extern int OtherCfgSave();
    extern int OtherCfgLoad();
    extern void OtherCfgPrint();
    extern int OtherCfgLoadDefValue();
    extern cJSON *OtherCfgLoadJson();
    extern int OtherJsonSaveCfg(cJSON *json);

#define OTHER_CFG_FILE "other_cfg.cjson"

    extern SDK_NET_OTHER_CFG runOtherCfg;
    extern SDK_CFG_MAP otherMap[];

#ifdef __cplusplus
}
#endif
#endif
