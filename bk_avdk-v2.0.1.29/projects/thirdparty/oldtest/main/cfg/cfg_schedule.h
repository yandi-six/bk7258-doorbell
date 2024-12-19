/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_SCHEDULE_H__
#define _SDK_CFG_SCHEDULE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"


/********************************************/
/***    Schedule   ***/
/********************************************/


typedef struct {
    SDK_S32              enable;
} SDK_NET_SCHEDULE_CFG, *LPSDK_NET_SCHEDULE_CFG;

extern int ScheduleCfgSave();
extern int ScheduleCfgLoad();
extern void ScheduleCfgPrint();
extern int ScheduleCfgLoadDefValue();
extern int ScheduleLoadActionsDefValue(SDK_ACTION *action);
extern cJSON* ScheduleCfgLoadJson();
extern int ScheduleJsonSaveCfg(cJSON *json);

#define SCHEDULE_CFG_FILE "schedule_cfg.cjson"

extern SDK_NET_SCHEDULE_CFG runScheduleCfg;
extern SDK_CFG_MAP scheduleMap[];

#ifdef __cplusplus
}
#endif
#endif

