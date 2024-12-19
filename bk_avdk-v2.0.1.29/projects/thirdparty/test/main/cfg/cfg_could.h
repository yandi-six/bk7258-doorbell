
#ifndef _SDK_CFG_COULD_H__
#define _SDK_CFG_COULD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

typedef struct {
    SDK_S32   enableEmail;      //ÊÇ·ñÆôÓÃ
    SDK_S32   attachPicture;    //ÊÇ·ñŽøžœŒþ
    SDK_U8    smtpServerVerify; //·¢ËÍ·þÎñÆ÷ÒªÇóÉí·ÝÑéÖ€
    SDK_U8    mailInterval;     //×îÉÙ2sÖÓ(1-2Ãë£»2-3Ãë£»3-4Ãë£»4-5Ãë)

    SDK_S8    eMailUser[MAX_STR_LEN_64];    //ÕËºÅ
    SDK_S8    eMailPass[MAX_STR_LEN_64];    //ÃÜÂë
    SDK_S32   encryptionType;   /**< ŒÓÃÜÀàÐÍ ssl*/
    SDK_S8    smtpServer[MAX_STR_LEN_128];   //smtp·þÎñÆ÷  //ÓÃÓÚ·¢ËÍÓÊŒþ
    SDK_S32   smtpPort;      /**< ·þÎñÆ÷¶Ë¿Ú,Ò»°ãÎª25£¬ÓÃ»§žùŸÝŸßÌå·þÎñÆ÷ÉèÖÃ */
    SDK_S8    pop3Server[MAX_STR_LEN_128];   //pop3·þÎñÆ÷  //ÓÃÓÚœÓÊÕÓÊŒþ,ºÍIMAPÐÔÖÊÀàËÆ
    SDK_S32   pop3Port;      /**< ·þÎñÆ÷¶Ë¿Ú,Ò»°ãÎª25£¬ÓÃ»§žùŸÝŸßÌå·þÎñÆ÷ÉèÖÃ */
    SDK_S8    fromAddr[MAX_STR_LEN_64]; 	/**< ·¢ËÍÈËµØÖ· */
    SDK_S8    toAddrList0[MAX_STR_LEN_64];  /**< ÊÕŒþÈËµØÖ· 1 */
    SDK_S8    toAddrList1[MAX_STR_LEN_64];  /**< ÊÕŒþÈËµØÖ· 2 */
    SDK_S8    toAddrList2[MAX_STR_LEN_64];  /**< ÊÕŒþÈËµØÖ· 3 */
    SDK_S8    toAddrList3[MAX_STR_LEN_64];  /**< ÊÕŒþÈËµØÖ· 4 */
    SDK_S8    ccAddrList0[MAX_STR_LEN_64];  /**< ÊÕŒþÈËµØÖ· 0 */
    SDK_S8    ccAddrList1[MAX_STR_LEN_64];  /**< ÊÕŒþÈËµØÖ· 1 */
    SDK_S8    ccAddrList2[MAX_STR_LEN_64];  /**< ÊÕŒþÈËµØÖ· 2 */
    SDK_S8    ccAddrList3[MAX_STR_LEN_64];  /**< ÊÕŒþÈËµØÖ· 3 */
    SDK_S8    bccAddrList0[MAX_STR_LEN_64]; /**< ÃÜËÍÈËµØÖ· 0 */
    SDK_S8    bccAddrList1[MAX_STR_LEN_64]; /**< ÃÜËÍÈËµØÖ· 1 */
    SDK_S8    bccAddrList2[MAX_STR_LEN_64]; /**< ÃÜËÍÈËµØÖ· 2 */
    SDK_S8    bccAddrList3[MAX_STR_LEN_64]; /**< ÃÜËÍÈËµØÖ· 3 */
} SDK_NET_COULD_CFG, *LPSDK_NET_COULD_CFG;


extern int CouldCfgSave();
extern int CouldCfgLoad();
extern void CouldCfgPrint();
extern int CouldCfgLoadDefValue();
extern cJSON* CouldCfgLoadJson();
extern int CouldJsonSaveCfg(cJSON *json);
#define COULD_CFG_FILE "could_cfg.cjson"

extern SDK_NET_COULD_CFG runCouldCfg;
extern SDK_CFG_MAP couldMap[];


#ifdef __cplusplus
}
#endif
#endif

