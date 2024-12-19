/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_PTZ_H__
#define _SDK_CFG_PTZ_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"


/***********************************/
/***         ptz                 ***/
/***********************************/
/*云台解码器485串口参数*/
typedef struct {
    SDK_U32 channel;         /* 对应通道 */
    SDK_U32 baudRate;        /* 波特率(bps)，600bps -- 115200bps */
    SDK_U8  dataBit;         /* 数据有几位 0－5位，1－6位，2－7位，3－8位 */
    SDK_U8  stopBit;         /* 停止位 0－1位，1－2位  */
    SDK_U8  parity;          /* 校验 0－无校验，1－奇校验，2－偶校验; */
    SDK_U8  flowcontrol;     /* 0－无，1－软流控,2-硬流控 */
    SDK_S8  decoderType[32]; /* 解码器类型, 见下表*/
    SDK_S32 workMode;        /* 工作模式，0-窄带传输(232串口用于PPP拨号),  1-控制台(232串口用于参数控制),  2-透明通道 */
    SDK_U16 decoderAddress;  /* 解码器地址:0 - 255*/
    SDK_U16 speedH;          /* 云台H速度*/
    SDK_U16 speedV;          /* 云台V速度*/
    SDK_U16 watchPos;        /* 看守位*/
    SDK_U32 res;
} SDK_NET_DECODERCFG,*LPSDK_NET_DECODERCFG;

#define SDK_PTZ_PROTOCOL_NUM        200     /* 最大支持的云台协议数 */
//云台协议表结构配置
typedef struct {
    SDK_U32 type;         /*解码器类型值，从1开始连续递增*/
    SDK_S8  describe[32]; /*解码器的描述符*/
} SDK_PTZ_PROTOCOL;


// 云台联动
typedef struct {
    SDK_U8  type;//详见SDK_PTZ_LINK_TYPE，0:无效，1:预置点，2:点间巡航，3:启用轨迹
    SDK_U8  value;
    SDK_U8  reserve[2];
} SDK_PTZ_LINK, *LPSDK_PTZ_LINK;

/*232串口参数*/
typedef struct {
    SDK_U32 baudRate; /* 波特率(bps) */
    SDK_U8  dataBit; /* 数据有几位 0－5位，1－6位，2－7位，3－8位; */
    SDK_U8  stopBit; /* 停止位 0－1位，1－2位; */
    SDK_U8  parity; /* 校验 0－无校验，1－奇校验，2－偶校验; */
    SDK_U8  flowcontrol; /* 0－无，1－软流控,2-硬流控 */
    SDK_U32 workMode; /* 工作模式，0－窄带传输（232串口用于PPP拨号），1－控制台（232串口用于参数控制），2－透明通道 */
} SDK_NET_RS232CFG, *LPSDK_NET_RS232CFG;

typedef struct {
    SDK_NET_DECODERCFG decoderCfg;
    SDK_S32          ptzNum;                        /*有效的ptz协议数目*/
    SDK_S8           describe[32];                  /*解码器的描述符*/
    SDK_PTZ_PROTOCOL   protocol[SDK_PTZ_PROTOCOL_NUM]; /*最大200中PTZ协议*/
} SDK_NET_PTZ_PROTOCOL_CFG;

typedef struct {
    SDK_S32     id;
    SDK_S32     serialPortID;
    SDK_S32     videoInputID;
    SDK_S32     duplexMode;   // 0-half, 1-full
    SDK_S32     controlType;  // 0- controlType, 1- external
    SDK_S32     protocol; // 0 pelco-d, 1 pelco-p
    SDK_S32     address;
} SDK_NET_PTZ_CFG;



#define PTZ_MAX_PRESET             255
#define PTZ_MAX_CRUISE_POINT_NUM   32
#define PTZ_MAX_CRUISE_GROUP_NUM   5

/* PTZ预制点功能结构体 */
typedef struct tagSDK_NET_PRESET_INFO
{
    SDK_U16   nChannel;
    SDK_U16   nPresetNum;                   //预置点个数
    SDK_U32   no[PTZ_MAX_PRESET];           //预置点序号
    SDK_S8    csName[PTZ_MAX_PRESET][64];   //预置点名称
}SDK_NET_PRESET_INFO;


/* PTZ 巡航功能结构体 */
typedef struct tagSDK_NET_CRUISE_POINT
{
	SDK_S8 	byPointIndex;   //巡航组中的下标,如果值大于PTZ_MAX_CRUISE_POINT_NUM 表示添加到末尾
	SDK_S8 	byPresetNo;     //预置点编号
	SDK_S8 	byRemainTime;   //预置点滞留时间
	SDK_S8 	bySpeed;        //到预置点速度
}SDK_NET_CRUISE_POINT;

typedef struct tagSDK_NET_CRUISE_GROUP
{
	SDK_S8 byPointNum;        //预置点数量
	SDK_S8 byCruiseIndex;     //本巡航的序号
	SDK_S8 byRes[2];
	SDK_NET_CRUISE_POINT struCruisePoint[PTZ_MAX_CRUISE_POINT_NUM];
}SDK_NET_CRUISE_GROUP;

typedef struct tagSDK_NET_CRUISE_CFG
{
	SDK_S32  nChannel;
	SDK_S8   byIsCruising;		//是否在巡航
	SDK_S8   byCruisingIndex;	//正在巡航的巡航编号
	SDK_S8   byPointIndex;		//正在巡航的预置点序号(数组下标)
	SDK_S8   byEnableCruise;;	//是否开启巡航
	SDK_NET_CRUISE_GROUP struCruise[PTZ_MAX_CRUISE_GROUP_NUM];
}SDK_NET_CRUISE_CFG;



extern int PtzCfgSave();
extern int PtzCfgLoad();
extern void PtzCfgPrint();
extern int PtzCfgLoadDefValue();
extern cJSON * PtzCfgLoadJson();
extern int PtzJsonSaveCfg(cJSON *json);


extern int PresetCruiseCfgSave();
extern int PresetCruiseCfgLoad();
extern int PresetCruiseCfgDefault();


#define PTZ_CFG_FILE    "ptz_cfg.cjson"
#define PTZ_CRUISE_FILE "ptz_cruise.cfg"

extern SDK_NET_PTZ_CFG     runPtzCfg;
extern SDK_NET_DECODERCFG  runPtzDecCfg;

extern SDK_NET_PRESET_INFO runPresetCfg;
extern SDK_NET_CRUISE_CFG  runCruiseCfg;

extern SDK_CFG_MAP ptzMap[];

#ifdef __cplusplus
}
#endif
#endif

