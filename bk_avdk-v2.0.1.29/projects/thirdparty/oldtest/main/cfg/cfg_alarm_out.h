
#ifndef _SDK_CFG_ALARM_OUT_H__
#define _SDK_CFG_ALARM_OUT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"
#include "cfg_ptz.h"


/*±¨¾¯Êä³ö*/
typedef struct {
    SDK_S32   channel;
    SDK_S8    alarmOutName[MAX_STR_LEN_32];   /* Ãû³Æ */
    SDK_S8    defaultState;       /* 0-low 1-high */
    SDK_S8    activeState;        /* 0-low 1-high*/
    SDK_S8    powerOnState;       /* 0-pulse 1-continuous*/
    SDK_S32   pulseDuration;      /* 1000 - 10000*/
} SDK_NET_ALARM_OUT_CFG, *LPSDK_NET_ALARM_OUT_CFG;


extern int AlarmOutCfgSave();
extern int AlarmOutCfgLoad();
extern void AlarmOutCfgPrint();
extern int AlarmOutCfgLoadDefValue();
extern cJSON* AlarmOutCfgLoadJson();
extern int AlarmOutJsonSaveCfg(cJSON *json);
#define ALARMOUT_CFG_FILE "alarm_out_cfg.cjson"

extern SDK_NET_ALARM_OUT_CFG runAlarmOutCfg;
extern SDK_CFG_MAP alarmOutMap[];

#ifdef __cplusplus
}
#endif
#endif

