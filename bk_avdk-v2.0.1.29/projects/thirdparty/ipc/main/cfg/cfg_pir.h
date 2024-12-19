/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_PIR_H__
#define _SDK_CFG_PIR_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"


/********************************************/
/***    Passive Infrared Radiation(PIR)   ***/
/********************************************/


typedef struct {
    SDK_S32              enable;
} SDK_NET_PIR_CFG, *LPSDK_NET_PIR_CFG;

extern int PirCfgSave();
extern int PirCfgLoad();
extern void PirCfgPrint();
extern int PirCfgLoadDefValue();
extern cJSON* PirCfgLoadJson();
extern int PirLoadActionsDefValue(SDK_ACTION *action);
extern int PirJsonSaveCfg(cJSON *json);

#define PIR_CFG_FILE "pir_cfg.cjson"

extern SDK_NET_PIR_CFG runPirCfg;
extern SDK_CFG_MAP pirMap[];

#ifdef __cplusplus
}
#endif
#endif

