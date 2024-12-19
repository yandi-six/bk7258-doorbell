
#ifndef _SDK_CFG_ALARM_IN_H__
#define _SDK_CFG_ALARM_IN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

typedef struct {
    SDK_S32            channel;
    SDK_S8             alarmInName[MAX_STR_LEN_32];    /* Ãû³Æ */
    SDK_S8             defaultState;       /* 0-low 1-high */
    SDK_S8             activeState;        /* 0-low 1-high*/
} SDK_NET_ALARM_IN_CFG, *LPSDK_NET_ALARM_IN_CFG;


extern int AlarmInCfgSave();
extern int AlarmInCfgLoad();
extern void AlarmInCfgPrint();
extern int AlarmInCfgLoadDefValue();
extern cJSON* AlarmInCfgLoadJson();
extern int AlarmInLoadActionsDefValue(SDK_ACTION *action);
extern int AlarmInJsonSaveCfg(cJSON *json);
#define ALARMIN_CFG_FILE "alarm_in_cfg.cjson"

extern SDK_NET_ALARM_IN_CFG runAlarmInCfg;
extern SDK_CFG_MAP alarmInMap[];


#ifdef __cplusplus
}
#endif
#endif

