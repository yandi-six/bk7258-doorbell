/*!
*****************************************************************************

*****************************************************************************
*/


#include "cfg_linkage.h"
#include "sdk_log.h"
SDK_NET_LINKAGE_CFG runLinkageCfg;



SDK_CFG_MAP linkageMap[] = {
    {"enable",            &(runLinkageCfg.enable),        SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    {NULL,},

};


void LinkageCfgPrint()
{
   // printf("********** Pir *********\n");
    CfgPrintMap(linkageMap);
   // printf("********** Pir *********\n\n");
}

int LinkageCfgSave()
{
    int ret = CfgSave(LINKAGE_CFG_FILE, "linkage", linkageMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", LINKAGE_CFG_FILE);
        return -1;
    }

    return 0;
}

int LinkageCfgLoad()
{
    int ret = CfgLoad(LINKAGE_CFG_FILE, "linkage", linkageMap);
    if (ret != 0) {
        webcam_error("CfgLoad %s error.", LINKAGE_CFG_FILE);
        return -1;
    }

    return 0;
}

int LinkageCfgLoadDefValue()
{
    CfgLoadDefValue(linkageMap);

    return 0;
}
int LinkageLoadActionsDefValue(SDK_ACTION *action){
	SDK_ACTION *temp = action;
	int i = 0;
	for(i =0;i< MAX_ACTIONS;i++){
	   memset(temp,0,sizeof(SDK_ACTION));
	   temp++;
        }
	action->selected = 1;
        action->index = 0;
	snprintf(action->lable,MAX_STR_LEN_32,"%s","record");
        return 1;

}
cJSON* LinkageCfgLoadJson(){
    char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(LINKAGE_CFG_FILE);
    if (data == NULL) {
        webcam_debug("load %s error, so to load default cfg param.\n", LINKAGE_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //ŽÓÅäÖÃÎÄŒþœâÎöcjsonÊ§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    free(data);
    return json;

err:
    if(data){
	free(data);
    }
    return NULL;

}
int LinkageJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(LINKAGE_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", LINKAGE_CFG_FILE);
        return -1;
    }

    free(out);
    LinkageCfgLoad();
    return 0;
}
