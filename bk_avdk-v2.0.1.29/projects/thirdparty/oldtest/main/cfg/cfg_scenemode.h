/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_SCENEMODE_H__
#define _SDK_CFG_SCENEMODE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"



typedef struct {
    SDK_S8    scenemodeName[MAX_STR_LEN_32];
    SDK_EVENT events[MAX_EVENTS]; 
} SDK_NET_SCENEMODE, *LPSDK_NET_SCENEMODE;

typedef struct {
    SDK_S32 selected;
    SDK_ACTION actions[MAX_ACTIONS];
    SDK_NET_SCENEMODE scenemodes[MAX_SCENEMODES];
} SDK_NET_SCENEMODSECFG, *LPSDK_NET_SCENEMODESCFG;
extern int ScenemodeCfgSave();
extern int ScenemodeCfgLoad();
extern int ScenemodeCfgLoadDefValue();
extern cJSON * ScenemodeCfgLoadJson();
extern int ScenemodeJsonSaveCfg(cJSON *json);
extern int ScenemodeLoadActionsDefValue(SDK_ACTION *action);

#define SCENEMODE_FILE "scenemode_info.cjson"


extern SDK_NET_SCENEMODSECFG runScenemodeCfg;
extern SDK_CFG_MAP scenemodeMap[];


#ifdef __cplusplus
}
#endif
#endif

