/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_ROI_H__
#define _SDK_CFG_ROI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

    /***********************************/
    /***         record              ***/
    /***********************************/
    typedef struct
    {
        SDK_S32 whole_region;
        SDK_S32 start_x;
        SDK_S32 start_y;
        SDK_S32 end_x;
        SDK_S32 end_y;
    } SDK_NET_ROI_CFG, *LPSDK_NET_ROI_CFG;



    extern int RoiCfgSave();
    extern int RoiCfgLoad();
    extern void RoiCfgPrint();
    extern int RoiCfgLoadDefValue();
    extern cJSON *RoiCfgLoadJson();
    extern int RoiJsonSaveCfg(cJSON *json);

#define ROI_CFG_FILE "roi_cfg.cjson"

    extern SDK_NET_ROI_CFG runRoiCfg;
    extern SDK_CFG_MAP roiMap[];

#ifdef __cplusplus
}
#endif
#endif
