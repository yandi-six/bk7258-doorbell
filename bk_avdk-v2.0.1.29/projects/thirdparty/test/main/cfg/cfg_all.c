/*!
*****************************************************************************

*****************************************************************************
*/

#include "cfg_all.h"

beken_mutex_t g_cfg_wr_mutex = NULL;

int CfgSaveAll()
{
    SystemCfgSave();
    NetworkCfgSave();
    MyRecordCfgSave();
    RoiCfgSave();
    OtherCfgSave();
    TimezoneCfgSave();
    RecstateCfgSave();
    // RecordCfgSave();

    /*
        AudioCfgSave();
        VideoCfgSave();
        ImageCfgSave();
        ChannelCfgSave();
        MdCfgSave();
        SnapCfgSave();
        RecordCfgSave();
        AlarmInCfgSave();
        AlarmOutCfgSave();
        PtzCfgSave();
        UserCfgSave();
        PirCfgSave();
        ScheduleCfgSave();
        PresetCruiseCfgSave();
        ScenemodeCfgSave();
        SystemInfoCfgSave();
        RtspCfgSave();
        OnvifCfgSave();
        WebCfgSave();
        WebRTCCfgSave();
        FtpCfgSave();
        EmailCfgSave();
        FrCfgSave();
        CouldCfgSave();
        EventPushCfgSave();
        VoiceAlarmCfgSave();
        LinkageCfgSave();
    */
    return 0;
}

int CfgLoadAll()
{
    SystemCfgLoad();
    NetworkCfgLoad();
    MyRecordCfgLoad();
    RoiCfgLoad();
    OtherCfgLoad();
    TimezoneCfgLoad();
    RecstateCfgLoad();
    // RecordCfgLoad();
    /*
        AudioCfgLoad();
        VideoCfgLoad();
        ImageCfgLoad();
        ChannelCfgLoad();
        MdCfgLoad();
        SnapCfgLoad();
        RecordCfgLoad();
        AlarmInCfgLoad();
        AlarmOutCfgLoad();
        PtzCfgLoad();
        UserCfgLoad();
        PirCfgLoad();
        ScheduleCfgLoad();
        PresetCruiseCfgLoad();
        ScenemodeCfgLoad();
        LoadSystemInfo();
        RtspCfgLoad();
        OnvifCfgLoad();
        WebCfgLoad();
        WebRTCCfgLoad();
        FtpCfgLoad();
        EmailCfgLoad();
        FrCfgLoad();
        CouldCfgLoad();
        EventPushCfgLoad();
        VoiceAlarmCfgLoad();
        LinkageCfgLoad();
    */

    return 0;
}

int CfgLoadDefValueAll()
{
    SystemCfgLoadDefValue();
    NetworkCfgLoadDefValue();

    MyRecordCfgLoadDefValue();
    RoiCfgLoadDefValue();
    OtherCfgLoadDefValue();
    TimezoneCfgLoadDefValue();
    RecstateCfgLoadDefValue();
    // RecordCfgLoadDefValue();
    /*
        AudioCfgLoadDefValue();
        VideoCfgLoadDefValue();
        ImageCfgLoadDefValue();
        ChannelCfgLoadDefValue();

        MdCfgLoadDefValue();
        SnapCfgLoadDefValue();
        RecordCfgLoadDefValue();
        AlarmInCfgLoadDefValue();
        AlarmOutCfgLoadDefValue();
        PtzCfgLoadDefValue();
        UserCfgLoadDefValue();
        PirCfgLoadDefValue();
        ScheduleCfgLoadDefValue();
        PresetCruiseCfgDefault();
        ScenemodeCfgLoadDefValue();
        RtspCfgLoadDefValue();
        OnvifCfgLoadDefValue();
        WebCfgLoadDefValue();
        WebRTCCfgLoadDefValue();
        FtpCfgLoadDefValue();
        EmailCfgLoadDefValue();
        FrCfgLoadDefValue();
        CouldCfgLoadDefValue();
        EventPushCfgLoadDefValue();
        VoiceAlarmCfgLoadDefValue();
        LinkageCfgLoadDefValue();
    */
    return 0;
}

int CfgInit()
{

    rtos_init_mutex(&g_cfg_write_mutex);
    rtos_init_mutex(&g_cfg_wr_mutex);
    CfgLoadAll();

    return 0;
}

int CfgUnInit()
{
    CfgSaveAll();
    rtos_deinit_mutex(&g_cfg_wr_mutex);
    rtos_deinit_mutex(&g_cfg_write_mutex);

    return 0;
}

int cfg_get_param(PARAM_ID id, void *dest)
{
    rtos_lock_mutex(&g_cfg_wr_mutex);
    switch (id)
    {
    // SDK_NET_SYSTEM_CFG runSystemCfg;
    case SYSTEM_PARAM_ID:
        os_memcpy(dest, &runSystemCfg, sizeof(SDK_NET_SYSTEM_CFG));
        break;

    // SDK_NET_AUDIO_CFG runAudioCfg;
    case AUDIO_PARAM_ID:
        os_memcpy(dest, &runAudioCfg, sizeof(SDK_NET_AUDIO_CFG));
        break;

    // SDK_NET_VIDEO_CFG runVideoCfg;
    case VIDEO_PARAM_ID:
        os_memcpy(dest, &runVideoCfg, sizeof(SDK_NET_VIDEO_CFG));
        break;

    // SDK_NET_IMAGE_CFG runImageCfg;
    case IMAGE_PARAM_ID:
        os_memcpy(dest, &runImageCfg, sizeof(SDK_NET_IMAGE_CFG));
        break;

    // SDK_NET_CHANNEL_CFG runChannelCfg;
    case CHANNEL_PARAM_ID:
        os_memcpy(dest, &runChannelCfg, sizeof(SDK_NET_CHANNEL_CFG));
        break;

    // SDK_NET_NETWORK_CFG runNetworkCfg;
    case NETWORK_PARAM_ID:
        os_memcpy(dest, &runNetworkCfg, sizeof(SDK_NET_NETWORK_CFG));
        break;

    // SDK_NET_MD_CFG runMdCfg;
    case MD_PARAM_ID:
        os_memcpy(dest, &runMdCfg, sizeof(SDK_NET_MD_CFG));
        break;

    // SDK_NET_SNAP_CFG runSnapCfg;
    case SNAP_PARAM_ID:
        os_memcpy(dest, &runSnapCfg, sizeof(SDK_NET_SNAP_CFG));
        break;

    // SDK_NET_RECORD_CFG runRecordCfg;
    case RECORD_PARAM_ID:
        os_memcpy(dest, &runRecordCfg, sizeof(SDK_NET_RECORD_CFG));
        break;

    // SDK_NET_ALARM_IN_CFG runAlarmInCfg;
    case ALARM_PARAM_IN_ID:
        os_memcpy(dest, &runAlarmInCfg, sizeof(SDK_NET_ALARM_IN_CFG));
        break;
    // SDK_NET_ALARM_OUT_CFG runAlarmOutCfg;
    case ALARM_PARAM_OUT_ID:
        os_memcpy(dest, &runAlarmOutCfg, sizeof(SDK_NET_ALARM_OUT_CFG));
        break;

    // SDK_NET_PTZ_CFG runPtzCfg;
    case PTZ_PARAM_ID:
        os_memcpy(dest, &runPtzCfg, sizeof(SDK_NET_PTZ_CFG));
        break;

    // SDK_NET_USER_CFG runUserCfg;
    case USER_PARAM_ID:
        os_memcpy(dest, &runUserCfg, sizeof(SDK_NET_USER_CFG));
        break;

    // SDK_NET_PtzDec_CFG runRS232Cfg;
    case PTZ_DEC_PARAM_ID:
        os_memcpy(dest, &runPtzDecCfg, sizeof(SDK_NET_DECODERCFG));
        break;

    // SDK_NET_PtzPreset_CFG runPresetCfg;
    case PTZ_PRESET_PARAM_ID:
        os_memcpy(dest, &runPresetCfg, sizeof(SDK_NET_PRESET_INFO));
        break;

    // SDK_NET_PtzCruise_CFG runCruiseCfg;
    case PTZ_CRUISE_PARAM_ID:
        os_memcpy(dest, &runCruiseCfg, sizeof(SDK_NET_CRUISE_CFG));
        break;

    case MYRECORD_PARAM_ID:
        os_memcpy(dest, &runMyRecordCfg, sizeof(SDK_NET_MYRECORD_CFG));
        break;

    case ROI_PARAM_ID:
        os_memcpy(dest, &runRoiCfg, sizeof(SDK_NET_ROI_CFG));
        break;

    case OTHER_PARAM_ID:
        os_memcpy(dest, &runOtherCfg, sizeof(SDK_NET_OTHER_CFG));
        break;

    case TIMEZONE_PARAM_ID:
        os_memcpy(dest, &runTimezoneCfg, sizeof(SDK_NET_TIMEZONE_CFG));
        break;

    case REC_STATE_PARAM_ID:
        os_memcpy(dest, &runRecordstateCfg, sizeof(SDK_RECORD_STATE_CFG));
        break;

    default:
        break;
    }

    rtos_unlock_mutex(&g_cfg_wr_mutex);
    return 0;
}

int cfg_set_param(PARAM_ID id, void *src)
{
    rtos_lock_mutex(&g_cfg_wr_mutex);
    switch (id)
    {
    // SDK_NET_SYSTEM_CFG runSystemCfg;
    case SYSTEM_PARAM_ID:
        os_memcpy(&runSystemCfg, src, sizeof(SDK_NET_SYSTEM_CFG));
        break;

    // SDK_NET_AUDIO_CFG runAudioCfg;
    case AUDIO_PARAM_ID:
        os_memcpy(&runAudioCfg, src, sizeof(SDK_NET_AUDIO_CFG));
        break;

    // SDK_NET_VIDEO_CFG runVideoCfg;
    case VIDEO_PARAM_ID:
        os_memcpy(&runVideoCfg, src, sizeof(SDK_NET_VIDEO_CFG));
        break;

    // SDK_NET_IMAGE_CFG runImageCfg;
    case IMAGE_PARAM_ID:
        os_memcpy(&runImageCfg, src, sizeof(SDK_NET_IMAGE_CFG));
        break;

    // SDK_NET_CHANNEL_CFG runChannelCfg;
    case CHANNEL_PARAM_ID:
        os_memcpy(&runChannelCfg, src, sizeof(SDK_NET_CHANNEL_CFG));
        break;

    // SDK_NET_NETWORK_CFG runNetworkCfg;
    case NETWORK_PARAM_ID:
        os_memcpy(&runNetworkCfg, src, sizeof(SDK_NET_NETWORK_CFG));
        break;

    // SDK_NET_MD_CFG runMdCfg;
    case MD_PARAM_ID:
        os_memcpy(&runMdCfg, src, sizeof(SDK_NET_MD_CFG));
        break;

    // SDK_NET_SNAP_CFG runSnapCfg;
    case SNAP_PARAM_ID:
        os_memcpy(&runSnapCfg, src, sizeof(SDK_NET_SNAP_CFG));
        break;

    // SDK_NET_RECORD_CFG runRecordCfg;
    case RECORD_PARAM_ID:
        os_memcpy(&runRecordCfg, src, sizeof(SDK_NET_RECORD_CFG));
        break;

    // SDK_NET_ALARM_IN_CFG runAlarmInCfg;
    case ALARM_PARAM_IN_ID:
        os_memcpy(&runAlarmInCfg, src, sizeof(SDK_NET_ALARM_IN_CFG));
        break;

    // SDK_NET_ALARM_OUT_CFG runAlarmOutCfg;
    case ALARM_PARAM_OUT_ID:
        os_memcpy(&runAlarmOutCfg, src, sizeof(SDK_NET_ALARM_OUT_CFG));
        break;

    // SDK_NET_PTZ_CFG runPtzCfg;
    case PTZ_PARAM_ID:
        os_memcpy(&runPtzCfg, src, sizeof(SDK_NET_PTZ_CFG));
        break;

    // SDK_NET_USER_CFG runUserCfg;
    case USER_PARAM_ID:
        os_memcpy(&runUserCfg, src, sizeof(SDK_NET_USER_CFG));
        break;

    // SDK_NET_PtzDec_CFG runRS232Cfg;
    case PTZ_DEC_PARAM_ID:
        os_memcpy(&runPtzDecCfg, src, sizeof(SDK_NET_DECODERCFG));
        break;

    // SDK_NET_PtzPreset_CFG runPresetCfg;
    case PTZ_PRESET_PARAM_ID:
        os_memcpy(&runPresetCfg, src, sizeof(SDK_NET_PRESET_INFO));
        break;

    // SDK_NET_PtzCruise_CFG runCruiseCfg;
    case PTZ_CRUISE_PARAM_ID:
        os_memcpy(&runCruiseCfg, src, sizeof(SDK_NET_CRUISE_CFG));
        break;

    case MYRECORD_PARAM_ID:
        os_memcpy(&runMyRecordCfg, src, sizeof(SDK_NET_MYRECORD_CFG));
        break;

    case ROI_PARAM_ID:
        os_memcpy(&runRoiCfg, src, sizeof(SDK_NET_ROI_CFG));
        break;

    case OTHER_PARAM_ID:
        os_memcpy(&runOtherCfg, src, sizeof(SDK_NET_OTHER_CFG));
        break;

    case TIMEZONE_PARAM_ID:
        os_memcpy(&runTimezoneCfg, src, sizeof(SDK_NET_TIMEZONE_CFG));
        break;

    case REC_STATE_PARAM_ID:
        os_memcpy(&runRecordstateCfg, src, sizeof(SDK_RECORD_STATE_CFG));
        break;

    default:
        break;
    }

    rtos_unlock_mutex(&g_cfg_wr_mutex);
    return 0;
}
