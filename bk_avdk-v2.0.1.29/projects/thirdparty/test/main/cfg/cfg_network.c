/*!
*****************************************************************************

*****************************************************************************
*/

#if 1
#include "cfg_network.h"
#include <sys/time.h>
static void random_produce_mac(char *randomMac);

SDK_NET_NETWORK_CFG runNetworkCfg;


SDK_CFG_MAP lanMap[] = {
    {"enable",   &(runNetworkCfg.lan.enable),     SDK_CFG_DATA_TYPE_S32,    "1",                 "rw", 0, 1,             NULL},
    {"netName",   &(runNetworkCfg.lan.netName),   SDK_CFG_DATA_TYPE_STRING, "eth0",              "rw", 1, MAX_STR_LEN_64, NULL},
    {"ipVersion", &(runNetworkCfg.lan.ipVersion), SDK_CFG_DATA_TYPE_S32,    "0",                 "rw", 0, 1,              "0 v4, 1 v6"},
    {"mac",       &(runNetworkCfg.lan.mac),       SDK_CFG_DATA_TYPE_STRING, "28:E3:83:F6:40:12", "rw", 1, MAX_STR_LEN_20, NULL},
    {"dhcpEnable",    &(runNetworkCfg.lan.dhcpIp),    SDK_CFG_DATA_TYPE_S32, "1",                "rw", 0, 1,              NULL},
    {"upnpEnable",&(runNetworkCfg.lan.upnpEnable),SDK_CFG_DATA_TYPE_S32,    "0",                 "rw", 0, 1,              NULL},
    {"ip",        &(runNetworkCfg.lan.ip),        SDK_CFG_DATA_TYPE_STRING, "192.168.1.250",     "rw", 1, MAX_STR_LEN_16, NULL},
    {"netmask",   &(runNetworkCfg.lan.netmask),   SDK_CFG_DATA_TYPE_STRING, "255.255.255.0",     "rw", 1, MAX_STR_LEN_16, NULL},
    {"gateway",   &(runNetworkCfg.lan.gateway),   SDK_CFG_DATA_TYPE_STRING, "192.168.1.1",      "rw", 1, MAX_STR_LEN_16, NULL},
    {"multicast", &(runNetworkCfg.lan.multicast), SDK_CFG_DATA_TYPE_STRING, "192.168.1.255",    "rw", 1, MAX_STR_LEN_16, NULL},
    {"dhcpDns",   &(runNetworkCfg.lan.dhcpDns),   SDK_CFG_DATA_TYPE_S32,    "1",                 "rw", 0, 1,              NULL},
    {"dns1",      &(runNetworkCfg.lan.dns1),      SDK_CFG_DATA_TYPE_STRING, "8.8.8.8",   "rw", 1, MAX_STR_LEN_16, NULL},
    {"dns2",      &(runNetworkCfg.lan.dns2),      SDK_CFG_DATA_TYPE_STRING, "8.8.4.4",           "rw", 1, MAX_STR_LEN_16, NULL},
    {NULL,},
};

SDK_CFG_MAP wifiMap[] = {
    {"enable",    &(runNetworkCfg.wifi.enable),    SDK_CFG_DATA_TYPE_S32,     "0",                "rw", 0, 1,             NULL},
    {"netName",   &(runNetworkCfg.wifi.netName),   SDK_CFG_DATA_TYPE_STRING, "wlan0",               "rw", 1, MAX_STR_LEN_64, NULL},
    {"ipVersion", &(runNetworkCfg.wifi.ipVersion), SDK_CFG_DATA_TYPE_S32,    "0",                 "rw", 0, 1,              "0 v4, 1 v6"},
    {"mac",       &(runNetworkCfg.wifi.mac),       SDK_CFG_DATA_TYPE_STRING, "28:E3:83:F6:40:13", "rw", 1, MAX_STR_LEN_20, NULL},
    {"dhcpEnable",&(runNetworkCfg.wifi.dhcpIp),SDK_CFG_DATA_TYPE_S32,    	"1",                 "rw", 0, 1,              NULL},
    {"upnpEnable",&(runNetworkCfg.wifi.upnpEnable),SDK_CFG_DATA_TYPE_S32,    "0",                 "rw", 0, 1,              NULL},
    {"ip",        &(runNetworkCfg.wifi.ip),        SDK_CFG_DATA_TYPE_STRING, "192.168.1.26",     "rw", 1, MAX_STR_LEN_16, NULL},
    {"netmask",   &(runNetworkCfg.wifi.netmask),   SDK_CFG_DATA_TYPE_STRING, "255.255.255.0",     "rw", 1, MAX_STR_LEN_16, NULL},
    {"gateway",   &(runNetworkCfg.wifi.gateway),   SDK_CFG_DATA_TYPE_STRING, "192.168.1.1",      "rw", 1, MAX_STR_LEN_16, NULL},
    {"multicast", &(runNetworkCfg.wifi.multicast), SDK_CFG_DATA_TYPE_STRING, "5.255.255.255",    "rw", 1, MAX_STR_LEN_16, NULL},
    {"dhcpDns",   &(runNetworkCfg.wifi.dhcpDns),   SDK_CFG_DATA_TYPE_S32,    "1",                 "rw", 0, 1,              NULL},
    {"dns1",      &(runNetworkCfg.wifi.dns1),      SDK_CFG_DATA_TYPE_STRING, "8.8.8.8",   "rw", 1, MAX_STR_LEN_16, NULL},
    {"dns2",      &(runNetworkCfg.wifi.dns2),      SDK_CFG_DATA_TYPE_STRING, "8.8.4.4",           "rw", 1, MAX_STR_LEN_16, NULL},
    {NULL,},
};

SDK_CFG_MAP wirelessMap[] = {
    {"enable", &runNetworkCfg.wireless.enable, SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1, NULL},
    {"isConfig", &runNetworkCfg.wireless.isConfig, SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1, NULL},
    {"mode", &runNetworkCfg.wireless.mode, SDK_CFG_DATA_TYPE_S32, "1", "rw", 0, 1, NULL},
    {"ssid", &runNetworkCfg.wireless.essid, SDK_CFG_DATA_TYPE_STRING, "", "rw", 1, MAX_STR_LEN_32, NULL},
    {"passd", &runNetworkCfg.wireless.passd, SDK_CFG_DATA_TYPE_STRING, "", "rw", 1, MAX_STR_LEN_32, NULL},
    {"api_url", &runNetworkCfg.wireless.api_url, SDK_CFG_DATA_TYPE_STRING, "", "rw", 1, MAX_STR_LEN_64, NULL},//+++
    {"apBssId", &runNetworkCfg.wireless.apBssId, SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    {"apPsk", &runNetworkCfg.wireless.apPsk, SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    {
        NULL,
    }

};


    // use default ip address for dhcp failed
    static void
    reset_net_default_dhcp(void)
{
    int i = 0;
    if(runNetworkCfg.lan.dhcpIp == 1)
    {
        for(i = 0;  lanMap[i].stringName != NULL;i++)
        {
            if(strcmp("ip",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.ip,lanMap[i].defaultValue);
            else if(strcmp("netmask",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.netmask,lanMap[i].defaultValue);
            else if(strcmp("gateway",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.gateway,lanMap[i].defaultValue);
            else if(strcmp("dns1",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.dns1,lanMap[i].defaultValue);
            else if(strcmp("dns2",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.dns2,lanMap[i].defaultValue);
        }
    }
    if(runNetworkCfg.wifi.dhcpIp == 1)
    {
        for(i = 0;  wifiMap[i].stringName != NULL;i++)
        {
            if(strcmp("ip",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.ip,wifiMap[i].defaultValue);
            else if(strcmp("netmask",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.netmask,wifiMap[i].defaultValue);
            else if(strcmp("gateway",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.gateway,wifiMap[i].defaultValue);
            else if(strcmp("dns1",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.dns1,wifiMap[i].defaultValue);
            else if(strcmp("dns2",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.dns2,wifiMap[i].defaultValue);
        }
    }


}
void NetworkCfgPrint()
{
    os_printf("*************** Network **************\n");

    os_printf("lan:\n");
    CfgPrintMap(lanMap);
    os_printf("\n");

    os_printf("wifi:\n");
    CfgPrintMap(wifiMap);
    os_printf("\n");
    os_printf("wireless:\n");
    CfgPrintMap(wirelessMap);

    os_printf("*************** Network **************\n\n");
}

int NetworkCfgDelete(){
    int ret = CfgWriteDelete(NETWORK_CFG_FILE);
    if (ret != 0) {
        os_printf("CfgWriteDelete %s error.\n", NETWORK_CFG_FILE);
        return ret;
    }
    return 0;
}

int NetworkCfgSave()
{
    cJSON *root;
    // cJSON *wifiLink = NULL;
    char *out;

    root = cJSON_CreateObject();//\B4\B4\BD\A8\CF\EEĿ

    CfgAddCjson(root, "lan", lanMap);
    CfgAddCjson(root, "wifi", wifiMap);
    CfgAddCjson(root, "wireless", wirelessMap);


    out = cJSON_Print(root);

    int ret = CfgWriteToFile(NETWORK_CFG_FILE, out);
    if (ret != 0) {
        os_printf("CfgWriteToFile %s error.\n", NETWORK_CFG_FILE);
        return -1;
    }

    os_free(out);
    cJSON_Delete(root);

    return 0;
}

int NetworkCfgLoadDefValue()
{
    CfgLoadDefValue(lanMap);
    CfgLoadDefValue(wifiMap);
    CfgLoadDefValue(wirelessMap);
    return 0;
}


int NetworkCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(NETWORK_CFG_FILE);
    if (data == NULL) {
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\B6\C1ȡʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        os_printf("load %s error, so to load default cfg param.\n", NETWORK_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\BD\E2\CE\F6cjsonʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        os_free(data);
        goto err;
    }


    CfgParseCjson(json, "lan", lanMap);
    CfgParseCjson(json, "wifi", wifiMap);
    CfgParseCjson(json, "wireless", wirelessMap);

    if(strcmp("28:E3:83:F6:40:12", runNetworkCfg.lan.mac) == 0){
    	    random_produce_mac(runNetworkCfg.lan.mac);
    }
    cJSON_Delete(json);

    os_free(data);
    return 0;

err:
    NetworkCfgLoadDefValue();
    NetworkCfgSave();
    return 0;
}

#endif
cJSON * NetworkCfgLoadJson(){

    char *data = NULL;
    data = CfgReadFromFile(NETWORK_CFG_FILE);
    if (data == NULL) {
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\B6\C1ȡʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        //webcam_debug("load %s error, so to load default cfg param.\n", NETWORK_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\BD\E2\CE\F6cjsonʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        //webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        os_free(data);
        goto err;
    }

     os_free(data);
     return json;
err:
    if(data!= NULL){
	os_free(data);
    }
    return NULL;
}

char *NetworkCfgGetJosnString(int type)
{

    char *out;
    cJSON *root;
    switch(type)
    {
        case 0:
        root = CfgDataToCjsonByMap(lanMap);
        break;
        case 1:
        root = CfgDataToCjsonByMap(wifiMap);
        break;
        default:
            return NULL;
        break;
    }

    if(root == NULL){
       // webcam_error("Network Map to json error.");
        return NULL;
    }
    out = cJSON_Print(root);
    //webcam_debug("Network type:%d,json:%s\n",type,out);
    cJSON_Delete(root);
    return out;

}
cJSON *NetworkCfgGetLanJosn(){
     return CfgDataToCjsonByMap(lanMap);  
}
cJSON *NetworkCfgGetWifiJosn(){
     return CfgDataToCjsonByMap(wifiMap);
}

//生成一个随机的 MAC 地址
static void random_produce_mac(char *randomMac)
{
#if CONFIG_TRNG_SUPPORT
	char HEXCHAR[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	char fmtMACAddr[]={'B','E',':','2','7',':','0','0',':','0','0',':','0','0',':','0','0',0};
	int i = 0, n = 0;
	unsigned long int seed = 0;
	seed = (unsigned long int)HEXCHAR+(unsigned long int)fmtMACAddr+(unsigned long int)&i +(unsigned long int)&n;
	//webcam_debug("seed: %lu\n", seed);
	srand(seed);
	for(i = 6; i < strlen(fmtMACAddr); i++)
	{
		n = (bk_rand()% 16);
		if(((i+1)% 3) !=0)
		{
			fmtMACAddr[i] = HEXCHAR[n];
		}
	}

	strcpy(randomMac, fmtMACAddr);
	//webcam_debug("Random mac: %s\n",randomMac);
#endif
}

