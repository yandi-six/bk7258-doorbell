/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_AUDIO_H__
#define _SDK_CFG_AUDIO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"


/***********************************/
/***       audio                 ***/
/***********************************/


typedef struct {
    SDK_S32                 mode;           // 0 disable; 1 input; 2- input&output
    SDK_S32                 type;           // 0 a-law; 1 u-law; 2 pcm; 3-adpcm
    SDK_S32                 chans;          // 1
    SDK_S8                  chans_opt[MAX_STR_LEN_64];          // 1
    SDK_S32   		    sampleRate;     //8000
    SDK_S8                  sampleRate_opt[MAX_STR_LEN_64];  
    SDK_S32                 sampleBitWidth; //8, 16
    SDK_S8                  sampleBitWidth_opt[MAX_STR_LEN_64];  
    SDK_S32                 inputVolume;    // 100
    SDK_S8                  inputVolume_opt[MAX_STR_LEN_64]; 
    SDK_S32                 outputVolume;   // 100
    SDK_S8                  outputVolume_opt[MAX_STR_LEN_64];
} SDK_NET_AUDIO_CFG, *lPSDK_NET_AUDIO_CFG;

extern int AudioCfgSave();
extern int AudioCfgLoad();
extern void AudioCfgPrint();
extern int AudioCfgLoadDefValue();
extern cJSON * AudioCfgLoadJson();
extern int AudioJsonSaveCfg(cJSON *json);

#define AUDIO_CFG_FILE "audio_cfg.cjson"

extern SDK_NET_AUDIO_CFG runAudioCfg;
extern SDK_CFG_MAP audioMap[];

#ifdef __cplusplus
}
#endif
#endif

