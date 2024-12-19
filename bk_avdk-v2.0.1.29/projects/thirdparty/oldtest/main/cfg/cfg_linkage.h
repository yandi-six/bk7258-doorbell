/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_LINKAGE_H__
#define _SDK_CFG_LINKAGE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"


/********************************************/
/***    Equipment linkage   ***/
/********************************************/


typedef struct {
    SDK_S32              enable;
} SDK_NET_LINKAGE_CFG, *LPSDK_NET_LINKAGE_CFG;

extern int LinkageCfgSave();
extern int LinkageCfgLoad();
extern void LinkageCfgPrint();
extern int LinkageCfgLoadDefValue();
extern int LinkageLoadActionsDefValue(SDK_ACTION *action);
extern cJSON* LinkageCfgLoadJson();
extern int LinkageJsonSaveCfg(cJSON *json);

#define LINKAGE_CFG_FILE "linkage_cfg.cjson"

extern SDK_NET_LINKAGE_CFG runLinkageCfg;
extern SDK_CFG_MAP linkageMap[];

#ifdef __cplusplus
}
#endif
#endif

