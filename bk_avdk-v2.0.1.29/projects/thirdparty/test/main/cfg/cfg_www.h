
#ifndef _SDK_CFG_WWW_H__
#define _SDK_CFG_WWW_H__

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
} SDK_NET_WWW_CFG, *LPSDK_NT_WWW_CFG;


extern int WebCfgSave();
extern int WebCfgLoad();
extern void WebCfgPrint();
extern int WebCfgLoadDefValue();
extern cJSON* WebCfgLoadJson();
#define WWW_CFG_FILE "www_cfg.cjson"

extern SDK_NET_WWW_CFG runWWWCfg;
extern SDK_CFG_MAP wwwMap[];


#ifdef __cplusplus
}
#endif
#endif

