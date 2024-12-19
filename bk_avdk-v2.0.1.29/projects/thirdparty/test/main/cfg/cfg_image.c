/*!
*****************************************************************************

*****************************************************************************
*/

#include "cfg_image.h"
#include "sdk_log.h"
SDK_NET_IMAGE_CFG runImageCfg;


SDK_CFG_MAP imageMap[] = {
    {"sceneMode",     &runImageCfg.sceneMode,     SDK_CFG_DATA_TYPE_U8, "0", "rw", 0, 2, "0 auto 1 indoor 2 outdoor"},
    {"imageStyle",    &runImageCfg.imageStyle,    SDK_CFG_DATA_TYPE_U8, "0", "rw", 0, 2, "0 nomal 1 lightness 2 bright"},
    {"wbMode",        &runImageCfg.wbMode,        SDK_CFG_DATA_TYPE_U8, "0", "rw", 0, 2,  "0 auto 1 indoor 2 outdoor"},

    {"irCutControlMode",  &runImageCfg.irCutControlMode,  SDK_CFG_DATA_TYPE_U8, "0", "rw", 0, 1, "0 hardware,  1 software"},
    {"irCutMode",         &runImageCfg.irCutMode,         SDK_CFG_DATA_TYPE_U8, "0", "rw", 0, 2, "0 auto, 1 day, 2 night"},
    {"enabledWDR",        &runImageCfg.enabledWDR,        SDK_CFG_DATA_TYPE_U8, "1", "rw", 0, 1, NULL},
    {"strengthWDR",       &runImageCfg.strengthWDR,       SDK_CFG_DATA_TYPE_U8, "3", "rw", 1, 5, NULL},
    {"enableDenoise3d",   &runImageCfg.enableDenoise3d,   SDK_CFG_DATA_TYPE_U8, "1", "rw", 0, 1, NULL},
    {"strengthDenoise3d", &runImageCfg.strengthDenoise3d, SDK_CFG_DATA_TYPE_U8, "1", "rw", 0, 100, NULL},

    {"lowlightMode",    &runImageCfg.lowlightMode,    SDK_CFG_DATA_TYPE_U8,  "3", "rw", 0, 3, "0 close, 1 only night, 2 day-night, 3 auto"},
    {"exposureMode",    &runImageCfg.exposureMode,    SDK_CFG_DATA_TYPE_U8,  "0", "rw", 0, 2, "0 - auto 1 - bright, 2 - dark"},
    {"dcIrisEnable",    &runImageCfg.dcIrisEnable,    SDK_CFG_DATA_TYPE_U8,  "1", "rw", 0, 1, NULL},
    {"antiFlickerFreq", &runImageCfg.antiFlickerFreq, SDK_CFG_DATA_TYPE_U8,  "50", "rw", 50, 60, "50HZ 60HZ"},
    {"backLightEnable", &runImageCfg.backLightEnable, SDK_CFG_DATA_TYPE_U8,  "1", "rw", 0, 1, NULL},
    {"backLightLevel",  &runImageCfg.backLightLevel,  SDK_CFG_DATA_TYPE_S32, "3", "rw", 0, 100, NULL},

    {"brightness",  	&runImageCfg.brightness, SDK_CFG_DATA_TYPE_S32, "128", "rw", 0, 256, NULL},
    {"brightness_default",  &runImageCfg.brightness_default, SDK_CFG_DATA_TYPE_S32, "128", "rw", 0, 256, NULL},
    {"brightness_opt",  &runImageCfg.brightness_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":256}",  "rw", 0, MAX_STR_LEN_128,  NULL},
    {"saturation", 	&runImageCfg.saturation, SDK_CFG_DATA_TYPE_S32, "128", "rw", 0, 256, NULL},
    {"saturation_default", &runImageCfg.saturation_default, SDK_CFG_DATA_TYPE_S32, "128", "rw", 0, 256, NULL},
    {"saturation_opt",  &runImageCfg.saturation_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":256}",  "rw", 0, MAX_STR_LEN_128,  NULL},
    {"contrast",    	&runImageCfg.contrast,   SDK_CFG_DATA_TYPE_S32, "128", "rw", 0, 256, NULL},
    {"contrast_default", &runImageCfg.contrast_default,   SDK_CFG_DATA_TYPE_S32, "128", "rw", 0, 256, NULL},
    {"contrast_opt",    &runImageCfg.contrast_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":256}",  "rw", 0, MAX_STR_LEN_128,  NULL},
    {"sharpness",   &runImageCfg.sharpness,  SDK_CFG_DATA_TYPE_S32, "128", "rw", 0, 256, NULL},
    {"sharpness_default",   &runImageCfg.sharpness_default,  SDK_CFG_DATA_TYPE_S32, "128", "rw", 0, 256, NULL},
    {"sharpness_opt",   &runImageCfg.sharpness_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":256}",  "rw", 0, MAX_STR_LEN_128,  NULL},
    {"hue",         	&runImageCfg.hue,        SDK_CFG_DATA_TYPE_S32, "128",   "rw", 0, 256, NULL},
    {"hue_default",     &runImageCfg.hue_default,        SDK_CFG_DATA_TYPE_S32, "128",   "rw", 0, 256, NULL},
    {"hue_opt",   	&runImageCfg.hue_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":256}",  "rw", 0, MAX_STR_LEN_128,  NULL},
    {"horizontal",   &runImageCfg.horizontal,   SDK_CFG_DATA_TYPE_U8, "0", "rw", 0, 1, NULL},
    {"vertical",     &runImageCfg.vertical, SDK_CFG_DATA_TYPE_U8, "0", "rw", 0, 1, NULL},

    {NULL,},

};

void ImageCfgPrint()
{
   // printf("********** Image *********\n");
    CfgPrintMap(imageMap);
   // printf("********** Image *********\n\n");
}

int ImageCfgSave()
{
    int ret = CfgSave(IMAGE_CFG_FILE, "image", imageMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", IMAGE_CFG_FILE);
        return -1;
    }

    return 0;
}

int ImageCfgLoad()
{
    int ret = CfgLoad(IMAGE_CFG_FILE, "image", imageMap);
    if (ret != 0) {
        webcam_error("CfgLoad %s error.", IMAGE_CFG_FILE);
        return -1;
    }

    return 0;
}
cJSON * ImageCfgLoadJsonFromFile(){

   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(IMAGE_CFG_FILE);
    if (data == NULL) {
        //ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        webcam_debug("load %s error, so to load default cfg param.\n", IMAGE_CFG_FILE);
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
int ImageCfgLoadDefValue()
{
    CfgLoadDefValue(imageMap);
    return 0;
}

cJSON * ImageCfgLoadJson(){
   
    cJSON *root;
    root = CfgDataToCjsonByMap(imageMap);
    if(root == NULL)
    {
        webcam_error("imageMap to json error.");
        return NULL;
    }
    cJSON *json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "image", root);
    return json;

}
int ImageJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);
    int ret = CfgWriteToFile(IMAGE_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", IMAGE_CFG_FILE);
        return -1;
    }

    free(out);
    ImageCfgLoad();
    return 0;
}
