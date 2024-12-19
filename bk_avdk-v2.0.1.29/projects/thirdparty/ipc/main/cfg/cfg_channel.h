/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_SHELTER_H__
#define _SDK_CFG_SHELTER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"


/***********************************/
/***        shelter            ***/
/***********************************/
//遮挡区域设置
typedef struct {
    SDK_S32    enable;     /* 遮挡使能 0-禁用, 1-使能 */
    float     x;          /* 遮挡区域左上角的x坐标 */
    SDK_S8    x_opt[MAX_STR_LEN_128];
    float     y;          /* 遮挡区域左上角的y坐标 */
    SDK_S8    y_opt[MAX_STR_LEN_128];
    float     width;      /* 遮挡区域宽度 */
    SDK_S8    width_opt[MAX_STR_LEN_128];
    float     height;     /* 遮挡区域高度 */
    SDK_S8    height_opt[MAX_STR_LEN_128];
    SDK_U32    color;      /* 遮挡颜色, 默认 0:黑色 按RGB格式*/
} SDK_NET_SHELTER_RECT;

/***********************************/
/***         osd                 ***/
/***********************************/
typedef struct {
    SDK_S32   enable;            /* 0-禁用, 1-使能 */
    SDK_S8    text[MAX_STR_LEN_128];  //通道名称
    float   x;
    SDK_S8    x_opt[MAX_STR_LEN_128];
    float   y;
    SDK_S8    y_opt[MAX_STR_LEN_128];
} SDK_NET_OSD_CHANNEL_NAME;

typedef struct {
    SDK_S32    enable; /* 0-禁用, 1-使能 */
    SDK_U8     dateFormat;
                /*日期格式(年月日格式)：
				0－YYYY-MM-DD		年月日
				1－MM-DD-YYYY		月日年
				2－YYYY/MM/DD		年月日
				3－MM/DD/YYYY		月日年
				4－DD-MM-YYYY		日月年
				5－DD/MM/YYYY		日月年

                */
    SDK_U8     dateSprtr; //0：":"，1："-"，2："/" 3："."
    SDK_U8     timeFmt; //时间格式 (0-24小时，1－12小时).
    float    x;
    SDK_S8    x_opt[MAX_STR_LEN_128];
    float    y;
    SDK_S8    y_opt[MAX_STR_LEN_128];
    SDK_U8		displayWeek; //是否显示星期几
} SDK_NET_OSD_DATETIME;



/***********************************/
/***    channel                  ***/
/***********************************/

typedef struct {
    SDK_NET_OSD_CHANNEL_NAME osdChannelName;
    SDK_NET_OSD_DATETIME     osdDatetime;

} SDK_NET_CHANNEL_INFO, *LPSDK_NET_CHANNEL_INFO;

typedef struct {
    SDK_NET_CHANNEL_INFO channelInfo;
    SDK_NET_SHELTER_RECT shelterRect; //支持4个遮挡区域
} SDK_NET_CHANNEL_CFG, *LPSDK_NET_CHANNEL_CFG;

extern int ChannelCfgSave();
extern int ChannelCfgLoad();
extern void ChannelCfgPrint();
extern int ChannelCfgLoadDefValue();
extern cJSON* ChannelCfgLoadJson();
extern int ChannelJsonSaveCfg(cJSON *json);

#define CHANNEL_CFG_FILE "channel_cfg.cjson"


extern SDK_NET_CHANNEL_CFG runChannelCfg;
extern SDK_CFG_MAP osdMap[];
extern SDK_CFG_MAP shelterRectMap[];

#ifdef __cplusplus
}
#endif
#endif

