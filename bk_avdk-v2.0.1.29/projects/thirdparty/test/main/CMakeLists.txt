set(incs .)

set(srcs app_main.c)


list(APPEND incs ./cfg)
list(APPEND srcs
    webrtc_main.c
)
if (CONFIG_SYS_CPU0)
list(APPEND srcs
    cJSON.c
    network_configure.c
    mongoose.c
    webserver.c
    stepmotor.c
    cfg/cfg_all.c
    cfg/cfg_common.c
    cfg/cfg_system.c
    cfg/cfg_network.c
    cfg/cfg_myrecord.c
    cfg/cfg_roi.c
    cfg/cfg_other.c
    cfg/cfg_timezone.c
    cfg/cfg_record_state.c
)
endif()
if (CONFIG_SYS_CPU1)
list(APPEND incs ./media)
list(APPEND srcs
    media/bk_dvp.c
    media/bk_psram_mem_slab.c
    media/bk_frame_buffer.c
    media/bk_aud_intf.c
    media/bk_aud_tras.c
    media/bk_aud_tras_drv.c
    media/bk_audio_osi_wrapper.c
    media/bk_video_osi_wrapper.c
    time/time.c
    time/ntp.c
    time/time_intf.c
    cJSON.c
    cfg/cfg_all.c
    cfg/cfg_common.c
    cfg/cfg_system.c
    cfg/cfg_network.c
    webrtc_motion.c
    cfg/cfg_myrecord.c
    cfg/cfg_roi.c
    cfg/cfg_other.c
    cfg/cfg_timezone.c
    cfg/cfg_record_state.c
)
endif()



if (CONFIG_SYS_CPU0)
set(priv_req bk_init bt lwip_intf_v2_1 webrtc_major project_defs webrtc_mdns bk_vfs fatfs)
endif()

if (CONFIG_SYS_CPU1)
set(priv_req avdk_libs webrtc_minor project_defs webrtc_stream)
if (CONFIG_WEBRTC_MP4)
list(APPEND priv_req webrtc_mp4)
endif()
if (CONFIG_WEBRTC_AVI)
list(APPEND priv_req webrtc_avi)
endif()
if (CONFIG_WEBRTC_JPEG)
list(APPEND priv_req webrtc_jpeg)
list(APPEND srcs  webrtc_jpeg.c)
endif()
endif()
if (CONFIG_SYS_CPU2)

list(APPEND srcs

    webrtc_motion.c

)
set(priv_req bk_init bt project_defs)
endif()
if (CONFIG_OVERRIDE_FLASH_PARTITION)
	list(APPEND srcs
		vendor_flash.c
	)
endif()

armino_component_register(
	SRCS "${srcs}"
	INCLUDE_DIRS "${incs}"
	PRIV_REQUIRES "${priv_req}"
)
