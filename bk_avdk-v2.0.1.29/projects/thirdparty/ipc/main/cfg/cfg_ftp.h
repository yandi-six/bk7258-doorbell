
#ifndef _SDK_CFG_FTP_H__
#define _SDK_CFG_FTP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

typedef struct {
    SDK_S32   enableFTP;       /*ÊÇ·ñÆô¶¯ftpÉÏŽ«¹ŠÄÜ*/
    SDK_S8    address[MAX_STR_LEN_128];     /*ftp ·þÎñÆ÷£¬¿ÉÒÔÊÇIPµØÖ·»òÓòÃû*/
    SDK_S32   port;            /*ftp¶Ë¿Ú*/
    SDK_S8    userName[MAX_STR_LEN_64];    /*ÓÃ»§Ãû*/
    SDK_S8    password[MAX_STR_LEN_64];    /*ÃÜÂë*/
    SDK_S8    datapath[MAX_STR_LEN_128];
    SDK_S8    filename[MAX_STR_LEN_128];
    SDK_S32   interval;
} SDK_NET_FTP_CFG, *LPSDK_NET_FTP_CFG;


extern int FtpCfgSave();
extern int FtpCfgLoad();
extern void FtpCfgPrint();
extern int FtpCfgLoadDefValue();
extern cJSON* FtpCfgLoadJson();
extern int FtpJsonSaveCfg(cJSON *json);
#define FTP_CFG_FILE "ftp_cfg.cjson"

extern SDK_NET_FTP_CFG runFtpCfg;
extern SDK_CFG_MAP ftpMap[];


#ifdef __cplusplus
}
#endif
#endif

