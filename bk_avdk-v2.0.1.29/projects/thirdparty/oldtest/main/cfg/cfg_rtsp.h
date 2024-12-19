
#ifndef _SDK_CFG_RTSP_H__
#define _SDK_CFG_RTSP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

typedef struct {
    SDK_S8             enable;
    SDK_S32            port;
    SDK_S8             activeState;
} SDK_NET_RTSP_CFG, *LPSDK_NT_RTSP_CFG;


extern int RtspCfgSave();
extern int RtspCfgLoad();
extern void RtspCfgPrint();
extern int RtspCfgLoadDefValue();
extern cJSON* RtspCfgLoadJson();
extern int RtspJsonSaveCfg(cJSON *json);
#define RTSP_CFG_FILE "rtsp_cfg.cjson"

extern SDK_NET_RTSP_CFG runRtspCfg;
extern SDK_CFG_MAP rtspMap[];


#ifdef __cplusplus
}
#endif
#endif

