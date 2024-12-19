/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_USER_H__
#define _SDK_CFG_USER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***         user                ***/
/***********************************/
#define SDK_MAX_USER_NUM             8
typedef struct {
    SDK_S32   enable;  /*0:无效用户，禁用, 1:启用*/
    SDK_S8    userName[MAX_STR_LEN_64]; /* 用户名最大32字节*/
    SDK_S8    password[MAX_STR_LEN_64]; /* 密码 */
    SDK_U32   userRight; /* 权限，对应掩码决定 ，16 项 */
} SDK_NET_USER_INFO,*LPSDK_NET_USER_INFO;

typedef struct {
    SDK_NET_USER_INFO user[SDK_MAX_USER_NUM]; //最大支持32个用户
} SDK_NET_USER_CFG,*LPSDK_NET_USER_CFG;

extern int UserCfgSave();
extern int UserCfgLoad();
extern void UserCfgPrint();
extern int UserCfgLoadDefValue();

int UserIsExist(const char *userName);
LPSDK_NET_USER_INFO UserGet(const char *userName);
int UserAdd(SDK_NET_USER_INFO *pUser);
int UserModify(SDK_NET_USER_INFO *pUser);
int UserModifyByIndex(int i, SDK_NET_USER_INFO *pUser);
int UserDel(const char *userName);
int UserCheck(SDK_NET_USER_INFO *pUser);

extern cJSON * UserCfgLoadJson();
extern char* UserCfgLoadJsonText(void);
extern int UserCfgSaveJson(char *data);
int UserMatching_login( char *username, char *password);


#define USER_CFG_FILE "user_cfg.cjson"

extern SDK_NET_USER_CFG runUserCfg;

#ifdef __cplusplus
}
#endif
#endif

