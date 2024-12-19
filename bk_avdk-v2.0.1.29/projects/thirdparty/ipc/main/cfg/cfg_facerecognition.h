/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_FACERECONGNITION_H__
#define _SDK_CFG_FACERECONGNITION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***    facerecognition            ***/
/***********************************/


typedef struct {
    SDK_S32             enable;        //是否进行布防
    SDK_S32             sensitive;     //灵敏度 取值0 - 100, 越小越灵敏*/
    SDK_S32             compensation;  // 0 , 1
    SDK_S32             detectionType; // 0 grid, 1 region
} SDK_NET_FR_CFG, *LPSDK_NET_FR_CFG;

extern int FrCfgSave();
extern int FrCfgLoad();
extern void FrCfgPrint();
extern int FrCfgLoadDefValue();
extern cJSON* FrCfgLoadJson();
extern int FrLoadActionsDefValue(SDK_ACTION *action);
extern int FrJsonSaveCfg(cJSON *json);

#define FR_CFG_FILE "facerecognition_cfg.cjson"

extern SDK_NET_FR_CFG runFrCfg;
extern SDK_CFG_MAP FrMap[];

#ifdef __cplusplus
}
#endif
#endif

