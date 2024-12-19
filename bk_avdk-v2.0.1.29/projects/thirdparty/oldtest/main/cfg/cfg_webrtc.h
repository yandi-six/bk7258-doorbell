
#ifndef _SDK_CFG_WEBRTC_H__
#define _SDK_CFG_WEBRTC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

typedef struct {
    SDK_S32            stunport;
    SDK_S8             activeState;
} SDK_NET_WEBRTC_CFG, *LPSDK_NET_WEBRTC_CFG;


extern int WebRTCCfgSave();
extern int WebRTCCfgLoad();
extern void WebRTCCfgPrint();
extern int WebRTCCfgLoadDefValue();
extern cJSON* WebRTCCfgLoadJson();
extern int WebRTCJsonSaveCfg(cJSON *json);
#define WEBRTC_CFG_FILE "webrtc_cfg.cjson"

extern SDK_NET_WEBRTC_CFG runWebRTCCfg;
extern SDK_CFG_MAP webrtcMap[];


#ifdef __cplusplus
}
#endif
#endif

