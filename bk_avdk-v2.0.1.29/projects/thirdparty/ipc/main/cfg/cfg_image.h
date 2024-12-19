/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_IMAGE_H__
#define _SDK_CFG_IMAGE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***          image              ***/
/***********************************/
typedef struct {
    SDK_U8	    horizontal; //垂直 翻转
    SDK_U8	    vertical; // 水平 翻转

    SDK_U8     sceneMode;  // 场景模式: 0 auto 1 indoor 2 outdoor
    SDK_U8     imageStyle; // 图像风格: 0 标准 1 明亮 2 艳丽
    SDK_U8     wbMode;     // 白平衡 0 auto 1 indoor 2 outdoor

    SDK_U8     irCutControlMode; // 0 software  ,  1 hardware
    SDK_U8     irCutMode;        // ircut日夜模式 0 auto, 1 day, 2 night
    SDK_U8     enabledWDR;
    SDK_U8     strengthWDR; //宽动态范围 1-5
    SDK_U8     enableDenoise3d;
    SDK_U8     strengthDenoise3d; // 3D去噪强度 1-5 对应SDK 0-512

    SDK_U8     lowlightMode; // 低照度模式: 0 close, 1 only night, 2 day-night, 3 auto
    SDK_U8     exposureMode; /* 曝光模式 0 - auto 1 - bright, 2 - dark */
    SDK_U8     dcIrisEnable; /* 1 - enable 0 - disable */
    SDK_U8     antiFlickerFreq; /* 50: 60  50HZ 60HZ */
    SDK_U8     backLightEnable;
    SDK_S32    backLightLevel;

    SDK_S32    brightness; /* 0 ~ 100 */
    SDK_S32    brightness_default; /* 0 ~ 100 */
    SDK_S8     brightness_opt[MAX_STR_LEN_128];
    SDK_S32    saturation; /* 0 ~ 100  */
    SDK_S32    saturation_default; /* 0 ~ 100  */
    SDK_S8     saturation_opt[MAX_STR_LEN_128];
    SDK_S32    contrast;   /* 0 ~ 100 */
    SDK_S32    contrast_default;   /* 0 ~ 100 */
    SDK_S8     contrast_opt[MAX_STR_LEN_128];
    SDK_S32    sharpness;  /* 0 ~ 100 */
    SDK_S32    sharpness_default;  /* 0 ~ 100 */
    SDK_S8     sharpness_opt[MAX_STR_LEN_128];  
    SDK_S32    hue;        /* 0 ~ 100 */
    SDK_S32    hue_default;        /* 0 ~ 100 */
    SDK_S8     hue_opt[MAX_STR_LEN_128];
} SDK_NET_IMAGE_CFG;

extern int ImageCfgSave();
extern int ImageCfgLoad();
extern void ImageCfgPrint();
extern int ImageCfgLoadDefValue();
extern cJSON * ImageCfgLoadJsonFromFile();
extern cJSON* ImageCfgLoadJson();
extern int ImageJsonSaveCfg(cJSON *json);

#define IMAGE_CFG_FILE "image_cfg.cjson"

extern SDK_NET_IMAGE_CFG runImageCfg;
extern SDK_CFG_MAP imageMap[];

#ifdef __cplusplus
}
#endif
#endif

