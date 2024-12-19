/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_ALL_H__
#define _SDK_CFG_ALL_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <sys/time.h>


#include "cfg_audio.h"
#include "cfg_image.h"
#include "cfg_video.h"
#include "cfg_system.h"
#include "cfg_network.h"
#include "cfg_channel.h"
#include "cfg_md.h"
#include "cfg_snap.h"
#include "cfg_record.h"
#include "cfg_alarm_in.h"
#include "cfg_alarm_out.h"
#include "cfg_ptz.h"
#include "cfg_user.h"
#include "cfg_pir.h"
#include "cfg_schedule.h"
#include "cfg_scenemode.h"
#include "cfg_www.h"
#include "cfg_onvif.h"
#include "cfg_rtsp.h"
#include "cfg_webrtc.h"
#include "cfg_ftp.h"
#include "cfg_email.h"
#include "cfg_facerecognition.h"
#include "cfg_could.h"
#include "cfg_event_push.h"
#include "cfg_voicealarm.h"
#include "cfg_linkage.h"

#include "cfg_myrecord.h"
#include "cfg_roi.h"
#include "cfg_other.h"
#include "cfg_timezone.h"
#include "cfg_record_state.h"

typedef enum {
    SYSTEM_PARAM_ID = 0,
    AUDIO_PARAM_ID,
    VIDEO_PARAM_ID,
    IMAGE_PARAM_ID,
    CHANNEL_PARAM_ID,
    NETWORK_PARAM_ID,
    MD_PARAM_ID,
    SNAP_PARAM_ID,
    RECORD_PARAM_ID,
    ALARM_PARAM_IN_ID,
    ALARM_PARAM_OUT_ID,
    PTZ_PARAM_ID,
    USER_PARAM_ID,
    PTZ_DEC_PARAM_ID,
    PTZ_PRESET_PARAM_ID,
    PTZ_CRUISE_PARAM_ID,
    
    MYRECORD_PARAM_ID,
    ROI_PARAM_ID,
    OTHER_PARAM_ID,
    TIMEZONE_PARAM_ID,
    REC_STATE_PARAM_ID,
} PARAM_ID;

extern int CfgInit();
extern int CfgUnInit();
extern int CfgSaveAll();
extern int CfgLoadAll();
extern int CfgLoadDefValueAll();

int cfg_get_param(PARAM_ID id, void *dest);
int cfg_set_param(PARAM_ID id, void *src);

extern beken_mutex_t g_cfg_write_mutex;

#ifdef __cplusplus
}
#endif
#endif
