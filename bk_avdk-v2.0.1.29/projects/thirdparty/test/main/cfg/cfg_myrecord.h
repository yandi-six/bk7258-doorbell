/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_MYRECORD_H__
#define _SDK_CFG_MYRECORD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

    /***********************************/
    /***         record              ***/
    /***********************************/
    typedef struct
    {
        SDK_S32 enable;
        SDK_S8 start_time[MAX_STR_LEN_16];
        SDK_S8 end_time[MAX_STR_LEN_16];
        SDK_S32 resolution_index;
        SDK_S32 record_time;
        SDK_S32 record_interval;
        SDK_S32 recordLen;
        SDK_S32 recycleRecord;
        SDK_S32 cover;
    } SDK_NET_MYRECORD_CFG, *LPSDK_NET_MYRECORD_CFG;



    extern int MyRecordCfgSave();
    extern int MyRecordCfgLoad();
    extern void MyRecordCfgPrint();
    extern int MyRecordCfgLoadDefValue();
    extern cJSON *MyRecordCfgLoadJson();
    extern int MyRecordJsonSaveCfg(cJSON *json);

#define MYRECORD_CFG_FILE "myrecord_cfg.cjson"

    extern SDK_NET_MYRECORD_CFG runMyRecordCfg;
    extern SDK_CFG_MAP myrecordMap[];

#ifdef __cplusplus
}
#endif
#endif
