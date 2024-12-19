/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_RECORD_STATE_H__
#define _SDK_CFG_RECORD_STATE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

typedef struct {
    int state;
} SDK_RECORD_STATE_CFG;


extern int RecstateCfgDelete();
extern int RecstateCfgSave();
extern int RecstateCfgLoad();
extern void RecstateCfgPrint();
extern int RecstateCfgLoadDefValue();
extern cJSON * RecstateCfgLoadJson();



#define RECSTATE_CFG_FILE "recstate_cfg.cjson"

extern SDK_RECORD_STATE_CFG runRecordstateCfg;

#ifdef __cplusplus
}
#endif
#endif
