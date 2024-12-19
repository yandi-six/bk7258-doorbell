/*!
*****************************************************************************

*****************************************************************************
*/
#include "cfg_video.h"
#include "cfg_channel.h"
#include "sdk_log.h"


SDK_NET_VIDEO_CFG runVideoCfg;

SDK_CFG_MAP videoMap[MAX_VENC_STREAM_NUM][MAX_VENC_ITEM_NUM] = {
    {
        {"id",                 &(runVideoCfg.vencStream[0].id),                      SDK_CFG_DATA_TYPE_S32, "0",       "rw", 0, 3, "0-3"},
        {"enable",             &(runVideoCfg.vencStream[0].enable),                  SDK_CFG_DATA_TYPE_S32, "1",       "rw", 0, 1, NULL},
        {"avStream",		   &(runVideoCfg.vencStream[0].avStream),				 SDK_CFG_DATA_TYPE_S32, "0", 	  "rw", 0, 2, "0: send all 1:just send video 2: not send"},
        {"format_encodeType",  &(runVideoCfg.vencStream[0].streamFormat.encodeType), SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 2, "0: none, 1: H.264, 2: MJPEG"},
#ifdef GK7101
        {"format_width",       &(runVideoCfg.vencStream[0].streamFormat.width),      SDK_CFG_DATA_TYPE_U16, "1920",    "rw", 0, 1920, "{\"opt\" : [ \"1920x1080\", \"1280x960\", \"1280x720\", \"1024x768\", \"1024x576\", \"960x576\", \"960x480\", \"720x576\", \"720x480\" ]}"},
        {"format_height",      &(runVideoCfg.vencStream[0].streamFormat.height),     SDK_CFG_DATA_TYPE_U16, "1080",    "rw", 0, 1080, NULL},
#else
        #if (defined(CUS_SENSOR_AR0130) || (CUS_SENSOR_SC1035))
        {"format_width",       &(runVideoCfg.vencStream[0].streamFormat.width),      SDK_CFG_DATA_TYPE_U16, "1280",    "rw", 0, 1280, "{\"opt\" : [ \"1280x960\", \"1280x720\", \"1024x576\", \"640x380\" ]}"},
        {"format_height",      &(runVideoCfg.vencStream[0].streamFormat.height),     SDK_CFG_DATA_TYPE_U16, "720",     "rw", 0,  960, NULL},
        #else
        {"format_width",       &(runVideoCfg.vencStream[0].streamFormat.width),      SDK_CFG_DATA_TYPE_U16, "1280",    "rw", 0, 1280, "{\"opt\" : [ \"1280x720\", \"1024x576\", \"960x576\", \"960x480\", \"720x576\", \"720x480\",  \"640x480\" ]}"},
        {"format_height",      &(runVideoCfg.vencStream[0].streamFormat.height),     SDK_CFG_DATA_TYPE_U16, "720",    "rw", 0,  720, NULL},
        #endif
#endif

        {"format_fps",         &(runVideoCfg.vencStream[0].streamFormat.fps),        SDK_CFG_DATA_TYPE_U32, "25",      "rw", 1, 25,   NULL},
        {"format_keepAspRat",  &(runVideoCfg.vencStream[0].streamFormat.keepAspRat), SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "1: means keep aspect ration, 0: means do not keep."},
        {"format_streamName",  &(runVideoCfg.vencStream[0].streamFormat.streamName), SDK_CFG_DATA_TYPE_STRING,  "CH-0", "rw", 0, MAX_STR_LEN_128,  NULL},
        {"h264_gopM",          &(runVideoCfg.vencStream[0].h264Conf.gopM),           SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 120,  NULL},
        {"h264_gopN",          &(runVideoCfg.vencStream[0].h264Conf.gopN),           SDK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 50,  NULL},
        {"h264_idrInterval",   &(runVideoCfg.vencStream[0].h264Conf.idrInterval),    SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 30,  NULL},
        {"h264_gopModel",      &(runVideoCfg.vencStream[0].h264Conf.gopModel),       SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 5,  NULL},
        {"h264_profile",       &(runVideoCfg.vencStream[0].h264Conf.profile),        SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 1,  "0: main, 1: baseline"},
        {"h264_brcMode",       &(runVideoCfg.vencStream[0].h264Conf.brcMode),        SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 3,  "0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality"},
        #ifdef GK7101
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[0].h264Conf.cbrAvgBps),      SDK_CFG_DATA_TYPE_U32, "2000", "rw", 0, 8096,   NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[0].h264Conf.vbrMinbps),      SDK_CFG_DATA_TYPE_U32, "1000", "rw", 0, 100,    NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[0].h264Conf.vbrMaxbps),      SDK_CFG_DATA_TYPE_U32, "2000", "rw", 0, 8096,   NULL},
        #else
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[0].h264Conf.cbrAvgBps),      SDK_CFG_DATA_TYPE_U32, "2000", "rw", 0, 4096,   NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[0].h264Conf.vbrMinbps),      SDK_CFG_DATA_TYPE_U32, "1000", "rw", 0, 50,     NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[0].h264Conf.vbrMaxbps),      SDK_CFG_DATA_TYPE_U32, "2000", "rw", 0, 4096,   NULL},
        #endif
        {"h264_quality",       &(runVideoCfg.vencStream[0].h264Conf.quality),        SDK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"h264_qcon",          &(runVideoCfg.vencStream[0].h264Conf.qcon),           SDK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"mjpeg_chromaFormat", &(runVideoCfg.vencStream[0].mjpegConf.chromaFormat),  SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "0: YUV 422, 1: YUV 420."},
        {"mjpeg_quality",      &(runVideoCfg.vencStream[0].mjpegConf.quality),       SDK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 100,  "1 ~ 100, 100 is best quality"},
        {NULL,},

    },
    {
        {"id",                 &(runVideoCfg.vencStream[1].id),                      SDK_CFG_DATA_TYPE_S32, "1",       "rw", 0, 3, "0-3"},
        {"enable",             &(runVideoCfg.vencStream[1].enable),                  SDK_CFG_DATA_TYPE_S32, "1",       "rw", 0, 1, NULL},
		{"avStream",		   &(runVideoCfg.vencStream[1].avStream),				 SDK_CFG_DATA_TYPE_S32, "0", 	  "rw", 0, 2, "0: send all 1:just send video 2: not send"},
        {"format_encodeType",  &(runVideoCfg.vencStream[1].streamFormat.encodeType), SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 2, "0: none, 1: H.264, 2: MJPEG"},
#ifdef GK7101
        {"format_width",       &(runVideoCfg.vencStream[1].streamFormat.width),      SDK_CFG_DATA_TYPE_U16, "640",     "rw", 0, 640, "{\"opt\" : [ \"640x480\", \"640x360\", \"352x288\", \"352x240\" ]}"},
        {"format_height",      &(runVideoCfg.vencStream[1].streamFormat.height),     SDK_CFG_DATA_TYPE_U16, "480",     "rw", 0, 480, NULL},
#else
		{"format_width",	   &(runVideoCfg.vencStream[1].streamFormat.width), 	 SDK_CFG_DATA_TYPE_U16, "640",	  "rw", 0, 640, "{\"opt\" : [ \"640x360\", \"352x288\", \"352x240\" ]}"},
		{"format_height",	   &(runVideoCfg.vencStream[1].streamFormat.height),	 SDK_CFG_DATA_TYPE_U16, "360",	  "rw", 0, 360, NULL},
#endif
        {"format_fps",         &(runVideoCfg.vencStream[1].streamFormat.fps),        SDK_CFG_DATA_TYPE_U32, "8",       "rw", 1, 30,  NULL},
        {"format_keepAspRat",  &(runVideoCfg.vencStream[1].streamFormat.keepAspRat), SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "1: means keep aspect ration, 0: means do not keep."},
        {"format_streamName",  &(runVideoCfg.vencStream[1].streamFormat.streamName), SDK_CFG_DATA_TYPE_STRING,  "CH-1", "rw", 0, MAX_STR_LEN_128,  NULL},

        {"h264_gopM",          &(runVideoCfg.vencStream[1].h264Conf.gopM),           SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 120,  NULL},
        {"h264_gopN",          &(runVideoCfg.vencStream[1].h264Conf.gopN),           SDK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 50,  NULL},
        {"h264_idrInterval",   &(runVideoCfg.vencStream[1].h264Conf.idrInterval),    SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 30,  NULL},
        {"h264_gopModel",      &(runVideoCfg.vencStream[1].h264Conf.gopModel),       SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 5,  NULL},
        {"h264_profile",       &(runVideoCfg.vencStream[1].h264Conf.profile),        SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 1,  "0: main, 1: baseline"},
        {"h264_brcMode",       &(runVideoCfg.vencStream[1].h264Conf.brcMode),        SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 3,  "0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality"},
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[1].h264Conf.cbrAvgBps),      SDK_CFG_DATA_TYPE_U32, "200",    "rw", 0, 3000,  NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[1].h264Conf.vbrMinbps),      SDK_CFG_DATA_TYPE_U32, "100",    "rw", 0,  200,  NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[1].h264Conf.vbrMaxbps),      SDK_CFG_DATA_TYPE_U32, "300",    "rw", 0, 3000,  NULL},
        {"h264_quality",       &(runVideoCfg.vencStream[1].h264Conf.quality),        SDK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"h264_qcon",          &(runVideoCfg.vencStream[1].h264Conf.qcon),           SDK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"mjpeg_chromaFormat", &(runVideoCfg.vencStream[1].mjpegConf.chromaFormat),  SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "0: YUV 422, 1: YUV 420."},
        {"mjpeg_quality",      &(runVideoCfg.vencStream[1].mjpegConf.quality),       SDK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 100,  "1 ~ 100, 100 is best quality"},
        {NULL,},
    },
    {
        {"id",                 &(runVideoCfg.vencStream[2].id),                      SDK_CFG_DATA_TYPE_S32, "2",       "rw", 0, 3, "0-3"},
        {"enable",             &(runVideoCfg.vencStream[2].enable),                  SDK_CFG_DATA_TYPE_S32, "1",       "rw", 0, 1, NULL},
	{"avStream",		   &(runVideoCfg.vencStream[2].avStream),				 SDK_CFG_DATA_TYPE_S32, "0", 	  "rw", 0, 2, "0: send all 1:just send video 2: not send"},
        {"format_encodeType",  &(runVideoCfg.vencStream[2].streamFormat.encodeType), SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 2, "0: none, 1: H.264, 2: MJPEG"},
#if (defined(CUS_SENSOR_AR0130) || defined(CUS_SENSOR_SC1035))
        {"format_width",       &(runVideoCfg.vencStream[2].streamFormat.width),      SDK_CFG_DATA_TYPE_U16, "320",       "rw", 0, 320, "{\"opt\" : [ \"320x220\", \"320x180\" ]}"},
        {"format_height",      &(runVideoCfg.vencStream[2].streamFormat.height),     SDK_CFG_DATA_TYPE_U16, "220",       "rw", 0, 240, NULL},
#else
        {"format_width",       &(runVideoCfg.vencStream[2].streamFormat.width),      SDK_CFG_DATA_TYPE_U16, "320",       "rw", 0, 320, "{\"opt\" : [ \"320x240\", \"320x180\" ]}"},
        {"format_height",      &(runVideoCfg.vencStream[2].streamFormat.height),     SDK_CFG_DATA_TYPE_U16, "240",       "rw", 0, 240, NULL},
#endif
        {"format_fps",         &(runVideoCfg.vencStream[2].streamFormat.fps),        SDK_CFG_DATA_TYPE_U32, "25",       "rw", 1, 30,   NULL},
        {"format_keepAspRat",  &(runVideoCfg.vencStream[2].streamFormat.keepAspRat), SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "1: means keep aspect ration, 0: means do not keep."},
        {"format_streamName",  &(runVideoCfg.vencStream[2].streamFormat.streamName), SDK_CFG_DATA_TYPE_STRING,  "CH-2", "rw", 0, MAX_STR_LEN_128,  NULL},

        {"h264_gopM",          &(runVideoCfg.vencStream[2].h264Conf.gopM),           SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 120,  NULL},
        {"h264_gopN",          &(runVideoCfg.vencStream[2].h264Conf.gopN),           SDK_CFG_DATA_TYPE_U8,  "25",      "rw", 1, 50,  NULL},
        {"h264_idrInterval",   &(runVideoCfg.vencStream[2].h264Conf.idrInterval),    SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 30,  NULL},
        {"h264_gopModel",      &(runVideoCfg.vencStream[2].h264Conf.gopModel),       SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 5,  NULL},
        {"h264_profile",       &(runVideoCfg.vencStream[2].h264Conf.profile),        SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 1,  "0: main, 1: baseline"},
        {"h264_brcMode",       &(runVideoCfg.vencStream[2].h264Conf.brcMode),        SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 3,  "0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality"},
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[2].h264Conf.cbrAvgBps),      SDK_CFG_DATA_TYPE_U32, "128",    "rw", 0, 400,  NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[2].h264Conf.vbrMinbps),      SDK_CFG_DATA_TYPE_U32, "88",     "rw", 0, 10,  NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[2].h264Conf.vbrMaxbps),      SDK_CFG_DATA_TYPE_U32, "128",    "rw", 0, 400,  NULL},
        {"h264_quality",       &(runVideoCfg.vencStream[2].h264Conf.quality),        SDK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"h264_qcon",          &(runVideoCfg.vencStream[2].h264Conf.qcon),           SDK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"mjpeg_chromaFormat", &(runVideoCfg.vencStream[2].mjpegConf.chromaFormat),  SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "0: YUV 422, 1: YUV 420."},
        {"mjpeg_quality",      &(runVideoCfg.vencStream[2].mjpegConf.quality),       SDK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 100,  "1 ~ 100, 100 is best quality"},
        {NULL,},
    },
    {
        {"id",                 &(runVideoCfg.vencStream[3].id),                      SDK_CFG_DATA_TYPE_S32, "3",       "rw", 0, 3, "0-3"},
        {"enable",             &(runVideoCfg.vencStream[3].enable),                  SDK_CFG_DATA_TYPE_S32, "0",       "rw", 0, 1, NULL},
	{"avStream",	       &(runVideoCfg.vencStream[3].avStream),		     SDK_CFG_DATA_TYPE_S32, "0", 	  "rw", 0, 2, "0: send all 1:just send video 2: not send"},
        {"format_encodeType",  &(runVideoCfg.vencStream[3].streamFormat.encodeType), SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 2, "0: none, 1: H.264, 2: MJPEG"},
        {"format_width",       &(runVideoCfg.vencStream[3].streamFormat.width),      SDK_CFG_DATA_TYPE_U16, "0",       "rw", 0, 160, "{\"opt\" : [ \"160x90\", \"160x120\"]}"},
        {"format_height",      &(runVideoCfg.vencStream[3].streamFormat.height),     SDK_CFG_DATA_TYPE_U16, "0",       "rw", 0, 90, NULL},
        {"format_fps",         &(runVideoCfg.vencStream[3].streamFormat.fps),        SDK_CFG_DATA_TYPE_U32, "0",       "rw", 1, 30,   NULL},
        {"format_keepAspRat",  &(runVideoCfg.vencStream[3].streamFormat.keepAspRat), SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "1: means keep aspect ration, 0: means do not keep."},
        {"format_streamName",  &(runVideoCfg.vencStream[3].streamFormat.streamName), SDK_CFG_DATA_TYPE_STRING,  "CH-3", "rw", 0, MAX_STR_LEN_128,  NULL},

        {"h264_gopM",          &(runVideoCfg.vencStream[3].h264Conf.gopM),           SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 120,  NULL},
        {"h264_gopN",          &(runVideoCfg.vencStream[3].h264Conf.gopN),           SDK_CFG_DATA_TYPE_U8,  "30",      "rw", 1, 50,  NULL},
        {"h264_idrInterval",   &(runVideoCfg.vencStream[3].h264Conf.idrInterval),    SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 30,  NULL},
        {"h264_gopModel",      &(runVideoCfg.vencStream[3].h264Conf.gopModel),       SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 5,  NULL},
        {"h264_profile",       &(runVideoCfg.vencStream[3].h264Conf.profile),        SDK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 1,  "0: main, 1: baseline"},
        {"h264_brcMode",       &(runVideoCfg.vencStream[3].h264Conf.brcMode),        SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 3,  "0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality"},
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[3].h264Conf.cbrAvgBps),      SDK_CFG_DATA_TYPE_U32, "100",     "rw", 0, 100,  NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[3].h264Conf.vbrMinbps),      SDK_CFG_DATA_TYPE_U32, "5",       "rw", 0, 5,  NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[3].h264Conf.vbrMaxbps),      SDK_CFG_DATA_TYPE_U32, "100",     "rw", 0, 100,  NULL},
        {"h264_quality",       &(runVideoCfg.vencStream[3].h264Conf.quality),        SDK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"h264_qcon",          &(runVideoCfg.vencStream[3].h264Conf.qcon),           SDK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"mjpeg_chromaFormat", &(runVideoCfg.vencStream[3].mjpegConf.chromaFormat),  SDK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "0: YUV 422, 1: YUV 420."},
        {"mjpeg_quality",      &(runVideoCfg.vencStream[3].mjpegConf.quality),       SDK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 100,  "1 ~ 100, 100 is best quality"},
        {NULL,},
    },
};


void VideoCfgPrint()
{
    int i;
    //printf("********** Video *********\n");
    for(i = 0; i < MAX_VENC_STREAM_NUM; i ++) {
        printf("stream%d:\n", i);
        CfgPrintMap(videoMap[i]);
        //printf("\n");
    }
    //printf("********** Video *********\n\n");
}

int VideoCfgSave()
{
    cJSON *root, *array, *item;
    char *out;

    root = cJSON_CreateObject();//\B4\B4\BD\A8\CF\EEĿ
    array = cJSON_CreateArray();
    int i;
    for (i = 0; i < MAX_VENC_STREAM_NUM; i ++) {
        item = CfgDataToCjsonByMap(videoMap[i]);
        cJSON_AddItemToObject(array, "item0", item);
    }
    cJSON_AddItemToObject(root, "vencstream", array);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(VIDEO_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", VIDEO_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int VideoCfgLoadDefValue()
{
    int i;
    for (i = 0; i < MAX_VENC_STREAM_NUM; i ++) {
        CfgLoadDefValue(videoMap[i]);
    }

    return 0;
}


int VideoCfgLoad()
{
    char *data = NULL;
    data = CfgReadFromFile(VIDEO_CFG_FILE);
    if (data == NULL) {
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\B6\C1ȡʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        webcam_error("load %s error, so to load default cfg param.\n", VIDEO_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\BD\E2\CE\F6cjsonʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    cJSON *array = NULL;
    array = cJSON_GetObjectItem(json, "vencstream");
    if(!array){
        webcam_error("get venc stream error\n");
        goto err1;
    }

	int arraySize = cJSON_GetArraySize(array);
	//webcam_error("arraySize=%d\n", arraySize);

    int index;
    cJSON *arrayItem = NULL;
    for(index = 0; index < arraySize; index++) {
        arrayItem = cJSON_GetArrayItem(array, index);
        if(!arrayItem){
            webcam_error("cJSON_GetArrayItem error\n");
            goto err1;
        }
        CfgCjsonToDataByMap(videoMap[index], arrayItem);
    }


    cJSON_Delete(json);
    free(data);
    if(runVideoCfg.vencStream[0].h264Conf.vbrMaxbps > 10000)
    {
        webcam_debug("video bps setting error,reset it\n");
        VideoCfgLoadDefValue();
        VideoCfgSave();
    }
    return 0;

err1:
    cJSON_Delete(json);
    free(data);
err:
    VideoCfgLoadDefValue();
    VideoCfgSave();
    return 0;

}
cJSON * VideoCfgLoadJson(){
   char *data = NULL;
    /* add by qqw */
    data = CfgReadFromFile(VIDEO_CFG_FILE);
    if (data == NULL) {
        //ŽÓÅäÖÃÎÄŒþ¶ÁÈ¡Ê§°Ü£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý
        webcam_debug("load %s error, so to load default cfg param.\n", VIDEO_CFG_FILE);
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

char* videoCfgLoadStreamJson( int streamId)
{
    cJSON *opt, *item,*streamJson,*resloution;
    char *out;
    char buf[64];
    int i = 0;

	//gbk => utf8
	//char bufer[128]={0};
	//gbk_to_utf8(runVideoCfg.vencStream[streamId].streamFormat.streamName, bufer, strlen(runVideoCfg.vencStream[streamId].streamFormat.streamName));
	//strcpy( runVideoCfg.vencStream[streamId].streamFormat.streamName, bufer);

    item = CfgDataToCjsonByMap(videoMap[streamId]);
    if(item == 0)
    {
        return 0;
    }
    opt =cJSON_Parse(videoMap[streamId][4].description);
    if(opt == 0)
    {
        return 0;
    }
    sprintf(buf,"%dx%d",runVideoCfg.vencStream[streamId].streamFormat.width,runVideoCfg.vencStream[streamId].streamFormat.height);
    streamJson = cJSON_GetObjectItem(opt,"opt");
    if(streamJson == NULL)
    {
		return 0;
    }
    //printf("resolution:%s, len:%d,steam:%s\n",buf,cJSON_GetArraySize(streamJson),cJSON_Print(streamJson));
    for(i = 0 ; i < cJSON_GetArraySize(streamJson); i++)
    {
        //printf("i:%s\n",cJSON_GetArrayItem(streamJson,i)->valuestring);
        if(strcmp(buf,cJSON_GetArrayItem(streamJson,i)->valuestring) == 0)
        {
            webcam_debug("break i:%d\n",i);
            break;
        }
    }
    if(i == cJSON_GetArraySize(streamJson))
    {

 		webcam_debug("private resolution: %dx%d,use default: 1280*720\n",runVideoCfg.vencStream[streamId].streamFormat.width,runVideoCfg.vencStream[streamId].streamFormat.height);
     	sprintf(buf,"1280x720");
    }
    //webcam_debug("i =%d\n",i);
    resloution = cJSON_CreateString(buf);
    cJSON_AddItemToObject(item, "resloution", resloution);
    cJSON_AddItemToObject(item, "resloution_opt", streamJson);

    out = cJSON_Print(item);
    webcam_debug("test out:%s\n",out);
    cJSON_Delete(item);

    return out;
}
int VideoJsonSaveCfg(cJSON *json){
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(VIDEO_CFG_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", VIDEO_CFG_FILE);
        return -1;
    }

    free(out);
    VideoCfgLoad();
    return 0;
}

