
#ifndef _SDK_CFG_ONVIF_H__
#define _SDK_CFG_ONVIF_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

typedef struct {
    SDK_S8             httpEnable;
    SDK_S32            httpPort;
    SDK_S8             httpActiveState;
    SDK_S8             httpsEnable;
    SDK_S32            httpsPort;
    SDK_S8             httpsActiveState;
} SDK_NET_ONVIF_CFG, *LPSDK_NT_ONVIF_CFG;


extern int OnvifCfgSave();
extern int OnvifCfgLoad();
extern void OnvifCfgPrint();
extern int OnvifCfgLoadDefValue();
extern cJSON* OnvifCfgLoadJson();
extern int OnvifJsonSaveCfg(cJSON *json);
#define ONVIF_CFG_FILE "onvif_cfg.cjson"

extern SDK_NET_ONVIF_CFG runOnvifCfg;
extern SDK_CFG_MAP OnvifMap[];


#ifdef __cplusplus
}
#endif
#endif

