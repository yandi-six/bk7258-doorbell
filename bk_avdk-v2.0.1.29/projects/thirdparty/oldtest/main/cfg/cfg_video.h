/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_VIDEO_H__
#define _SDK_CFG_VIDEO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

#define MAX_VENC_STREAM_NUM 2
#define MAX_VENC_ITEM_NUM 25



typedef struct {
    /*0: none, 1: H.264, 2: MJPEG*/
    SDK_U8                     encodeType;
    /*encode width.*/
    SDK_U16                    width;
    /*encode height.*/
    SDK_U16                    height;
    /*encode frame rate.*/
    SDK_U32    fps;
    /*encode video keep aspect ratio.*/
    /*vi width scale to video width ratio not equal video height ratio.*/
    /*keep aspect ration means use the small aspect ratio.*/
    /* 1: means keep aspect ration, 0: means do not keep.*/
    SDK_U8                     keepAspRat;
    SDK_U8                     streamName[32];
}SDK_NET_VENC_StreamFormatT;

typedef struct {
    /*gop M value.*/
    SDK_U8             gopM;
    /*gop N value.*/
    SDK_U8             gopN;
    /*IDR SDK_S32erval .*/
    SDK_U8             idrInterval;
    /*gop model.*/
    SDK_U8             gopModel;
    /*encode profile. 0: main, 1: baseline */
    SDK_U8             profile;
    /*0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality: SDK_VENC_BrcModeEnumT*/
    SDK_U8             brcMode;
    /*cbr mode, bit rate.*/
    SDK_U32            cbrAvgBps;
    /*vbr mode, min bit rate.*/
    SDK_U32            vbrMinbps;
    /*vbr mode, max bit rate.*/
    SDK_U32            vbrMaxbps;
    /*picure quality 0,3,0: poor, 3: best*/
    SDK_U8             quality;
    /*picure quality consistency ,0: poor ,3 best*/
    SDK_U8             qcon;
}SDK_NET_VENC_H264ConfigT;

typedef struct {
    /*0: YUV 422, 1: YUV 420.*/
    SDK_U8             chromaFormat;
    /*1 ~ 100, 100 is best quality.*/
    SDK_U8             quality;
} SDK_NET_VENC_MjpegConfigT;

typedef struct {
    SDK_S32 id;
    SDK_S32 enable;
    SDK_S32 avStream;
    SDK_NET_VENC_StreamFormatT streamFormat;
    SDK_NET_VENC_H264ConfigT   h264Conf;
    SDK_NET_VENC_MjpegConfigT  mjpegConf;
} SDK_NET_VENC_STREAM;

typedef struct {
    SDK_NET_VENC_STREAM vencStream[4];
} SDK_NET_VIDEO_CFG;


typedef enum {
    SDK_VIDEO_176X120 = 0, //QCIF
    SDK_VIDEO_176X144,
    SDK_VIDEO_352X240, //CIF
    SDK_VIDEO_352X288,
    SDK_VIDEO_720X240, //HD1
    SDK_VIDEO_720X288,
    SDK_VIDEO_704X240, //HD1
    SDK_VIDEO_704X288,
    SDK_VIDEO_720X480, //D1
    SDK_VIDEO_720X576,
    SDK_VIDEO_704X480, //D1
    SDK_VIDEO_704X576,
    SDK_VIDEO_160X120, //QQVGA
    SDK_VIDEO_320X240, //QVGA
    SDK_VIDEO_640X480, //VGA
    SDK_VIDEO_800X600, //VGA
    SDK_VIDEO_960X480,
    SDK_VIDEO_960X576,
    SDK_VIDEO_1024X768,
    SDK_VIDEO_1024X576,
    SDK_VIDEO_640X360, //360P
    SDK_VIDEO_1280X720, //720P
    SDK_VIDEO_1280X960, //960P
    SDK_VIDEO_1280X1024, //1024P
    SDK_VIDEO_1920X1080, //1080P
    SDK_VIDEO_2048X1536,
} SDK_VIDEO_RESOLUTION_E;

extern int VideoCfgSave();
extern int VideoCfgLoad();
extern void VideoCfgPrint();
extern int VideoCfgLoadDefValue();
extern char* videoCfgLoadStreamJson( int streamId);
extern cJSON * VideoCfgLoadJson();
extern int VideoJsonSaveCfg(cJSON *json);
#define VIDEO_CFG_FILE "video_cfg.cjson"

extern SDK_NET_VIDEO_CFG runVideoCfg;
extern SDK_CFG_MAP videoMap[MAX_VENC_STREAM_NUM][MAX_VENC_ITEM_NUM];

#ifdef __cplusplus
}
#endif
#endif

