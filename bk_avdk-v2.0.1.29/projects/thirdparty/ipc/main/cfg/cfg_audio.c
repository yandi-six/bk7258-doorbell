/*!
*****************************************************************************

*****************************************************************************
*/

#include "cfg_audio.h"
#include "sdk_log.h"
SDK_NET_AUDIO_CFG runAudioCfg;

SDK_CFG_MAP audioMap[] = {
    {"mode",           &runAudioCfg.mode,           SDK_CFG_DATA_TYPE_S32, "2",    "rw", 0, 2, "0 disable; 1 input; 2 input&output"},
    {"type",           &runAudioCfg.type,           SDK_CFG_DATA_TYPE_S32, "2",    "rw", 0, 2, "0 a-law; 1 u-law; 2 PCM; 3-adpcm"},
    {"chans",          &runAudioCfg.chans,          SDK_CFG_DATA_TYPE_S32, "1",    "rw", 1, 2, "1, 2"},
    {"chans_opt", &runAudioCfg.chans_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"opt\":[1,2]}",  "rw", 0, MAX_STR_LEN_64,  NULL},
    {"sampleRate",     &runAudioCfg.sampleRate,     SDK_CFG_DATA_TYPE_S32, "8000", "rw", 8000, 48000, NULL},
    {"sampleRate_opt", &runAudioCfg.sampleRate_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"opt\":[8000,16000,48000]}",  "rw", 0, MAX_STR_LEN_64,  NULL},
    {"sampleBitWidth", &runAudioCfg.sampleBitWidth, SDK_CFG_DATA_TYPE_S32, "16",   "rw", 8, 32, NULL},
    {"sampleBitWidth_opt", &runAudioCfg.sampleBitWidth_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"opt\":[16]}",  "rw", 0, MAX_STR_LEN_64,  NULL},
    {"inputVolume",    &runAudioCfg.inputVolume,    SDK_CFG_DATA_TYPE_S32, "7",   "rw", 0, 12, NULL},
    {"inputVolume_opt", &runAudioCfg.inputVolume_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":100}",  "rw", 0, MAX_STR_LEN_64,  NULL},
    {"outputVolume",   &runAudioCfg.outputVolume,   SDK_CFG_DATA_TYPE_S32, "7",   "rw", 0, 12, NULL},
    {"outputVolume_opt", &runAudioCfg.outputVolume_opt,SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING,    "{\"min\":0,\"max\":100}",  "rw", 0, MAX_STR_LEN_64,  NULL},
    {NULL,},
};

void AudioCfgPrint()
{
    webcam_debug("********** Audio *********");
    CfgPrintMap(audioMap);
    webcam_debug("********** Audio *********");
}

int AudioCfgSave()
{
    int ret = CfgSave(AUDIO_CFG_FILE, "audio", audioMap);
    if (ret != 0) {
        webcam_error("CfgSave %s error.", AUDIO_CFG_FILE);
        return -1;
    }

    return 0;
}

int AudioCfgLoad()
{
    int ret = CfgLoad(AUDIO_CFG_FILE, "audio", audioMap);
    if (ret != 0) {
        webcam_error("CfgLoad %s error.", AUDIO_CFG_FILE);
        return -1;
    }

    return 0;
}
cJSON * AudioCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(AUDIO_CFG_FILE);
    if (data == NULL) {
        //ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        webcam_debug("load %s error, so to load default cfg param.\n", AUDIO_CFG_FILE);
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
int AudioCfgLoadDefValue()
{
    CfgLoadDefValue(audioMap);

    return 0;
}
int AudioJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(AUDIO_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", AUDIO_CFG_FILE);
        return -1;
    }

    free(out);
    AudioCfgLoad();
    return 0;
}

