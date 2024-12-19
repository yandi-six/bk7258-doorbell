/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_MD_H__
#define _SDK_CFG_MD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***    motion detect            ***/
/***********************************/
#define SDK_NET_MD_GRID_ROW 15
#define SDK_NET_MD_GRID_COLUMN 22
typedef struct {
    SDK_S32 row;       //15
    SDK_S32 column;    //22
    SDK_S8  granularity[SDK_NET_MD_GRID_ROW * SDK_NET_MD_GRID_COLUMN + 1];
} SDK_NET_MD_GRID;

typedef struct {
    SDK_S32 enable;
    SDK_S32 x;
    SDK_S32 y;
    SDK_S32 width;
    SDK_S32 height;
} SDK_NET_MD_REGION;

typedef struct {
    SDK_S32             channel;
    SDK_S32             enable;        //是否进行布防
    SDK_S32             sensitive;     //灵敏度 取值0 - 100, 越小越灵敏*/
    SDK_S32             compensation;  // 0 , 1
    SDK_S32             detectionType; // 0 grid, 1 region
    SDK_NET_MD_GRID      mdGrid;
    SDK_NET_MD_REGION    mdRegion[4];
    SDK_S32              schedule_mode;     //0按布防时间scheduleTime, scheduleSlice   1全时段检测
    //SDK_SCHEDTIME        scheduleTime[7][12];  /*该通道的布防时间*/
    //SDK_ACTION           actions[MAX_ACTIONS];
} SDK_NET_MD_CFG, *LPSDK_NET_MD_CFG;

extern int MdCfgSave();
extern int MdCfgLoad();
extern void MdCfgPrint();
extern int MdCfgLoadDefValue();
extern cJSON* MdCfgLoadJson();
extern int MdLoadActionsDefValue(SDK_ACTION *action);
extern int MdJsonSaveCfg(cJSON *json);

#define MD_CFG_FILE "md_cfg.cjson"

extern SDK_NET_MD_CFG runMdCfg;
extern SDK_CFG_MAP mdMap[];

#ifdef __cplusplus
}
#endif
#endif

