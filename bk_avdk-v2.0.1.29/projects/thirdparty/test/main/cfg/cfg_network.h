/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_NETWORK_H__
#define _SDK_CFG_NETWORK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

#define WIFI_CONNECT_LIST_MAX_NUMBER 40

/***********************************/
/***       network               ***/
/***********************************/
//�������ýṹ
typedef enum {
    SDK_NET_TYPE_DHCP = 0,
    SDK_NET_TYPE_STATIC,
} SDK_NET_TYPE_E;

typedef struct {
    SDK_S32  enable;
    SDK_S8   netName[MAX_STR_LEN_64]; //�������ƣ����ڶ���������
    SDK_S32  ipVersion; //v4,v6
    SDK_S8   mac[MAX_STR_LEN_20];   //Mac��ַ
    SDK_S32  dhcpIp; // 0 static, 1 dhcp
    SDK_S32  upnpEnable; // 0 disable, 1 enable
    SDK_S8   ip[MAX_STR_LEN_16];
    SDK_S8   netmask[MAX_STR_LEN_16];  //�����ַ
    SDK_S8   gateway[MAX_STR_LEN_16];
    SDK_S8   multicast[MAX_STR_LEN_16];
    SDK_S32  dhcpDns; // 0 static, 1 dhcp
    SDK_S8   dns1[MAX_STR_LEN_16];
    SDK_S8   dns2[MAX_STR_LEN_16];
} SDK_NET_ETH, *LPSDK_NET_ETH;

/**UPNP**/
typedef struct {
    SDK_U32  enable;               /*�Ƿ�����upnp*/
    SDK_U32  mode;                 /*upnp������ʽ.0Ϊ�Զ��˿�ӳ�䣬1Ϊָ���˿�ӳ��*/
    SDK_U32  lineMode;             /*upnp����������ʽ.0Ϊ��������,1Ϊ��������*/
    SDK_S8   serverIp[MAX_STR_LEN_32];         /*upnpӳ������.������·����IP*/
    SDK_U32  dataPort;             /*upnpӳ�����ݶ˿�*/
    SDK_U32  webPort;              /*upnpӳ������˿�*/
    SDK_U32  mobilePort;           /*upnpӳ���ֻ��˿�*/
    SDK_U16  dataPort1;            /*upnp��ӳ��ɹ������ݶ˿�*/
    SDK_U16  webPort1;             /*upnp��ӳ��ɹ�������˿�*/
    SDK_U16  mobilePort1;          /*upnpӳ��ɹ����ֻ��˿�*/
    SDK_U16  dataPortOK;
    SDK_U16  webPortOK;
    SDK_U16  mobilePortOK;
} SDK_UPNP_CFG, *LPSDK_UPNP_CFG;

typedef struct {
    SDK_U32 enable;       //0-������,1-����
    SDK_S8  user[MAX_STR_LEN_64];     //PPPoE�û���
    SDK_S8  password[MAX_STR_LEN_64]; //PPPoE����
} SDK_NET_PPPOECFG, *LPSDK_NET_PPPOECFG;

typedef enum {
    SDK_DDNS_DYNDNS = 0,
    SDK_DDNS_3322,
    SDK_DDNS_NIGHTOWLDVR,
    SDK_DDNS_NOIP,
    SDK_DDNS_MYEYE,
    SDK_DDNS_PEANUTHULL,
    SDK_DDNS_BUTT,
    SDK_DDNS_CHANGEIP,
    SDK_DDNS_POPDVR,
    SDK_DDNS_SKYBEST,
    SDK_DDNS_DVRTOP,
} SDK_DDNS_TYPE;

typedef struct {
    SDK_U32     enableDDNS;   //�Ƿ�����DDNS
    SDK_DDNS_TYPE type;         //DDNS����������, �����������ͣ�SDK_DDNS_TYPE
    SDK_S8      username[MAX_STR_LEN_64];
    SDK_S8      password[MAX_STR_LEN_64];
    SDK_S8      domain[MAX_STR_LEN_256];   //��DDNS������ע���������ַ
    SDK_S8      address[MAX_STR_LEN_256];  //DNS��������ַ��������IP��ַ������ www.dynddns.org
    SDK_S32     port;         //DNS�������˿ڣ�Ĭ��Ϊ6500
} SDK_NET_DDNSCFG, *LPSDK_NET_DDNSCFG;

typedef struct {
    SDK_S32   enableEmail;      //�Ƿ�����
    SDK_S32   attachPicture;    //�Ƿ������
    SDK_U8    smtpServerVerify; //���ͷ�����Ҫ��������֤
    SDK_U8    mailInterval;     //����2s��(1-2�룻2-3�룻3-4�룻4-5��)

    SDK_S8    eMailUser[MAX_STR_LEN_64];    //�˺�
    SDK_S8    eMailPass[MAX_STR_LEN_64];    //����
    SDK_S32   encryptionType;   /**< �������� ssl*/
    SDK_S8    smtpServer[MAX_STR_LEN_128];   //smtp������  //���ڷ����ʼ�
    SDK_S32   smtpPort;      /**< �������˿�,һ��Ϊ25���û����ݾ������������ */
    SDK_S8    pop3Server[MAX_STR_LEN_128];   //pop3������  //���ڽ����ʼ�,��IMAP��������
    SDK_S32   pop3Port;      /**< �������˿�,һ��Ϊ25���û����ݾ������������ */
    SDK_S8    fromAddr[MAX_STR_LEN_64]; 	/**< �����˵�ַ */
    SDK_S8    toAddrList0[MAX_STR_LEN_64];  /**< �ռ��˵�ַ 1 */
    SDK_S8    toAddrList1[MAX_STR_LEN_64];  /**< �ռ��˵�ַ 2 */
    SDK_S8    toAddrList2[MAX_STR_LEN_64];  /**< �ռ��˵�ַ 3 */
    SDK_S8    toAddrList3[MAX_STR_LEN_64];  /**< �ռ��˵�ַ 4 */
    SDK_S8    ccAddrList0[MAX_STR_LEN_64];  /**< �ռ��˵�ַ 0 */
    SDK_S8    ccAddrList1[MAX_STR_LEN_64];  /**< �ռ��˵�ַ 1 */
    SDK_S8    ccAddrList2[MAX_STR_LEN_64];  /**< �ռ��˵�ַ 2 */
    SDK_S8    ccAddrList3[MAX_STR_LEN_64];  /**< �ռ��˵�ַ 3 */
    SDK_S8    bccAddrList0[MAX_STR_LEN_64]; /**< �����˵�ַ 0 */
    SDK_S8    bccAddrList1[MAX_STR_LEN_64]; /**< �����˵�ַ 1 */
    SDK_S8    bccAddrList2[MAX_STR_LEN_64]; /**< �����˵�ַ 2 */
    SDK_S8    bccAddrList3[MAX_STR_LEN_64]; /**< �����˵�ַ 3 */
} SDK_NET_EMAIL_PARAM, *LPSDK_NET_EMAIL_PARAM;

/* ftp�ϴ�����*/
typedef struct {
    SDK_S32   enableFTP;       /*�Ƿ�����ftp�ϴ�����*/
    SDK_S8    address[MAX_STR_LEN_128];     /*ftp ��������������IP��ַ������*/
    SDK_S32   port;            /*ftp�˿�*/
    SDK_S8    userName[MAX_STR_LEN_64];    /*�û���*/
    SDK_S8    password[MAX_STR_LEN_64];    /*����*/
    SDK_S8    datapath[MAX_STR_LEN_128];
    SDK_S8    filename[MAX_STR_LEN_128];
    SDK_S32   interval;
} SDK_NET_FTP_PARAM, *LPSDK_NET_FTP_PARAM;

typedef struct {
    SDK_U32 enable;
    SDK_U32 isConfig; 
    SDK_S32 mode; //0 accessPoSDK_S32, 1 stationMode
    SDK_S32 staMode; //802.11bgn mixed
    SDK_U8  essid[32];
    SDK_U8  passd[32];
    SDK_S8  api_url[64];
    SDK_S32 apBssId;
    SDK_S32 apEssId;
    SDK_S32 apPsk;
    SDK_S32 fixedBpsModeEnabled;
    SDK_S32 bssId;
    SDK_S32 essId;
    SDK_S32 Psk;
    SDK_S32 apMode; //["802.11b","802.11g","802.11n","802.11bg","802.11bgn"]
    SDK_S32 apMode80211nChannel; // ["Auto","1","2","3","4","5","6","7","8","9","10","11","12","13","14"]
    SDK_S32 essIdBroadcastingEnabled;
    SDK_S32 wpaMode; //["WPA_PSK","WPA2_PSK"]
} SDK_NET_WIRELESS_CFG, *LPSDK_NET_WIRELESS_CFG;




typedef struct {
    SDK_NET_ETH          lan;
    SDK_NET_ETH          wifi;
    SDK_NET_WIRELESS_CFG wireless;
    SDK_UPNP_CFG         upnp;
    SDK_NET_PPPOECFG     pppoe;
    SDK_NET_DDNSCFG      ddns;
} SDK_NET_NETWORK_CFG, *LPSDK_NET_NETWORK_CFG;
extern int NetworkCfgDelete();
extern int NetworkCfgSave();
extern int NetworkCfgLoad();
extern void NetworkCfgPrint();
extern int NetworkCfgLoadDefValue();
extern char *NetworkCfgGetJosnString(int type);
extern cJSON * NetworkCfgLoadJson();

extern cJSON *NetworkCfgGetLanJosn();
extern cJSON *NetworkCfgGetWifiJosn();

#define NETWORK_CFG_FILE "network_cfg.cjson"

extern SDK_NET_NETWORK_CFG runNetworkCfg;
extern SDK_CFG_MAP lanMap[];
extern SDK_CFG_MAP wifiMap[];
extern SDK_CFG_MAP wirelessMap[];

#ifdef __cplusplus
}
#endif
#endif

