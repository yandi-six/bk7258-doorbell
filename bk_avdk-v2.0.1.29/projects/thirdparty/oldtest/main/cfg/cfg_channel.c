/*!
*****************************************************************************

*****************************************************************************
*/

#include "cfg_channel.h"
#include "sdk_log.h"


SDK_NET_CHANNEL_CFG runChannelCfg;


SDK_CFG_MAP osdMap[]= {
        {"osdChannelName_enable",  &(runChannelCfg.channelInfo.osdChannelName.enable),  SDK_CFG_DATA_TYPE_S32,    "1",   "rw", 0, 1,               NULL},
        {"osdChannelName_text",    &(runChannelCfg.channelInfo.osdChannelName.text),    SDK_CFG_DATA_TYPE_STRING, "WebRTC Camera", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelName_x",       &(runChannelCfg.channelInfo.osdChannelName.x),       SDK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelName_x_opt",   &(runChannelCfg.channelInfo.osdChannelName.x_opt),SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":1280}",  "rw", 0, MAX_STR_LEN_128,            NULL},
        {"osdChannelName_y",       &(runChannelCfg.channelInfo.osdChannelName.y),       SDK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelName_y_opt",   &(runChannelCfg.channelInfo.osdChannelName.y_opt), SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":1080}",  "rw", 0, MAX_STR_LEN_128,            NULL},
        {"osdDatetime_enable",     &(runChannelCfg.channelInfo.osdDatetime.enable),     SDK_CFG_DATA_TYPE_S32, "0",   "rw", 0, 1,    NULL},
        {"osdDatetime_dateFormat", &(runChannelCfg.channelInfo.osdDatetime.dateFormat), SDK_CFG_DATA_TYPE_U8,  "0",   "rw", 1, 120,  "0:XXXX-XX-XX year mon date; 1:XX-XX-XXXX mon date year 2:XX-XX-XXXX date mon year"},
        {"osdDatetime_dateSprtr",  &(runChannelCfg.channelInfo.osdDatetime.dateSprtr),  SDK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 3,    "0 :, 1 -, 2 //, 3 ."},
        {"osdDatetime_timeFmt",    &(runChannelCfg.channelInfo.osdDatetime.timeFmt),    SDK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 1,    "0 - 24,1 - 12"},
        {"osdDatetime_x",          &(runChannelCfg.channelInfo.osdDatetime.x),          SDK_CFG_DATA_TYPE_FLOAT, "0.2", "rw", 0, 1.0, NULL},
        {"osdDatetime_x_opt",      &(runChannelCfg.channelInfo.osdDatetime.x_opt),	SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":1280}",  "rw", 0, MAX_STR_LEN_128,            NULL},
        {"osdDatetime_y",          &(runChannelCfg.channelInfo.osdDatetime.y),          SDK_CFG_DATA_TYPE_FLOAT, "0",  "rw", 0, 1.0, NULL},
        {"osdDatetime_y_opt",      &(runChannelCfg.channelInfo.osdDatetime.y_opt),	SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":1080}",  "rw", 0, MAX_STR_LEN_128,            NULL},
	{"osdDatetime_displayWeek", &(runChannelCfg.channelInfo.osdDatetime.displayWeek),    SDK_CFG_DATA_TYPE_U8,  "1",	"rw", 0, 1, "0 - off,1 - on"},

        {NULL,},
};

SDK_CFG_MAP shelterRectMap[] = {
        {"shelterRect_enable", 	   &(runChannelCfg.shelterRect.enable), SDK_CFG_DATA_TYPE_S32, 	"0", "rw", 0, 1,    NULL},
        {"shelterRect_x",          &(runChannelCfg.shelterRect.x),      SDK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_x_opt",      &(runChannelCfg.shelterRect.x_opt),SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":1280}",  "rw", 0, MAX_STR_LEN_128,            NULL},
        {"shelterRect_y",          &(runChannelCfg.shelterRect.y),      SDK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_y_opt",      &(runChannelCfg.shelterRect.y_opt),SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":1080}",  "rw", 0, MAX_STR_LEN_128,            NULL},
        {"shelterRect_width",      &(runChannelCfg.shelterRect.width),  SDK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_width_opt",  &(runChannelCfg.shelterRect.width_opt),SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":1280}",  "rw", 0, MAX_STR_LEN_128,            NULL},
        {"shelterRect_height",     &(runChannelCfg.shelterRect.height), SDK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_height_opt", &(runChannelCfg.shelterRect.height_opt),SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":1080}",  "rw", 0, MAX_STR_LEN_128,            NULL},
        {"shelterRect_color",      &(runChannelCfg.shelterRect.color),  SDK_CFG_DATA_TYPE_U32, 	"0", "rw", 0, 0x7FFFFFFF,  NULL},
        {NULL,},
};


void ChannelCfgPrint()
{
      CfgPrintMap(osdMap);
      CfgPrintMap(shelterRectMap);
}

int ChannelCfgSave()
{
    cJSON *root, *item;
    char *out;

    root = cJSON_CreateObject();//创建项目
    item = CfgDataToCjsonByMap(osdMap);
    cJSON_AddItemToObject(root, "osd", item);

    item = CfgDataToCjsonByMap(shelterRectMap);

    cJSON_AddItemToObject(root, "shelter", item);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(CHANNEL_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", CHANNEL_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int ChannelCfgLoadDefValue()
{

     CfgLoadDefValue(osdMap);
    
     CfgLoadDefValue(shelterRectMap);
    

    return 0;
}


int ChannelCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(CHANNEL_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_error("load %s error, so to load default cfg param.\n", CHANNEL_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    // get channel osd
    cJSON *osditem = NULL;
    osditem = cJSON_GetObjectItem(json, "osd");
    if(!osditem){
        webcam_error("get osd error\n");
        goto err1;
    }
    CfgCjsonToDataByMap(osdMap, osditem);

    cJSON *shelteritem = NULL;
    shelteritem = cJSON_GetObjectItem(json, "shelter");
    if(!shelteritem){
        webcam_error("get shelter error\n");
        goto err1;
    }

    CfgCjsonToDataByMap(shelterRectMap, shelteritem);
    

    cJSON_Delete(json);
    free(data);
    return 0;

err1:
    cJSON_Delete(json);
    free(data);
err:
    ChannelCfgLoadDefValue();
    ChannelCfgSave();
    return 0;

}
cJSON * ChannelCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(CHANNEL_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        webcam_debug("load %s error, so to load default cfg param.\n", CHANNEL_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
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
int ChannelJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(CHANNEL_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", CHANNEL_CFG_FILE);
        return -1;
    }

    free(out);
    ChannelCfgLoad();
    return 0;
}


