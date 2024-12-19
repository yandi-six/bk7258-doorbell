/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_SNAP_H__
#define _SDK_CFG_SNAP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***         snap                ***/
/***********************************/
/* 定时抓图 */
typedef struct {
    SDK_S32     enable;
    SDK_SCHEDTIME scheduleTime[7][12];/**该通道的videoloss的布防时间*/
    SDK_S32     interval;           // 几ms一次
    SDK_S32     nums;               // 一次几张
    SDK_S32     pictureQuality;     // 最好:0 好:1  一般:2  不好:3差:4
    SDK_S32     imageSize;          // 画面大小；0: 1080p 1: 720p 2 D1 3：CIF，4：QCIF
    SDK_S32     snapShotImageType;  // 0 JPEG；1 bmp
    SDK_S32     storagerMode;       //存储模式(0:本地, 1:FTP, 2: FTP|LOCAL(优先FTP,FTP失败后录像本地))
    SDK_S32     channelID;          //通道号
} SDK_TIMER_SNAP;

/*事件抓图*/
typedef struct {
	SDK_S32     enable;
	SDK_S32     interval;           //抓拍间隔ms
	SDK_S32     nums;               // 一次几张
	SDK_S32     pictureQuality;     // 按数值 0- 100 ，100% 最好
	SDK_S32     imageSize;          // 画面大小
	SDK_S32     snapShotImageType;  // 0 JPEG；1 bmp
	SDK_S32     storagerMode;       //存储模式(0:本地
	SDK_S32     channelID;          //通道号
}SDK_EVENT_SNAP;

typedef struct {
	SDK_TIMER_SNAP timer_snap;
	SDK_EVENT_SNAP event_snap;
} SDK_NET_SNAP_CFG, *LPSDK_NET_SNAP_CFG;

extern int SnapCfgSave();
extern int SnapCfgLoad();
extern void SnapCfgPrint();
extern int SnapCfgLoadDefValue();
extern cJSON * SnapCfgLoadJson();
extern int SnapJsonSaveCfg(cJSON *json);

#define SNAP_CFG_FILE "snap_cfg.cjson"


extern SDK_NET_SNAP_CFG runSnapCfg;
extern SDK_CFG_MAP snapTimerMap[];
extern SDK_CFG_MAP snapEventMap[];

#ifdef __cplusplus
}
#endif
#endif

