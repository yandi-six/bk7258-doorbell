#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <components/shell_task.h>
#include <components/event.h>
#include <components/log.h>
#include <driver/pwr_clk.h>
#include <driver/mailbox_channel.h>
#include "rtc_bk.h"
#include "rtc_list.h"

#include <modules/wifi.h>
#include <components/netif.h>
#include "bk_wifi.h"
#include <driver/pwr_clk.h>
#include <driver/flash.h>
#include <driver/flash_partition.h>
#include "flash_driver.h"
#include "bk_gpio.h"
#include <driver/gpio.h>
#include <driver/hal/hal_gpio_types.h>
#include "gpio_driver.h"
#include "time/time.h"
#include "time/time_intf.h"

#include "project_defs.h"
#if  CONFIG_SYS_CPU1
#include "bk_audio_osi_wrapper.h"
#include "bk_video_osi_wrapper.h"
#endif

#if CONFIG_SYS_CPU0

#ifdef CONFIG_BT_REUSE_MEDIA_MEMORY
#include "components/bluetooth/bk_ble.h"
#include "components/bluetooth/bk_dm_ble.h"
#include "components/bluetooth/bk_dm_bluetooth.h"
#endif
#include "stepmotor.h"
#include "network_configure.h"
#include <driver/adc.h>
#include "socket_major.h"
#include "vfs_file_major.h"
#include "cfg_all.h"
#include "mongoose.h"
#include "webserver.h"
#include <time.h>
#if CONFIG_WEBRTC_MDNS
#include "webrtc_mdns.h"
#endif
#include <lwip/api.h>
#include <lwip/apps/sntp.h>
#if CONFIG_VFS
#include "bk_vfs.h"
#include "bk_filesystem.h"
#endif
#endif

#if CONFIG_SYS_CPU1
#if CONFIG_TRNG_SUPPORT 
#include <driver/trng.h>
#endif
#include "g711common.h"
#include "bk_frame_buffer.h"
#include "bk_psram_mem_slab.h"
#include "bk_peripheral.h"
#include "socket_minor.h"
#include "vfs_file_minor.h"
#include "bk_dvp.h"
#include "cJSON.h"
//#if CONFIG_WEBRTC_SDK
#include "webrtc_streamer.h"
//#endif
#if CONFIG_WEBRTC_MP4
#include "webrtc-mp4.h"
#endif
#if CONFIG_WEBRTC_AVI
#include "webrtc_avilib.h"
#endif
#if (CONFIG_WEBRTC_JPEG)
#include "webrtc_jpeg.h"
#endif
#include "bk_aud_intf.h"
#include "bk_aud_intf_types.h"
#include "cfg_all.h"
#include "webrtc_motion.h"
#endif
#if CONFIG_SYS_CPU2
#include "webrtc_motion.h"
#endif

#define   USE_CPU2  1


#define TAG "runhua_main"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)


#define GPIO_OFF   1376317 
#define GPIO_ON    1376316
#define SYSTEM_INFO_HEAD 0x0e1a0b4f
typedef struct _system_info_data_tag{
	uint32_t head;
        char serialNumber[64];
 	char serverAddr[128];
	char initString[256];		
}system_info_data_;

extern uint32_t platform_is_in_interrupt_context( void );


static beken_semaphore_t event_sem = NULL;
static u32 g_mailbox_msg_id = 0;
static bool runhua_runing = false;
static bool mailbox_thread_runing = false;
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
static beken_queue_t mailbox_cpu0_cpu1_msg_queue_req = NULL;
static beken_queue_t mailbox_cpu0_cpu1_msg_queue_resp = NULL;
static beken_mutex_t mailbox_cpu0_cpu1_send_msg_mutex = NULL;
static beken_thread_t  mailbox_cpu0_cpu1_req_thread_hdl = NULL;
static beken_thread_t  mailbox_cpu0_cpu1_resp_thread_hdl = NULL;
static beken_semaphore_t mailbox_cpu0_cpu1_req_sem = NULL;
static beken_semaphore_t mailbox_cpu0_cpu1_resp_sem = NULL;
#endif
#if (CONFIG_SYS_CPU1 || CONFIG_SYS_CPU2)
static beken_queue_t mailbox_cpu1_cpu2_msg_queue_req = NULL;
static beken_queue_t mailbox_cpu1_cpu2_msg_queue_resp = NULL;
static beken_mutex_t mailbox_cpu1_cpu2_send_msg_mutex = NULL;
static beken_thread_t  mailbox_cpu1_cpu2_req_thread_hdl = NULL;
static beken_thread_t  mailbox_cpu1_cpu2_resp_thread_hdl = NULL;
static beken_semaphore_t mailbox_cpu1_cpu2_req_sem = NULL;
static beken_semaphore_t mailbox_cpu1_cpu2_resp_sem = NULL;
#endif

static beken_thread_t  runhua_thread_hdl = NULL;

int webrtc_mailbox_send_media_req_msg(u32 param1,u32 param2,u32 param3);
int webrtc_mailbox_send_media_response_msg(u32 param1,u32 param2,u32 param3);
void webrtc_handle_req_rx(webrtc_cmd_t *pcmd);
void webrtc_handle_resp_rx(webrtc_cmd_t *pcmd);
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
static project_config_t project_config = {0};
#endif
#ifdef USE_CPU2
#if (CONFIG_SYS_CPU1 || CONFIG_SYS_CPU2)
static project_config_t project_config_cpu12 = {0};
#endif
#endif

#if CONFIG_SYS_CPU1
typedef struct _audio_data{
        char *data;
	int size;	
}audio_data;

typedef struct _record_data{
	bool isvideo;
        char *data;
	int size;	
}record_data;

typedef struct _Webrtc_remoteplay{
        bool stop_;
        webrtc_avi_t *hAviHandle;
	bool started;
	int nFileLen;
        char szsessionId[128];
	char szfrom[128];
        char szto[128];
        char szsessionType[128];
        char szpath[256];
	char szname[256];


	int datachannel_streamid;

	beken_thread_t pthid_;
	RTCList *msg_list;

	long frames;
	int framew;
	int frameh;
	double framerate; 


	int audiobytes ;
	int audiochannels;
	int audiobits;	
	long audiorate;


        long delay;
	bool bPause;
	bool bStart;
	int sended;
        long long lpreshowtime ;
        long long lprevideotime;
	long long lpreaudiotime;
	long long lpcurrenttime;
        int streams;
	bool stopvideo;
	bool stopaudio;
	bool seek;
        int  seekIndex;
	webrtc_video_code_type_t code_type;

}Webrtc_remoteplay;

static RTCList *g_webrtc_remoteplay = NULL;
static RTCList *record_data_list = NULL;
static RTCList *audio_data_list = NULL;
static beken_mutex_t audio_mutex = NULL;
typedef struct _datachannel_msg{
        char *msg;
	int msgsize;
	int streamid;
	
}datachannel_msg;

static RTCList *dc_msg_list = NULL;
static beken_mutex_t datachannel_mutex = NULL;

static beken_thread_t  socket_thread_hdl = NULL;
static bool socket_runing = false;
static beken_thread_t  webrtc_media_thread_hdl = NULL;
static beken_thread_t  webrtc_media_yuv_thread_hdl = NULL;
static beken_semaphore_t webrtc_media_yuv_exit_sem = NULL;
static beken_semaphore_t webrtc_media_exit_sem = NULL;
static bool webrtc_media_camera_ok = false;
static bool webrtc_media_thread_runing = false;
static bool webrtc_yuv_thread_runing = false;

media_debug_t *media_debug = NULL;
static int g_socket = -1;
static bool g_socket_send = false;
static media_camera_device_t camera_device = {
	.type = DVP_CAMERA,
	.mode = H264_YUV_MODE, //H264_YUV_MODE   H264_MODE
	.fmt = PIXEL_FMT_H264,
	.info.resolution.width = 640,
	.info.resolution.height = 480,
	.info.fps = FPS25,
};

static aud_intf_drv_setup_t aud_intf_drv_setup = DEFAULT_AUD_INTF_DRV_SETUP_CONFIG();
static aud_intf_voc_setup_t aud_intf_voc_setup = DEFAULT_AUD_INTF_VOC_SETUP_CONFIG();
static aud_intf_spk_setup_t aud_intf_spk_setup = DEFAULT_AUD_INTF_SPK_SETUP_CONFIG();
static aud_intf_work_mode_t aud_work_mode = AUD_INTF_WORK_MODE_NULL;
static bool audio_opened = false;
static uint8_t audio_type = 0;
static uint32_t audio_samp_rate = 8000;
static bool aec_enable = true;
static int video_width = 640;
static int video_height = 480;
static int video_fps = 25;

static uint32_t calculate_fps_starttime = 0;
static int calculate_video_fps = 0;

static bool webrtc_streamer_runing = false;
static bool webrtc_streamer_online = false;
static bool webrtc_streamer_start_cmd = false;

static bool webrtc_streamer_media_start = false;



static bool webrtc_record_runing = false;
static bool webrtc_recording = false;
static bool webrtc_record_skip = false;
static bool webrtc_can_record = false;
static int  webrtc_record_event = 0;
static beken_thread_t  webrtc_record_thread_hdl = NULL;
static beken_semaphore_t webrtc_record_sem = NULL;
static beken_semaphore_t webrtc_record_exit_sem = NULL;

static beken_queue_t webrtc_record_queue = NULL;
static beken_mutex_t webrtc_record_queue_mutex = NULL;

static bool webrtc_audio_play_runing = false;
static beken_thread_t  webrtc_audio_play_thread_hdl = NULL;
static beken_semaphore_t webrtc_audio_play_sem = NULL;
static beken_semaphore_t webrtc_audio_play_exit_sem = NULL;
static beken_semaphore_t webrtc_audio_playing_sem = NULL;
static bool webrtc_audio_playing_file = false;
static int play_file_index = 0;
static bool webrtc_audio_playing_init = false;
static bool webrtc_cpu1_save_network = false;
static bool webrtc_cpu1_cloud_publish_stream = false;
static char g_szCouldSessionId[64]= {0};
void webrtc_streamer_stop(void);
void webrtc_streamer_start(void);
static int sdcard_mounted = 0;
static bool sdcard_can_mount = false;
static int get_video_frame_count = 0;
static bool cpu2_runing = false;
static int get_yuv_frame_count = 0;
static bool motion_detection_enable = true;
static bool motion_detection_alarm = true;
static bool motion_detection_record = true;
static bool motion_detection_recording = false;
static bool motion_detectioning = false;
static uint32_t motion_detection_starttime = 0;
static int motion_detection_sensitivity = 5;
static uint32_t motion_detection_delay = 60*1000;
static char motion_detection_start_time[32] ="00:00:00";
static char motion_detection_end_time[32] ="23:59:59";
static bool yuv_frame_capture = false;
static bool record_snapshot = false;
static char record_snapshot_filename[64] ={0};
static char current_record_filename[64]= {0};
void webrtc_cpu1_list_files(const char *path);
void webrtc_sdcard_info(const char *path,int *total_space,int *free_space);

static int mirror_h = 0;
static int mirror_v = 0;
static int timezone_offset = 12;
static char timezone_abbr[32] = "CST";
static int timezone_isdst = 0;
static char timezone_name[64] = "China Standard Time";
static char timezone_text[128] = "(UTC+08:00) Beijing, Chongqing, Hong Kong, Urumqi";
static char timezone_ntp[64] = "time.windows.com";
static int webrtc_record_auto = 1;
static char webrtc_record_start_time[32] ="00:00:00";
static char webrtc_record_end_time[32] ="23:59:59";
static int webrtc_record_resolution_index = 0;
static int webrtc_record_time = 60;
static int webrtc_record_interval = 0;
static int webrtc_record_cover = 1;

static int webrtc_preview_enable = 1;
static int webrtc_preview_resolution_index = 0;

#endif




extern void delay_us(UINT32 us);
#if CONFIG_WIFI_ENABLE
extern void rwnxl_set_video_transfer_flag(uint32_t video_transfer_flag);
#else
#define rwnxl_set_video_transfer_flag(...)
#endif

#if CONFIG_SYS_CPU0
#if CONFIG_WEBRTC_MDNS
static beken_thread_t  mdns_thread_hal = NULL;
static beken_semaphore_t  webrtc_mdns_exit_sem = NULL;
static Webrtc_mdns *g_pmdns = NULL;
static bool mdns_runing = false;
#endif
void webrtc_http_post(char *url,char *post_data);
static bool wakeup_cpu2 = false;
static bool wakeup_cpu1 = false;
static bool cpu1_runing = false;
static bool wifiorble_start = false;
static bool wifi_conneced_sended = false;
static beken_thread_t  light_sensor_thread_hal = NULL;
static bool light_sensor_runing = false;
static beken_semaphore_t light_sensor_sem = NULL;
void webrtc_cpu0_list_files(const char *path);
typedef enum {
    SYSTEM_WIFI_CONFIG = 0,
    SYSTEM_RUNING,
}SYSTEM_RUN_STATE_E;
static SYSTEM_RUN_STATE_E system_run_state = SYSTEM_RUNING;
static bool g_wifi_config_finished = false;
static int wifi_connected =0;
static bool g_wifi_started = false;
void webrtc_mailbox_send_wifi_connect_msg(void);
void webrtc_mailbox_send_wifi_disconnect_msg(void);
void webrtc_shutdown_cpu1();
void webrtc_network_configure_update();
bool system_reboot = false;
bool network_configure_restart_state = false;
bool configure_wifi_update = false;
static char config_wifi_key[64]= {0};
static char config_wifi_name[64]= {0};
static bool webrtc_system_info_inited = false;
static bool webrtc_system_info_geting = false;
void webrtc_write_system_info(char *initstring,char* serveraddr,char*serialnumber);
void webrtc_erase_system_info();
void webrtc_unmount_sdcard(void);
void webrtc_mount_sdcard(void);
//#define KEY_RING GPIO_46 
#define SD_CARD_DETECT_PIN	GPIO_6
#define KEY_DEFAULT GPIO_44

#define IR_LED_PIN_NUMBER   46
#define FILIGHT_LED_PIN_NUMBER 45
#define IR_GAP_VALUE     500

static bool g_is_night_mode = false;
static uint32_t sd_card_detect = GPIO_OFF;
static bool sd_card_mounted = false;
static uint32_t default_key_down = GPIO_OFF;
static uint32_t default_key_down_timestamp = 0;
static stepmotor_msg g_stepmsg ={0};
static bool g_can_mount_sdcard = false;
#endif
uint32_t get_cur_timestamp(void){
	 return bk_get_milliseconds();
}
int webrtc_camera_rand(void){
#if CONFIG_TRNG_SUPPORT            
        return bk_rand();
#else
	return rand();
#endif
}
void webrtc_camera_random_uuid( char *buf ,int size)
{
    int addrpoint = (int)buf;
    uint64_t time = rtos_get_time();
    int seed = (addrpoint & 0x3FFFFFFF) + (time & 0x3FFFFFFF);
    const char *c = "69ab5681vghqw54312tmp0ikdflxsiybnz7";
    char *p = buf;
    int n;
    for( n = 0; n < 16; ++n )
    {
	time = rtos_get_time();
	seed = (addrpoint & 0x3FFFFFFF) + (time & 0x3FFFFFFF);
	srand(seed); 
        int b = webrtc_camera_rand()%255;
        switch( n )
        {
            case 6:
                sprintf(
                    p,
                    "4%x",
                    b%15 );
                break;
            case 8:
                sprintf(
                    p,
                    "%c%x",
                    c[webrtc_camera_rand()%strlen( c )],
                    b%15 );
                break;
            default:
                sprintf(
                    p,
                    "%02x",
                    b );
                break;
        }
        p += 2;
        switch( n )
        {
            case 3:
            case 5:
            case 7:
            case 9:
                *p++ = '-';
                break;
        }
    }
    *p = 0;  
   
}
#if CONFIG_SYS_CPU1
#if (CONFIG_WEBRTC_MP4) 
bool runhua_webrtc_aksiframe = false;
void play_mp4_stream(){	  
#if 1
          LOGW("thread_mp4_main_stream start\n");
	  int res =0;
	  webrtc_mp4_reader *reader = NULL;
	  char mp4file[64]= {0};
	  sprintf(mp4file,"%s:/%s","1","4KstanbulCityvga.mp4");
          //char s_pts[64], s_dts[64];
	  int64_t startpos = 0;
	  int64_t v_pts=0, v_dts=0;
	  int64_t a_pts=0, a_dts=0;
	  uint32_t now = get_cur_timestamp();
	  uint32_t video_read_next  = 0;
          uint32_t audio_read_next  = 0;
	  webrtc_reader_mp4_video_info_t *pvideo_info = psram_malloc(sizeof(webrtc_reader_mp4_video_info_t));
	  webrtc_reader_mp4_audio_info_t *paudio_info = psram_malloc(sizeof(webrtc_reader_mp4_audio_info_t));
	  os_memset(pvideo_info,0,sizeof(webrtc_reader_mp4_video_info_t));
	  os_memset(paudio_info,0,sizeof(webrtc_reader_mp4_audio_info_t));
	  webrtc_reader_mp4_param_t *pmp4_read_param = psram_malloc(sizeof(webrtc_reader_mp4_param_t));
	  int databufsize = 16*1024;
	  char *data = (char*)psram_malloc(databufsize);
	   reader = webrtc_mp4_reader_create(mp4file);
	   if(reader!= NULL){
			webrtc_mp4_reader_getvideoinfo(reader,pvideo_info);
			LOGW("mp4 is video  = %d object = 0x%x width = %d height = %d \n",pvideo_info->isvideo,pvideo_info->object,pvideo_info->width,pvideo_info->height);
			webrtc_mp4_reader_getaudioinfo(reader,paudio_info);
			LOGW("mp4 is audio  = %d object = 0x%x sample_rate = %d bit_per_sample = %d channel_count = %d \n",paudio_info->isaudio,paudio_info->object,paudio_info->sample_rate,paudio_info->bit_per_sample,paudio_info->channel_count);
	   }else{
		LOGW("Failed webrtc_mp4_reader_create %s\n",mp4file);	
	   }
	  while(runhua_runing){
				now = get_cur_timestamp();
				if(runhua_webrtc_aksiframe){
					runhua_webrtc_aksiframe = false;
					video_read_next  = now;
					audio_read_next  = now;
				        startpos = 0;
					webrtc_mp4_reader_seek(reader,&startpos);
				}
				
				if((pvideo_info->isvideo && now>=video_read_next) || (paudio_info->isaudio && now>=audio_read_next)){
					memset(pmp4_read_param,0,sizeof(webrtc_reader_mp4_param_t));
					pmp4_read_param->ptr = (uint8_t *)data;
					pmp4_read_param->bytes = databufsize;
					res = webrtc_mp4_reader_read(reader,pmp4_read_param);
					if(res>0){
						
						if(pmp4_read_param->object == WEBRTC_MP4_OBJECT_H264){
							 
							 video_read_next = now+(uint32_t)(pmp4_read_param->dts - v_dts);
						  	 v_pts = pmp4_read_param->pts;
						  	 v_dts = pmp4_read_param->dts;
							 webrtc_streamer_input_video_data(WEBRTC_STREAM_MAIN,WEBRTC_VIDEO_H264,(unsigned char *)pmp4_read_param->ptr,pmp4_read_param->bytes);
						}else if(pmp4_read_param->object == WEBRTC_MP4_OBJECT_OPUS){
				  		  	audio_read_next = now+(uint32_t)(pmp4_read_param->dts - a_dts);
						  	a_pts = pmp4_read_param->pts;
						  	a_dts = pmp4_read_param->dts;
							webrtc_streamer_input_audio_data((unsigned char *)pmp4_read_param->ptr,pmp4_read_param->bytes);
						  
						  
						}
					}else{
						video_read_next  = now;
					        audio_read_next  = now;
						startpos = 0;
						webrtc_mp4_reader_seek(reader,&startpos);
						//LOGW(" webrtc_mp4_reader_seek  startpos = %d\n",startpos);
					}
				}
				 rtos_delay_milliseconds(10);
				//rtos_get_semaphore(&event_sem, 5);

	  }
	  if(pvideo_info!= NULL){
		psram_free(pvideo_info);
		pvideo_info = NULL;
	  }
	  if(paudio_info!= NULL){
		psram_free(paudio_info);
		paudio_info = NULL;
	  }
	  if(pmp4_read_param!= NULL){
		psram_free(pmp4_read_param);
		pmp4_read_param = NULL;
	  }
	  if(data!= NULL){
		psram_free(data);
		data = NULL;
	  }
	  if(reader!= NULL){
			webrtc_mp4_reader_destroy(reader);
			reader = NULL;
	  }
	
	LOGW("play end\n");

#endif	 
}
#endif
#endif
#if CONFIG_SYS_CPU1
uint32_t webrtc_yuv422_to_jpg(int width, int height, unsigned char *inputYuv,unsigned char *outJpeg){
	return uyvy_to_jpg(width,height,inputYuv,outJpeg);
}
static void destory_record_data(record_data *data)
{
	   if(data){
		   if(data->data!= NULL){
			rtc_bk_free(data->data);
			data->data = NULL;
		   }
		   rtc_bk_free(data);
		   data = NULL;
	   }
}

static void destory_audio_data(audio_data *data)
{
	   if(data){
		   if(data->data!= NULL){
			rtc_bk_free(data->data);
			data->data = NULL;
		   }
		   rtc_bk_free(data);
		   data = NULL;
	   }
}
static void del_all_audio_data(){
	if(audio_data_list!= NULL){
		rtc_list_for_each(audio_data_list, (void (*)(void*))destory_audio_data);		
		rtc_list_free(audio_data_list);
		audio_data_list = NULL;
	}
}
static void del_all_record_data(){
	if(webrtc_record_queue_mutex!=NULL){
		rtos_lock_mutex(&webrtc_record_queue_mutex);
		if(record_data_list!= NULL){
			rtc_list_for_each(record_data_list, (void (*)(void*))destory_record_data);		
			rtc_list_free(record_data_list);
			record_data_list = NULL;
		}
		rtos_unlock_mutex(&webrtc_record_queue_mutex);
	}
}
static void destory_datachannel_message(datachannel_msg *msg)
{
	   if(msg){
		   if(msg->msg!= NULL){
			rtc_bk_free(msg->msg);
			msg->msg = NULL;
		   }
		   rtc_bk_free(msg);
		   msg = NULL;
	   }
}
static void del_all_datachannel_message(){
	if(dc_msg_list!= NULL){
		rtc_list_for_each(dc_msg_list, (void (*)(void*))destory_datachannel_message);		
		rtc_list_free(dc_msg_list);
		dc_msg_list = NULL;
	}
}

#endif
#if CONFIG_VFS

static int flash_format_lfs(void) {
	struct bk_little_fs_partition partition;
	bk_logic_partition_t *pt = bk_flash_partition_get_info(BK_PARTITION_USR_CONFIG);
	char *fs_name = NULL;

	int ret;

	fs_name = "littlefs";
	partition.part_type = LFS_FLASH;
	partition.part_flash.start_addr = pt->partition_start_addr;
	partition.part_flash.size = pt->partition_length;

	ret = bk_vfs_mkfs("PART_NONE", fs_name, &partition);
	
	return ret;
}

static int flash_mount_lfs(char *mount_point) {
	struct bk_little_fs_partition partition;
	bk_logic_partition_t *pt = bk_flash_partition_get_info(BK_PARTITION_USR_CONFIG);
	char *fs_name = NULL;

	int ret;

	fs_name = "littlefs";
	partition.part_type = LFS_FLASH;
	partition.part_flash.start_addr = pt->partition_start_addr;
	partition.part_flash.size = pt->partition_length;
	partition.mount_path = mount_point;

	ret = bk_vfs_mount("SOURCE_NONE", partition.mount_path, fs_name, 0, &partition);

	return ret;
}

static int flash_format_fatfs(void) {
	struct bk_fatfs_partition partition;
	char *fs_name = NULL;
	int ret;
	fs_name = "fatfs";
	partition.part_type = FATFS_DEVICE;
	partition.part_dev.device_name = FATFS_DEV_FLASH;
	ret = bk_vfs_mkfs(FATFS_DEV_FLASH, fs_name, &partition);	
	return ret;
}

static int flash_mount_fatfs(char *mount_point) {
	struct bk_fatfs_partition partition;
	char *fs_name = NULL;
	int ret;
	fs_name = "fatfs";
	partition.part_type = FATFS_DEVICE;
	partition.part_dev.device_name = FATFS_DEV_FLASH;
	partition.mount_path = mount_point;

	ret = bk_vfs_mount(FATFS_DEV_FLASH, partition.mount_path, fs_name, 0, &partition);

	return ret;
}
static int tfcard_format_fatfs(void) {
	struct bk_fatfs_partition partition;
	char *fs_name = NULL;

	int ret;

	fs_name = "fatfs";
	partition.part_type = FATFS_DEVICE;
	partition.part_dev.device_name = FATFS_DEV_SDCARD;

	ret = bk_vfs_mkfs(FATFS_DEV_SDCARD, fs_name, &partition);
	
	return ret;
}
static int umount_vfs(char *mount_point) {
	int ret;
	
	ret = bk_vfs_umount(mount_point);
	return ret;
}
static int tfcard_mount_fatfs(char *mount_point) {

	umount_vfs(mount_point);
	struct bk_fatfs_partition partition;
	char *fs_name = NULL;

	int ret;

	fs_name = "fatfs";
	partition.part_type = FATFS_DEVICE;
	partition.part_dev.device_name = FATFS_DEV_SDCARD;
	partition.mount_path = mount_point;

	ret = bk_vfs_mount(FATFS_DEV_SDCARD, partition.mount_path, fs_name, 0, &partition);
	return ret;
}




#endif

#if CONFIG_SYS_CPU0
#if CONFIG_WEBRTC_MDNS
int webrtc_mdns_gethostbyname(const char *host,int port, struct sockaddr_in *sa){
	
	int n = -1;
	char szport[16];
        snprintf(szport,sizeof(szport),"%d",port);
	struct addrinfo hints, *res, *rp;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if(port == -1){
		
		if((n = getaddrinfo(host, NULL, &hints, &res)) != 0){

		     return -1;
		}
	}else{
		
		if((n = getaddrinfo(host, szport, &hints, &res)) != 0){

		     return -1;
		}
	}

        for (rp = res; rp != NULL; rp = rp->ai_next) {
             if(rp->ai_family ==AF_INET){
		
		 struct sockaddr_in *s4 = (struct sockaddr_in *)rp->ai_addr;
                 os_memcpy(sa,s4,rp->ai_addrlen);
		 //webrtc_warning("webrtc_gethostbyname: %s ip: %s:%d", host,inet_ntoa(s4->sin_addr),htons(s4->sin_port));
		 n = 0;
                 break;
            }
        }

        if(res!= NULL){
		freeaddrinfo(res);
	}
	return n;
}
int webrtc_mdns_parse_hostname_to_addr(const char *server, struct sockaddr *addr, int *addrlen){


	int result = 0;
	int family = PF_INET;
	int port_int = 3478;
	char port[6];
	char host[128]={0};
	char *p1, *p2;
	if ((sscanf(server, "[%64[^]]]:%d", host, &port_int) == 2) || (sscanf(server, "[%64[^]]]", host) == 1)) {
		family = PF_INET6;
	} else {
		p1 = strchr(server, ':');
		p2 = strrchr(server, ':');
		if (p1 && p2 && (p1 != p2)) {
			family = PF_INET6;
			host[128-1]='\0';
			strncpy(host, server, sizeof(host) - 1);
		} else if (sscanf(server, "%[^:]:%d", host, &port_int) != 2) {
			host[128-1]='\0';
			strncpy(host, server, sizeof(host) - 1);
		}
	}
	snprintf(port, sizeof(port), "%d", port_int);
       


        struct sockaddr_in server_addr;
        bzero(&server_addr,sizeof(server_addr)); 
        server_addr.sin_family = family;
        server_addr.sin_port = htons(port_int);
  
	if (inet_aton(host, &server_addr.sin_addr) == 0){

			bzero(&server_addr,sizeof(server_addr));
			server_addr.sin_family=AF_INET;
			server_addr.sin_port=htons(port_int);
			if(webrtc_mdns_gethostbyname(host,port_int,&server_addr)!=0){
				LOGE("gethostbyname() error.  %s:%d",host,port_int);
				return -1;
			}
			result = 1;
	 }
        //LOGD("ice_parse_hostname_to_addr  %s address %s",server,inet_ntoa(server_addr.sin_addr));
        os_memcpy(addr,&server_addr,sizeof(server_addr));
        *addrlen=sizeof(server_addr);

	return result;
}
static void uninit_mdns(){
	if(g_pmdns!= NULL){
		rtc_bk_free(g_pmdns);
		g_pmdns = NULL;
	}
}

static void webrtc_streamer_mdns(beken_thread_arg_t p){
     PWebrtc_mdns pmdns = (PWebrtc_mdns)p;
     pmdns->mdns_thread_runing = true;  
#if 0
     unsigned char ttl = 1;
     unsigned char loopback = 0;
     unsigned int reuseaddr = 1;
    int sockfd=socket(PF_INET,SOCK_DGRAM,0);
    struct ip_mreq mreq;
    char buf[1024]={0};
   
    if(sockfd==-1){
        LOGE("socket error");
       
    }
                LOGE("%s %d sock = %d\n", __func__, __LINE__,sockfd);
    		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
#ifdef SO_REUSEPORT
    		setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuseaddr, sizeof(reuseaddr));
#endif
    		setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl));
    		setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*)&loopback, sizeof(loopback));
    LOGE("start \n");
    struct sockaddr_in addr,client_addr;
    memset(&addr,0,sizeof(addr));
    socklen_t addrLen = sizeof(client_addr);
    addr.sin_family=PF_INET;
    addr.sin_port=htons(5353);
    inet_aton("224.0.0.251",&addr.sin_addr);
    inet_aton("224.0.0.251",&mreq.imr_multiaddr);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if(setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(struct ip_mreq))==-1){
    LOGE("setsockopt error");
	
    }
    if(bind(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))==-1){
        LOGE("bind error");
       
    }
    sz=sizeof(addr);
    for(;;){
	addrLen = sizeof(client_addr);
        int ret = recvfrom(sockfd,buf,sizeof(buf),0,(struct sockaddr*)&client_addr,&addrLen);
	if(ret<0){
            LOGE("receive error\n");
            break;
        }
	LOGE("mdns  recv -------  %d from %s:%d\n",ret,inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
    }
    LOGE("end\n");
#endif
#if 1
	     struct timeval timeout;
	     size_t capacity = 1024;
	     char* buffer  = NULL;
	     char* outbuffer = NULL;    
	     char *service_name_buffer = NULL;
	     char *hostname_buffer = NULL; 
	     char *service_instance_buffer = NULL;
	     char *qualified_hostname_buffer = NULL;
	     uint32_t senddelay = get_cur_timestamp();
	     uint32_t now = senddelay;
	     LOGD("webrtc streamer mdns start timestamp = %u",get_cur_timestamp());
	    
	     pmdns->mdns_runing = true;
	     int ret = 0;
	     pmdns->mdns_thread_exit_ = false;
	     int isock;
	     int nfds =0;
	     int sockets[2];
	     int num_sockets = 0;
	     struct ip_mreq mreq[2];
	     struct sockaddr_in service_address_ipv4;
	     int in_addrlen = sizeof(service_address_ipv4);

	     
	     bzero(&service_address_ipv4,sizeof(service_address_ipv4)); 
	     service_address_ipv4.sin_family = PF_INET;
	     service_address_ipv4.sin_port = htons(3478);

	#ifdef WEBRTC_ENABLE_IPV6
	     struct sockaddr_in6 service_address_ipv6;
	#endif
	      if(strlen(pmdns->szlocal_addr_)>0){
			webrtc_mdns_parse_hostname_to_addr(pmdns->szlocal_addr_,(struct sockaddr *)&service_address_ipv4,&in_addrlen);
	      }else{

	      }
	      int i = 0;
	      bool error_exit = false;
	      size_t additional_count = 0;
	      webrtc_mdns_record_t additional[10];
	      for(i =0;i<10;i++){
		           os_memset((char*)&additional[i],0,sizeof(webrtc_mdns_record_t));
	      }
	      char addrbuffer[64] = {0};
	      fd_set readfs;
	      struct sockaddr remaddr;
	      
	      socklen_t addrlen = sizeof (remaddr);
	      LOGW("%s %d local address = %s\n", __func__, __LINE__,pmdns->szlocal_addr_);



	    
	   
	     buffer = rtc_bk_malloc(capacity);
	     if(buffer == NULL){
		goto exit_thread;
	     }
	     outbuffer = rtc_bk_malloc(capacity);
	     if(outbuffer == NULL){
		goto exit_thread;
	     }
	     service_name_buffer = (char *)rtc_bk_malloc(128);
	     hostname_buffer = (char *)rtc_bk_malloc(128);
	     service_instance_buffer = (char *)rtc_bk_malloc(256);
	     qualified_hostname_buffer = (char *)rtc_bk_malloc(256);
		if(service_name_buffer!= NULL){
		   sprintf(service_name_buffer,"_%s._tcp.local.",pmdns->szMdnsservername_);//_webrtc-webcam._tcp.local.
		}else{
			goto exit_thread;
		}
		if(hostname_buffer!= NULL){
			  sprintf(hostname_buffer,"%s",pmdns->szEquipmentId_);
		}else{
			goto exit_thread;
		}
		if(service_instance_buffer== NULL){
			goto exit_thread;
		}
		if(qualified_hostname_buffer== NULL){
			goto exit_thread;
		}
		int service_port = pmdns->web_http_port;

		webrtc_mdns_string_t service_string = (webrtc_mdns_string_t){service_name_buffer, strlen(service_name_buffer)};
		webrtc_mdns_string_t hostname_string = (webrtc_mdns_string_t){hostname_buffer, strlen(hostname_buffer)};

	      
		snprintf(service_instance_buffer, 256 - 1, "%.*s.%.*s",
			 MDNS_STRING_FORMAT(hostname_string), MDNS_STRING_FORMAT(service_string));
		webrtc_mdns_string_t service_instance_string =
		    (webrtc_mdns_string_t){service_instance_buffer, strlen(service_instance_buffer)};

	
		snprintf(qualified_hostname_buffer, 256 - 1, "%.*s.local.",
			 MDNS_STRING_FORMAT(hostname_string));
		webrtc_mdns_string_t hostname_qualified_string =
		    (webrtc_mdns_string_t){qualified_hostname_buffer, strlen(qualified_hostname_buffer)};



		pmdns->mdns_service_.service = service_string;
		pmdns->mdns_service_.hostname = hostname_string;
		pmdns->mdns_service_.service_instance = service_instance_string;
		pmdns->mdns_service_.hostname_qualified = hostname_qualified_string;
		pmdns->mdns_service_.address_ipv4 = service_address_ipv4;
	#ifdef WEBRTC_ENABLE_IPV6
		pmdns->mdns_service_.address_ipv6 = service_address_ipv6;
	#endif
		pmdns->mdns_service_.port = service_port;


		//LOGW("mdns service_string %.*s\n",MDNS_STRING_FORMAT(service_string));
		//LOGW("mdns hostname_string %.*s\n",MDNS_STRING_FORMAT(hostname_string));
		//LOGW("mdns service_instance_string %.*s\n",MDNS_STRING_FORMAT(service_instance_string));
		LOGD("mdns hostname %.*s\n",MDNS_STRING_FORMAT(hostname_qualified_string));

		// Setup our mDNS records

		// PTR record reverse mapping "<_service-name>._tcp.local." to
		// "<hostname>.<_service-name>._tcp.local."
		pmdns->mdns_service_.record_ptr = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.service,
			                             .type = MDNS_RECORDTYPE_PTR,
			                             .data.ptr.name = pmdns->mdns_service_.service_instance};
	

		// SRV record mapping "<hostname>.<_service-name>._tcp.local." to
		// "<hostname>.local." with port. Set weight & priority to 0.
		pmdns->mdns_service_.record_srv = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.service_instance,
			                             .type = MDNS_RECORDTYPE_SRV,
			                             .data.srv.name = pmdns->mdns_service_.hostname_qualified,
			                             .data.srv.port = pmdns->mdns_service_.port,
			                             .data.srv.priority = 0,
			                             .data.srv.weight = 0};
		

		// A/AAAA records mapping "<hostname>.local." to IPv4/IPv6 addresses
		pmdns->mdns_service_.record_a = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.hostname_qualified,
			                           .type = MDNS_RECORDTYPE_A,
			                           .data.a.addr = pmdns->mdns_service_.address_ipv4};
	#ifdef WEBRTC_ENABLE_IPV6
		pmdns->mdns_service_.record_aaaa = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.hostname_qualified,
			                              .type = MDNS_RECORDTYPE_AAAA,
			                              .data.aaaa.addr = pmdns->mdns_service_.address_ipv6};
	#endif

		// Add two test TXT records for our service instance name, will be coalesced into
		// one record with both key-value pair strings by the library
		pmdns->mdns_service_.txt_record[0] = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.service_instance,
			                                .type = MDNS_RECORDTYPE_TXT,
			                                .data.txt.key = {MDNS_STRING_CONST("serno")},
			                                .data.txt.value = {MDNS_STRING_CONST(pmdns->szRegNum_)}};
		pmdns->mdns_service_.txt_record[1] = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.service_instance,
			                                .type = MDNS_RECORDTYPE_TXT,
			                                .data.txt.key = {MDNS_STRING_CONST("name")},
			                                .data.txt.value = {MDNS_STRING_CONST(pmdns->szEquipmentName_)}};
		pmdns->mdns_service_.txt_record[2] = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.service_instance,
			                                .type = MDNS_RECORDTYPE_TXT,
			                                .data.txt.key = {MDNS_STRING_CONST("type")},
			                                .data.txt.value = {MDNS_STRING_CONST(pmdns->szEquipmentType_)}};
		pmdns->mdns_service_.txt_record[3] = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.service_instance,
			                                .type = MDNS_RECORDTYPE_TXT,
			                                .data.txt.key = {MDNS_STRING_CONST("http")},
			                                .data.txt.value = {MDNS_STRING_CONST(pmdns->szweb_http_port_)}};
		pmdns->mdns_service_.txt_record[4] = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.service_instance,
			                                .type = MDNS_RECORDTYPE_TXT,
			                                .data.txt.key = {MDNS_STRING_CONST("https")},
			                                .data.txt.value = {MDNS_STRING_CONST(pmdns->szweb_https_port_)}};
		pmdns->mdns_service_.txt_record[5] = (webrtc_mdns_record_t){.name = pmdns->mdns_service_.service_instance,
			                                .type = MDNS_RECORDTYPE_TXT,
			                                .data.txt.key = {MDNS_STRING_CONST("version")},
			                                .data.txt.value = {MDNS_STRING_CONST(pmdns->szversion_)}};

	
	     struct sockaddr_in sock_addr;
	     struct sockaddr_in client_sock_addr;
	     os_memset(&sock_addr, 0, sizeof(struct sockaddr_in));
	     sock_addr.sin_family = AF_INET;
	     sock_addr.sin_addr.s_addr = INADDR_ANY;
	     sock_addr.sin_port = htons(MDNS_PORT);

	     os_memset(&client_sock_addr, 0, sizeof(struct sockaddr_in));
	     client_sock_addr.sin_family = AF_INET;
	     client_sock_addr.sin_addr.s_addr =inet_addr(pmdns->szlocal_addr_); //INADDR_ANY;
	     client_sock_addr.sin_port = htons(0);

	     sockets[0] = webrtc_mdns_socket_open_ipv4(&client_sock_addr,&mreq[0]);
	     if (sockets[0] < 0){
			LOGE("%s %d \n", __func__, __LINE__);
			goto exit_thread;
	     }


	     sockets[1] = webrtc_mdns_socket_open_ipv4(&sock_addr,&mreq[1]);
	     if (sockets[1] < 0){
			LOGE("%s %d \n", __func__, __LINE__);
			goto exit_thread;
	     }
		num_sockets = 2;

		//LOGW("%s %d \n", __func__, __LINE__);
		additional[additional_count] = pmdns->mdns_service_.record_srv;
		additional_count++;

		if (pmdns->mdns_service_.address_ipv4.sin_family == AF_INET){
			additional[additional_count] = pmdns->mdns_service_.record_a;
			additional_count++;
		}
#ifdef WEBRTC_ENABLE_IPV6
		if (pmdns->mdns_service_.address_ipv6.sin6_family == AF_INET6){
			additional[additional_count] = pmdns->mdns_service_.record_aaaa;
			additional_count++;
		}
#endif
		if(pmdns->mdns_send_txt_== true){
			additional[additional_count] = pmdns->mdns_service_.txt_record[0];
			additional_count++;
			additional[additional_count] = pmdns->mdns_service_.txt_record[1];
			additional_count++;
			additional[additional_count] = pmdns->mdns_service_.txt_record[2];
			additional_count++;
			additional[additional_count] = pmdns->mdns_service_.txt_record[3];
			additional_count++;
			additional[additional_count] = pmdns->mdns_service_.txt_record[4];
			additional_count++;
			additional[additional_count] = pmdns->mdns_service_.txt_record[5];
			additional_count++;
		}


        if(sockets[1]!=-1){
                rtc_bk_memset(buffer,0,capacity);
		webrtc_mdns_announce_multicast(sockets[1], buffer, capacity, pmdns->mdns_service_.record_ptr, 0, 0,
			                        additional, additional_count);
	        senddelay = get_cur_timestamp();
	}
       pmdns->mdns_runing = true;
       while(pmdns->mdns_runing){
	
	        timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&readfs);

		for ( isock = 0; isock < num_sockets; isock++) {
			if (sockets[isock] >= nfds)
				nfds = sockets[isock] + 1;
			FD_SET(sockets[isock], &readfs);
		}
		ret = select(nfds, &readfs, NULL, NULL, &timeout);
		if(ret > 0) {
			
				for ( isock = 0; isock < num_sockets; isock++) {
					if (FD_ISSET(sockets[isock], &readfs)) {
						rtc_bk_memset(buffer,0,capacity);
						rtc_bk_memset(outbuffer,0,capacity);
						rtc_bk_memset(&remaddr,0,sizeof(struct sockaddr));
						addrlen = sizeof(struct sockaddr);
						ret = recvfrom(sockets[isock], buffer, capacity, 0, &remaddr, &addrlen);
						if(ret>0){
							if(strlen(pmdns->szlocal_addr_)>0){
								webrtc_mdns_parse_hostname_to_addr(pmdns->szlocal_addr_,(struct sockaddr *)&service_address_ipv4,&in_addrlen);
								pmdns->mdns_service_.address_ipv4 = service_address_ipv4;
								pmdns->mdns_service_.record_a.data.a.addr = service_address_ipv4;
	    						}
	     						webrtc_mdns_string_t addr = webrtc_ip_address_to_string(addrbuffer, sizeof(addrbuffer), (const struct sockaddr*)&remaddr,
				                                                                                               addrlen);
							//struct sockaddr_in *aaddr  = (struct sockaddr_in *)&remaddr;
							//struct in_addr inaddr  = aaddr->sin_addr;
							LOGD("mdns  isock = %d recv -------  %d from  %.*s\n",isock,ret,MDNS_STRING_FORMAT(addr));
							webrtc_mdns_handle(sockets[isock],&remaddr,addrlen,buffer,ret,outbuffer,capacity,pmdns);
						
						}else{
							//LOGE("%s %d \n", __func__, __LINE__);
						}
					}else{
						//LOGE("%s %d \n", __func__, __LINE__);
					}
					//FD_SET(sockets[isock], &readfs);
				}
				
			
		}else{
			if(ret == 0){

				       now = get_cur_timestamp();
				       if(now-senddelay>55000){
						senddelay = now;
						if(sockets[1]!=-1){
							rtc_bk_memset(buffer,0,capacity);
							webrtc_mdns_announce_multicast(sockets[1], buffer, capacity, pmdns->mdns_service_.record_ptr, 0, 0,
											additional, additional_count);
						}

				       }

				       continue;
			}else{
					error_exit = true;
					LOGW("webrtc_mdns_socket_listen  select error  %d\n",ret);
					break;
			}
		}
	}

	if(error_exit== false){
	    if(sockets[1]!=-1){
            webrtc_mdns_goodbye_multicast(sockets[1], buffer, capacity, pmdns->mdns_service_.record_ptr, 0, 0,
			                        additional, additional_count);
	    }
	}
       for ( isock = 0; isock < num_sockets; isock++) {
	       if(setsockopt(sockets[isock], IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq[isock], sizeof(mreq)) == -1) {
		  LOGE("%s %d \n", __func__, __LINE__);
	       }
       }



exit_thread:
	 for ( isock = 0; isock < num_sockets; isock++) {
	      if(sockets[isock]!= -1){
			webrtc_mdns_socket_close(sockets[isock]);
			sockets[isock] = -1; 
	       }
	}
       if(buffer!= NULL){
            rtc_bk_free(buffer);
	    buffer = NULL;
	}
        if(outbuffer!= NULL){
            rtc_bk_free(outbuffer);
	    outbuffer = NULL;
	}
	
       if(service_name_buffer!= NULL){
            rtc_bk_free(service_name_buffer);
	    service_name_buffer = NULL;
	}
       if(hostname_buffer!= NULL){
            rtc_bk_free(hostname_buffer);
	    hostname_buffer = NULL;
	}
       if(service_instance_buffer!= NULL){
            rtc_bk_free(service_instance_buffer);
	    service_instance_buffer = NULL;
	}
       if(qualified_hostname_buffer!= NULL){
            rtc_bk_free(qualified_hostname_buffer);
	    qualified_hostname_buffer = NULL;
	}

     
     LOGD("webrtc_streamer_mdns end\n");
     mdns_thread_hal = NULL;
     pmdns->mdns_thread_runing = false; 
     mdns_runing = false;
     uninit_mdns();
     if(webrtc_mdns_exit_sem!= NULL){
			int count = rtos_get_semaphore_count(&webrtc_mdns_exit_sem);
			if(count == 0){				
				rtos_set_semaphore(&webrtc_mdns_exit_sem);
			}
     }
#endif
     rtos_delete_thread(NULL);


}
static void init_mdns(){
	LOGD("%s %d \n", __func__, __LINE__);
        if(g_pmdns==NULL){
		g_pmdns = (Webrtc_mdns*)rtc_bk_malloc(sizeof(Webrtc_mdns));
		g_pmdns->querys_= NULL;
	}
	if(g_pmdns!= NULL){
		netif_ip4_config_t ip4_config;
		extern uint32_t uap_ip_is_start(void);
		os_memset(&ip4_config, 0x0, sizeof(netif_ip4_config_t));
		if (uap_ip_is_start()){
						bk_netif_get_ip4_config(NETIF_IF_AP, &ip4_config);
		}else{
						bk_netif_get_ip4_config(NETIF_IF_STA, &ip4_config);
		}
		snprintf(g_pmdns->szlocal_addr_,sizeof(g_pmdns->szlocal_addr_),"%s",ip4_config.ip);
		snprintf(g_pmdns->szRegNum_,sizeof(g_pmdns->szRegNum_),"%s",runSystemCfg.deviceInfo.serialNumber);
		snprintf(g_pmdns->szEquipmentName_,sizeof(g_pmdns->szEquipmentName_),"%s","webrtc_camera");
		snprintf(g_pmdns->szMdnsservername_,sizeof(g_pmdns->szMdnsservername_),"%s","webrtc-webcam");
		snprintf(g_pmdns->szEquipmentType_,sizeof(g_pmdns->szEquipmentType_),"%s","camera");
		snprintf(g_pmdns->szweb_http_port_,sizeof(g_pmdns->szweb_http_port_),"%d",80);
		snprintf(g_pmdns->szweb_https_port_,sizeof(g_pmdns->szweb_https_port_),"%d",443);
		snprintf(g_pmdns->szweb_local_addr_,sizeof(g_pmdns->szweb_local_addr_),"%s",ip4_config.ip);
		snprintf(g_pmdns->szversion_,sizeof(g_pmdns->szversion_),"%s","v.0.122");
    		webrtc_camera_random_uuid(g_pmdns->szEquipmentId_,37);

		g_pmdns->mdns_send_txt_ = true;
		g_pmdns->mdns_sock_ = -1;
		g_pmdns->mdns_runing = false;
		g_pmdns->mdns_thread_runing = false;
		g_pmdns->mdns_thread_exit_ = false;
                g_pmdns->web_https_port = 443;
         	g_pmdns->web_http_port = 80;
		
		

	}

}

static void start_mdns(){
        LOGW("%s %d \n", __func__, __LINE__);
	bk_err_t ret = BK_OK;
	init_mdns();
	if(g_pmdns!= NULL && mdns_thread_hal == NULL){
	mdns_runing = true;
	rtos_init_semaphore_ex(&webrtc_mdns_exit_sem, 1, 0);
	ret = rtos_create_psram_thread(&mdns_thread_hal,
						 5,
						 "mdns",
						 (beken_thread_function_t)webrtc_streamer_mdns,
						 16*1024,
						 g_pmdns);
	if (ret != kNoErr) {
		LOGE("create mdns  task fail \r\n");
		mdns_thread_hal = NULL;
		mdns_runing = false;
	}


	}
}
static void stop_mdns(){
	if(g_pmdns!= NULL && mdns_runing == true && g_pmdns->mdns_runing){
		g_pmdns->mdns_runing = false;
		if(webrtc_mdns_exit_sem!=NULL){
			rtos_get_semaphore(&webrtc_mdns_exit_sem, BEKEN_NEVER_TIMEOUT);
			rtos_deinit_semaphore(&webrtc_mdns_exit_sem);
			webrtc_mdns_exit_sem = NULL;

		}
	}
}
#endif
extern void *net_get_sta_handle(void);

static void webrtc_wifi_event_cb(void *new_evt)
{
	wifi_linkstate_reason_t info = *((wifi_linkstate_reason_t *)new_evt);
	switch (info.state)
	{
		case WIFI_LINKSTATE_STA_GOT_IP:
		{
			LOGW("WIFI_LINKSTATE_STA_GOT_IP\r\n");
			
			netif_ip4_config_t ip4_config;
			extern uint32_t uap_ip_is_start(void);

			os_memset(&ip4_config, 0x0, sizeof(netif_ip4_config_t));
			if (uap_ip_is_start()){
						bk_netif_get_ip4_config(NETIF_IF_AP, &ip4_config);
			}else{
						bk_netif_get_ip4_config(NETIF_IF_STA, &ip4_config);
			}
			wifi_connected = 1;
			if(cpu1_runing == true && wifi_conneced_sended == false){			  
			    if(event_sem!= NULL){
				int count = rtos_get_semaphore_count(&event_sem);
			   	if(count == 0){
		           		rtos_set_semaphore(&event_sem);
				}
	                    }  		   
			    
			}
	
			LOGW("ip: %s\n", ip4_config.ip);
		}
		break;

		case WIFI_LINKSTATE_STA_DISCONNECTED:
		{
			wifi_connected = 0;
			webrtc_mailbox_send_wifi_disconnect_msg();
			LOGW("WIFI_LINKSTATE_STA_DISCONNECTED\r\n");
		}
		break;

		case WIFI_LINKSTATE_AP_CONNECTED:
		{
			
			LOGW("WIFI_LINKSTATE_AP_CONNECTED\r\n");
		}
		break;

		case WIFI_LINKSTATE_AP_DISCONNECTED:
		{
			LOGW("WIFI_LINKSTATE_AP_DISCONNECTED\r\n");
		}
		break;

		default:
			LOGW("WIFI_LINKSTATE %d\r\n", info.state);
			break;

	}
}
void webrtc_wifi_sta_stop(){
     bk_wlan_status_register_cb(NULL);
     //bk_wlan_status_register_cb_internal(NULL);
     bk_wifi_sta_stop();
     g_wifi_started = false;
}
int webrtc_wifi_sta_connect(char *ssid, char *key)
{
	if(g_wifi_started== true){
		webrtc_wifi_sta_stop();
	}
	int len;

	bk_wlan_status_register_cb(webrtc_wifi_event_cb);
	//bk_wlan_status_register_cb_internal(webrtc_wifi_event_cb);

	wifi_sta_config_t sta_config = WIFI_DEFAULT_STA_CONFIG();

	len = os_strlen(key);

	if (32 < len)
	{
		LOGE("ssid name more than 32 Bytes\r\n");
		return BK_FAIL;
	}

	os_strcpy(sta_config.ssid, ssid);

	len = os_strlen(key);

	if (64 < len)
	{
		LOGE("key more than 64 Bytes\r\n");
		return BK_FAIL;
	}

	os_strcpy(sta_config.password, key);
	
	LOGE("ssid:%s key:%s\r\n", sta_config.ssid, sta_config.password);
	BK_LOG_ON_ERR(bk_wifi_sta_set_config(&sta_config));
	BK_LOG_ON_ERR(bk_wifi_sta_start());
        g_wifi_started = true;
	return BK_OK;
}
#endif
static void destory_mailbox_send_cmd_data(webrtc_cmd_t *cmd)
{
	   
	   if(cmd){
		  // LOGE("%s %d   %d    %p\n", __func__, __LINE__,cmd->msgid,cmd);
		   if(cmd->recvdata.data!= NULL){
			rtc_bk_free(cmd->recvdata.data);
			cmd->recvdata.data = NULL;
		   }
		   if(cmd->senddata.data!= NULL){
			rtc_bk_free(cmd->senddata.data);
			cmd->senddata.data = NULL;
		   }
		   if(cmd->sem!= NULL){
			rtos_deinit_semaphore(&cmd->sem);
			cmd->sem = NULL;
		   }
		   if(cmd->mutex!= NULL){
			rtos_deinit_mutex(&cmd->mutex);
			cmd->mutex = NULL;
		   }
	   }
	    //LOGE("%s %d \n", __func__, __LINE__);
}
void webrtc_set_mac_address(){
#if CONFIG_SYS_CPU0


#endif
}
void webrtc_set_default_system_info(){
#if CONFIG_SYS_CPU0
   // webrtc_erase_system_info();
#endif
}
void webrtc_capture(){
#if CONFIG_SYS_CPU1
	yuv_frame_capture = true;
 
#endif
}
void webrtc_ls_files(char *path){
#if CONFIG_SYS_CPU0
	webrtc_cpu0_list_files(path);
#endif
#if CONFIG_SYS_CPU1
	webrtc_cpu1_list_files(path);
#endif
}
void webrtc_vfs_statfs(char *path){
#if CONFIG_SYS_CPU0
	bk_err_t ret = BK_OK;
	struct statfs   fsinfo ={0};
	ret = bk_vfs_statfs(path,&fsinfo);
	if(ret == BK_OK){
		LOGW("%s %d f_bsize = %lu\n", __func__, __LINE__,fsinfo.f_bsize);
		LOGW("%s %d f_blocks = %lu\n", __func__, __LINE__,fsinfo.f_blocks);
		LOGW("%s %d f_bfree = %lu\n", __func__, __LINE__,fsinfo.f_bfree);
		LOGW("%s %d f_bavail = %lu\n", __func__, __LINE__,fsinfo.f_bavail);
	}else{
		LOGE("%s %d failed statfs err = %d\n", __func__, __LINE__,ret);
	}
#endif
}
void webrtc_vfs_del_file(char *path){
#if CONFIG_SYS_CPU0
	bk_err_t ret = BK_OK;
	ret = bk_vfs_unlink(path);
	if(ret == BK_OK){
		LOGW("%s %d delete file successed  %s\n", __func__, __LINE__,path);
	}else{
		LOGE("%s %d failed delete file err = %d\n", __func__, __LINE__,ret);
	}
#endif
}
void webrtc_vfs_mkdir(char *path){
#if CONFIG_SYS_CPU0
	bk_err_t ret = BK_OK;
	ret = bk_vfs_mkdir(path,S_IWUSR|S_IRUSR|S_IXUSR);
	if(ret == BK_OK){
		LOGW("%s %d mkdir successed %s\n", __func__, __LINE__,path);
	}else{
		LOGE("%s %d failed mkdir err = %d\n", __func__, __LINE__,ret);
	}
#endif
}
void webrtc_vfs_rmdir(char *path){
#if CONFIG_SYS_CPU0
	bk_err_t ret = BK_OK;
	ret = bk_vfs_rmdir(path);
	if(ret == BK_OK){
		LOGW("%s %d rmdir successed %s\n", __func__, __LINE__,path);
	}else{
		LOGE("%s %d failed rmdir err = %d\n", __func__, __LINE__,ret);
	}
#endif
}
void webrtc_vfs_format_sdcard(){
#if CONFIG_SYS_CPU0
	bk_err_t ret = BK_OK;
	ret = tfcard_format_fatfs();
	if(ret == BK_OK){
		LOGW("%s %d tfcard format  successed \n", __func__, __LINE__);
	}else{
		LOGE("%s %d failed tfcard format err = %d\n", __func__, __LINE__,ret);
	}
#endif
}
#if CONFIG_SYS_CPU0 
int checkMkdir(char *sPath)
{
	int iRet = 0;                         
	char sFilePath[256 + 1] = {0};              
	char sPathTmp[256 + 1] = {0};               
	char *pDir = NULL;

	struct stat stFileStat;


	os_memset(sFilePath, 0x00, sizeof(sFilePath));
	os_memset(sPathTmp, 0x00, sizeof(sPathTmp));
	os_memset(&stFileStat, 0x00, sizeof(stFileStat));

	os_memcpy(sFilePath, sPath, sizeof(sFilePath));

	
	pDir = strtok(sFilePath, "/");
	strcat(sPathTmp, "/");
	strcat(sPathTmp, pDir);
	strcat(sPathTmp, "/");

	os_memset(&stFileStat, 0x00, sizeof(stFileStat));
	bk_vfs_stat(sPathTmp, &stFileStat);

	if( !S_ISDIR(stFileStat.st_mode) )
	{
		iRet = bk_vfs_mkdir(sPathTmp, S_IWUSR|S_IRUSR|S_IXUSR);
		if( -1 == iRet )
		{
			LOGW("mkdir path [%s] error [%ld]\n", sPathTmp, iRet);
			return iRet;
		
		}	
	}

	while( NULL != ( pDir=strtok(NULL, "/") ) )
	{
		strcat(sPathTmp, pDir);
		strcat(sPathTmp, "/");

		os_memset(&stFileStat, 0x00, sizeof(stFileStat));
		bk_vfs_stat(sPathTmp, &stFileStat);

		if( !S_ISDIR(stFileStat.st_mode) )
		{
			iRet = bk_vfs_mkdir(sPathTmp, S_IWUSR|S_IRUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP);
			if( -1 == iRet )
			{
				LOGW("mkdir path [%s] error [%ld]\n", sPathTmp, iRet);
				return iRet;
		
			}
		}
	}

	return iRet;
}
#endif
#if CONFIG_SYS_CPU1 
int checkMkdir(char *sPath)
{

	int iRet = 0;                         
	char sFilePath[64] = {0};              
	char sPathTmp[64] = {0};               
	char *pDir = NULL;

	struct stat stFileStat;


	os_memset(sFilePath, 0x00, sizeof(sFilePath));
	os_memset(sPathTmp, 0x00, sizeof(sPathTmp));
	os_memset(&stFileStat, 0x00, sizeof(stFileStat));

	os_memcpy(sFilePath, sPath, sizeof(sFilePath));

	
	pDir = strtok(sFilePath, "/");
	strcat(sPathTmp, "/");
	strcat(sPathTmp, pDir);
	strcat(sPathTmp, "/");

	os_memset(&stFileStat, 0x00, sizeof(stFileStat));


	while( NULL != ( pDir=strtok(NULL, "/") ) )
	{
		strcat(sPathTmp, pDir);
		strcat(sPathTmp, "/");

		os_memset(&stFileStat, 0x00, sizeof(stFileStat));
		vfs_file_stat(sPathTmp, &stFileStat);

		if( !S_ISDIR(stFileStat.st_mode) )
		{
			iRet = vfs_file_mkdir(sPathTmp, S_IWUSR|S_IRUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP);
			if( -1 == iRet )
			{
				LOGW("mkdir path [%s] error [%ld]\n", sPathTmp, iRet);
				return iRet;
		
			}
		}
	}

	return iRet;

}
#endif
void webrtc_could_publish_stream_start(){
#if CONFIG_SYS_CPU0
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_COULD_PUBLISH_START,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		   	rtos_unlock_mutex(&pcmd->mutex);
	           	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
#endif
}
void webrtc_could_publish_stream_stop(){
#if CONFIG_SYS_CPU0
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_COULD_PUBLISH_STOP,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		   	rtos_unlock_mutex(&pcmd->mutex);
	           	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
#endif
}
void webrtc_test(void){
#if CONFIG_SYS_CPU0
    
#endif
/*
#if CONFIG_SYS_CPU0	
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_TEST,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		   	rtos_unlock_mutex(&pcmd->mutex);
	           	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
	
#endif
*/
}
void webrtc_reboot(void){
#if CONFIG_SYS_CPU0
	system_reboot = true;
	runhua_runing = false;

#endif
}
void webrtc_mailbox_send_start_record_msg(void){
#if CONFIG_SYS_CPU0
	
	//webrtc_cmd_t *pcmd = (webrtc_cmd_t*)rtc_bk_malloc(sizeof(webrtc_cmd_t));
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_START_RECORD,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		   	rtos_unlock_mutex(&pcmd->mutex);
	           	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
#endif
}
void webrtc_mailbox_send_stop_record_msg(void){
#if CONFIG_SYS_CPU0
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res = webrtc_mailbox_send_media_req_msg(APP_COM_STOP_RECORD,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		    	rtos_unlock_mutex(&pcmd->mutex);
	            	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}	
#endif
}
void webrtc_mailbox_send_stop_cpu1_msg(void){
     // LOGW("%s %d  \n", __func__, __LINE__);
#if CONFIG_SYS_CPU0
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){

	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_STOP_CPU1,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}	
#endif
}
#if CONFIG_SYS_CPU0
void webrtc_mailbox_sdcard_mount(int mounted){
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    pcmd->param.nparam1 = mounted;
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_SDCARD_MOUNT,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
}
void webrtc_mailbox_send_wifi_connect_msg(void){
       //LOGD("%s %d  \n", __func__, __LINE__);
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_WIFI_CONNECT,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}	
}
void webrtc_mailbox_send_wifi_disconnect_msg(void){
     // LOGW("%s %d  \n", __func__, __LINE__);
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){

	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_WIFI_DISCONNECT,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}	

}
void webrtc_mailbox_send_wifi_config_msg(void){
     // LOGW("%s %d  \n", __func__, __LINE__);
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){

	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    pcmd->param.param1 = (u32)&runNetworkCfg;
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_WIFI_CONFIG,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}	

}

#endif
void webrtc_connect_msg(char *ssid, char *key){
#if CONFIG_SYS_CPU0
	 //webrtc_wifi_sta_connect(ssid,key);
	runNetworkCfg.wireless.isConfig = 0;
	snprintf((char*)runNetworkCfg.wireless.essid,32,"%s",ssid);
	snprintf((char*)runNetworkCfg.wireless.passd,32,"%s",key);
	NetworkCfgSave();
#endif
}
#if CONFIG_SYS_CPU1
void webrtc_mailbox_cpu1_sdcard_mount(){
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_CPU1_SDCARD_MOUNT,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
}
void webrtc_mailbox_cpu1_sdcard_format(){
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_FORMAT_SDCORD,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
}
void webrtc_mailbox_cpu1_send_ptz_ctrl(int direction_x,int direction_y){
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    pcmd->param.nparam1 = direction_x;
	    pcmd->param.nparam2 = direction_y;
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_PTZ_CTRL,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
}
void webrtc_mailbox_send_cpu1_start_msg(void){
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_CPU1_START,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
}
void webrtc_mailbox_send_wifi_config_ok_msg(void){
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_send_media_req_msg(APP_COM_WIFI_CONFIG_OK,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
	       if(pcmd->responseed == 0){
		 pcmd->isWaited = 1;
		 rtos_unlock_mutex(&pcmd->mutex);
	         rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
	       }else{
		rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
}
#endif
#if (CONFIG_SYS_CPU1 || CONFIG_SYS_CPU2)
static void webrtc_mailbox_cpu1_cpu2_rx_isr(void *param, mb_chnl_cmd_t *cmd_buf){

     bk_err_t ret = BK_OK;
     webrtc_cmd_t *pcmd = (webrtc_cmd_t *)cmd_buf->param2;
     if(pcmd){
		queue_msg_t msg;
		msg.param = (void*)pcmd;
		if(cmd_buf->hdr.cmd == MESSAGE_HEAD_COM_RESP){
			if (mailbox_cpu1_cpu2_msg_queue_resp){
				ret = rtos_push_to_queue(&mailbox_cpu1_cpu2_msg_queue_resp, &msg, BEKEN_NO_WAIT);
				if (BK_OK != ret){
					LOGE("%s failed\n", __func__);
				}
			}
		
			if(mailbox_cpu1_cpu2_resp_sem!= NULL){
				int count = rtos_get_semaphore_count(&mailbox_cpu1_cpu2_resp_sem);
			   	if(count == 0){
					rtos_set_semaphore(&mailbox_cpu1_cpu2_resp_sem);
				}

			}
		}else {
			if (mailbox_cpu1_cpu2_msg_queue_req){
				ret = rtos_push_to_queue(&mailbox_cpu1_cpu2_msg_queue_req, &msg, BEKEN_NO_WAIT);
				if (BK_OK != ret)
				{
					LOGE("%s failed\n", __func__);
				}
			}
		
			if(mailbox_cpu1_cpu2_req_sem!= NULL){
				int count = rtos_get_semaphore_count(&mailbox_cpu1_cpu2_req_sem);
			   	if(count == 0){
					rtos_set_semaphore(&mailbox_cpu1_cpu2_req_sem);
				}

			}

		}


   }
}
static void webrtc_mailbox_cpu1_cpu2_tx_isr(void *param)
{
 // LOGW("%s %d param = %p\n", __func__, __LINE__,param);
}
static void webrtc_mailbox_cpu1_cpu2_tx_cmpl_isr(void *param, mb_chnl_ack_t *ack_buf)
{
  //LOGW("%s %d cmd = 0x%x %d %d\n", __func__, __LINE__,ack_buf->hdr.cmd,ack_buf->ack_data1,ack_buf->ack_data2);
   //LOGW("%s %d cmd = %d state = %d  Reserved = %d param = %p\n", __func__, __LINE__,ack_buf->hdr.cmd,ack_buf->hdr.state,ack_buf->hdr.Reserved,param);
}
int webrtc_mailbox_cpu1_cpu2_send_media_req_msg(u32 param1,u32 param2,u32 param3)
{
      //LOGE("%s --------- %d \n", __func__, __LINE__);
#if (CONFIG_SYS_CPU1)
	if(mailbox_cpu1_cpu2_send_msg_mutex && param2!= 0){
		rtos_lock_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		webrtc_cmd_t *pcmd = (webrtc_cmd_t *)param2;
		u8 chstatus =0;	
		int delaytime = 0;
		g_mailbox_msg_id++;
		bk_err_t ret = BK_OK;
		mb_chnl_cmd_t mb_cmd;
		mb_cmd.hdr.cmd = MESSAGE_HEAD_COM_REQ;
		mb_cmd.param1 = param1;
		mb_cmd.param2 = param2;
		mb_cmd.param3 = param3;
		if(pcmd!= NULL){
		    pcmd->mb_cmd.param1 = param1;
		    pcmd->mb_cmd.param2 = param2;
		    pcmd->mb_cmd.param3 = param3;
		    pcmd->hdrcmd = MESSAGE_HEAD_COM_REQ;
		    pcmd->txstate = 0;
		    pcmd->responseed = 0;
		    pcmd->isWaited = 0;			
		    pcmd->msgid = MESSAGE_HEAD_ID;  
		}
checkbusy:
		mb_chnl_ctrl(CP2_MB_CHNL_MEDIA,MB_CHNL_GET_STATUS,&chstatus);
		if(chstatus == 1){
			LOGE("%s %d mailbox  is busy\n", __func__, __LINE__);
			rtos_delay_milliseconds(1);
			delaytime++;
			if(delaytime<3){
			goto checkbusy;
			}
			//ret = BK_ERR_BUSY;
		}
		ret = mb_chnl_write(CP2_MB_CHNL_MEDIA, &mb_cmd);
		if (ret != BK_OK){		
				LOGE("%s %d FAILED  %d\n", __func__, __LINE__,ret);
		}
		
		rtos_unlock_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		return ret;
	}
#endif
	return BK_OK;
}
int webrtc_mailbox_cpu1_cpu2_send_media_response_msg(u32 param1,u32 param2,u32 param3){
#if (CONFIG_SYS_CPU1)
	if(mailbox_cpu1_cpu2_send_msg_mutex && param2!= 0){
		rtos_lock_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		u8 chstatus =0;	
		int delaytime = 0;
		bk_err_t ret = BK_OK;
		mb_chnl_cmd_t mb_cmd;
		mb_cmd.hdr.cmd = MESSAGE_HEAD_COM_RESP;
		mb_cmd.param1 = param1;
		mb_cmd.param2 = param2;
		mb_cmd.param3 = param3;
		webrtc_cmd_t *pcmd = (webrtc_cmd_t *)param2;
		if(pcmd!= NULL){
			pcmd->txstate = 0;
			pcmd->hdrcmd = MESSAGE_HEAD_COM_RESP;
		}
checkbusy:
		mb_chnl_ctrl(CP2_MB_CHNL_MEDIA,MB_CHNL_GET_STATUS,&chstatus);
	        if(chstatus == 1){
			LOGE("%s %d mailbox  is busy\n", __func__, __LINE__);
			rtos_delay_milliseconds(1);
			delaytime++;
			if(delaytime<3){
			   goto checkbusy;
			}
			//ret = BK_ERR_BUSY;
		}
		ret = mb_chnl_write(CP2_MB_CHNL_MEDIA, &mb_cmd);
		if (ret != BK_OK){
		
			LOGE("%s %d FAILED %d\n", __func__, __LINE__,ret);
		}
		
		rtos_unlock_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		return ret;
	}
#endif
    return BK_OK;
}
int webrtc_mailbox_cpu2_cpu1_send_media_req_msg(u32 param1,u32 param2,u32 param3)
{
#if (CONFIG_SYS_CPU2)
	if(mailbox_cpu1_cpu2_send_msg_mutex && param2!= 0){
		rtos_lock_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		webrtc_cmd_t *pcmd = (webrtc_cmd_t *)param2;
		u8 chstatus =0;	
		int delaytime = 0;
		g_mailbox_msg_id++;
		bk_err_t ret = BK_OK;
		mb_chnl_cmd_t mb_cmd;
		mb_cmd.hdr.cmd = MESSAGE_HEAD_COM_REQ;
		mb_cmd.param1 = param1;
		mb_cmd.param2 = param2;
		mb_cmd.param3 = param3;
		if(pcmd!= NULL){
		    pcmd->mb_cmd.param1 = param1;
		    pcmd->mb_cmd.param2 = param2;
		    pcmd->mb_cmd.param3 = param3;
		    pcmd->hdrcmd = MESSAGE_HEAD_COM_REQ;
		    pcmd->txstate = 0;
		    pcmd->responseed = 0;
		    pcmd->isWaited = 0;			
		    pcmd->msgid = MESSAGE_HEAD_ID;  
		}
checkbusy:
		mb_chnl_ctrl(CP1_MB_CHNL_MEDIA,MB_CHNL_GET_STATUS,&chstatus);
		if(chstatus == 1){
			LOGE("%s %d mailbox  is busy\n", __func__, __LINE__);
			rtos_delay_milliseconds(1);
			delaytime++;
			if(delaytime<3){
			goto checkbusy;
			}
			//ret = BK_ERR_BUSY;
		}
		ret = mb_chnl_write(CP1_MB_CHNL_MEDIA, &mb_cmd);
		if (ret != BK_OK){		
				LOGE("%s %d FAILED  %d\n", __func__, __LINE__,ret);
		}
		
		rtos_unlock_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		return ret;
	}
#endif
	return BK_OK;
}
int webrtc_mailbox_cpu2_cpu1_send_media_response_msg(u32 param1,u32 param2,u32 param3){
#if (CONFIG_SYS_CPU2)
	if(mailbox_cpu1_cpu2_send_msg_mutex && param2!= 0){
		rtos_lock_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		u8 chstatus =0;	
		int delaytime = 0;
		bk_err_t ret = BK_OK;
		mb_chnl_cmd_t mb_cmd;
		mb_cmd.hdr.cmd = MESSAGE_HEAD_COM_RESP;
		mb_cmd.param1 = param1;
		mb_cmd.param2 = param2;
		mb_cmd.param3 = param3;
		webrtc_cmd_t *pcmd = (webrtc_cmd_t *)param2;
		if(pcmd!= NULL){
			pcmd->txstate = 0;
			pcmd->hdrcmd = MESSAGE_HEAD_COM_RESP;
		}
checkbusy:
		mb_chnl_ctrl(CP1_MB_CHNL_MEDIA,MB_CHNL_GET_STATUS,&chstatus);
	        if(chstatus == 1){
			LOGE("%s %d mailbox  is busy\n", __func__, __LINE__);
			rtos_delay_milliseconds(1);
			delaytime++;
			if(delaytime<3){
			   goto checkbusy;
			}
			//ret = BK_ERR_BUSY;
		}
		ret = mb_chnl_write(CP1_MB_CHNL_MEDIA, &mb_cmd);
		if (ret != BK_OK){
		
			LOGE("%s %d FAILED %d\n", __func__, __LINE__,ret);
		}
		
		rtos_unlock_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		return ret;
	}
#endif
    return BK_OK;
}
#endif
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
static void webrtc_mailbox_cpu0_cpu1_rx_isr(void *param, mb_chnl_cmd_t *cmd_buf)
{
     bk_err_t ret = BK_OK;
     webrtc_cmd_t *pcmd = (webrtc_cmd_t *)cmd_buf->param2;
     if(pcmd){
		queue_msg_t msg;
		msg.param = (void*)pcmd;
		if(cmd_buf->hdr.cmd == MESSAGE_HEAD_COM_RESP){
			if (mailbox_cpu0_cpu1_msg_queue_resp){
				ret = rtos_push_to_queue(&mailbox_cpu0_cpu1_msg_queue_resp, &msg, BEKEN_NO_WAIT);
				if (BK_OK != ret)
				{
					LOGE("%s failed\n", __func__);
				}
			}
		
			if(mailbox_cpu0_cpu1_resp_sem!= NULL){
				int count = rtos_get_semaphore_count(&mailbox_cpu0_cpu1_resp_sem);
			   	if(count == 0){
					rtos_set_semaphore(&mailbox_cpu0_cpu1_resp_sem);
				}

			}
		}else {
			if (mailbox_cpu0_cpu1_msg_queue_req){
				ret = rtos_push_to_queue(&mailbox_cpu0_cpu1_msg_queue_req, &msg, BEKEN_NO_WAIT);
				if (BK_OK != ret)
				{
					LOGE("%s failed\n", __func__);
				}
			}
		
			if(mailbox_cpu0_cpu1_req_sem!= NULL){
				int count = rtos_get_semaphore_count(&mailbox_cpu0_cpu1_req_sem);
			   	if(count == 0){
					rtos_set_semaphore(&mailbox_cpu0_cpu1_req_sem);
				}

			}

		}


   }

   //LOGE("%s %d \n", __func__, __LINE__);
}
static void webrtc_mailbox_cpu0_cpu1_tx_isr(void *param)
{
 // LOGW("%s %d param = %p\n", __func__, __LINE__,param);
}
static void webrtc_mailbox_cpu0_cpu1_tx_cmpl_isr(void *param, mb_chnl_ack_t *ack_buf)
{
  //LOGW("%s %d cmd = 0x%x %d %d\n", __func__, __LINE__,ack_buf->hdr.cmd,ack_buf->ack_data1,ack_buf->ack_data2);
   //LOGW("%s %d cmd = %d state = %d  Reserved = %d param = %p\n", __func__, __LINE__,ack_buf->hdr.cmd,ack_buf->hdr.state,ack_buf->hdr.Reserved,param);
}
int webrtc_mailbox_send_media_req_msg(u32 param1,u32 param2,u32 param3)
{
      //LOGE("%s --------- %d \n", __func__, __LINE__);
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
	if(mailbox_cpu0_cpu1_send_msg_mutex && param2!= 0){
		rtos_lock_mutex(&mailbox_cpu0_cpu1_send_msg_mutex);
		webrtc_cmd_t *pcmd = (webrtc_cmd_t *)param2;
		u8 chstatus =0;	
		int delaytime = 0;
		g_mailbox_msg_id++;
		bk_err_t ret = BK_OK;
		mb_chnl_cmd_t mb_cmd;
		mb_cmd.hdr.cmd = MESSAGE_HEAD_COM_REQ;
		mb_cmd.param1 = param1;
		mb_cmd.param2 = param2;
		mb_cmd.param3 = param3;
		if(pcmd!= NULL){
		    pcmd->mb_cmd.param1 = param1;
		    pcmd->mb_cmd.param2 = param2;
		    pcmd->mb_cmd.param3 = param3;
		    pcmd->hdrcmd = MESSAGE_HEAD_COM_REQ;
		    pcmd->txstate = 0;
		    pcmd->responseed = 0;
		    pcmd->isWaited = 0;			
		    pcmd->msgid = MESSAGE_HEAD_ID;  
		}
checkbusy:
		mb_chnl_ctrl(MB_CHNL_MEDIA,MB_CHNL_GET_STATUS,&chstatus);
		if(chstatus == 1){
			LOGE("%s %d mailbox  is busy\n", __func__, __LINE__);
			rtos_delay_milliseconds(1);
			delaytime++;
			if(delaytime<3){
			goto checkbusy;
			}
			//ret = BK_ERR_BUSY;
		}
		ret = mb_chnl_write(MB_CHNL_MEDIA, &mb_cmd);
		if (ret != BK_OK){		
				LOGE("%s %d FAILED  %d\n", __func__, __LINE__,ret);
		}
		
		rtos_unlock_mutex(&mailbox_cpu0_cpu1_send_msg_mutex);
		return ret;
	}
#endif
	return BK_OK;
}

int webrtc_mailbox_send_media_response_msg(u32 param1,u32 param2,u32 param3){
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
	if(mailbox_cpu0_cpu1_send_msg_mutex && param2!= 0){
		rtos_lock_mutex(&mailbox_cpu0_cpu1_send_msg_mutex);
		u8 chstatus =0;	
		int delaytime = 0;
		bk_err_t ret = BK_OK;
		mb_chnl_cmd_t mb_cmd;
		mb_cmd.hdr.cmd = MESSAGE_HEAD_COM_RESP;
		mb_cmd.param1 = param1;
		mb_cmd.param2 = param2;
		mb_cmd.param3 = param3;
		webrtc_cmd_t *pcmd = (webrtc_cmd_t *)param2;
		if(pcmd!= NULL){
			pcmd->txstate = 0;
			pcmd->hdrcmd = MESSAGE_HEAD_COM_RESP;
		}
checkbusy:
		mb_chnl_ctrl(MB_CHNL_MEDIA,MB_CHNL_GET_STATUS,&chstatus);
	        if(chstatus == 1){
			LOGE("%s %d mailbox  is busy\n", __func__, __LINE__);
			rtos_delay_milliseconds(1);
			delaytime++;
			if(delaytime<3){
			   goto checkbusy;
			}
			//ret = BK_ERR_BUSY;
		}
		ret = mb_chnl_write(MB_CHNL_MEDIA, &mb_cmd);
		if (ret != BK_OK){
		
			LOGE("%s %d FAILED %d\n", __func__, __LINE__,ret);
		}
		
		rtos_unlock_mutex(&mailbox_cpu0_cpu1_send_msg_mutex);
		return ret;
	}
#endif
    return BK_OK;
}
static void dealwith_mailbox_cpu0_cpu1_req_msg(webrtc_cmd_t *pcmd)
{
     //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
     
     if(pcmd->mb_cmd.param1>=SOCKET_COM_MIN && pcmd->mb_cmd.param1<SOCKET_COM_MAX){
		socket_handle_req_rx(pcmd);
     }else if(pcmd->mb_cmd.param1>=VFS_FILE_COM_MIN && pcmd->mb_cmd.param1<VFS_FILE_COM_MAX){
		vfs_file_handle_req_rx(pcmd);
     }else if(pcmd->mb_cmd.param1>=APP_COM_MIN && pcmd->mb_cmd.param1<APP_COM_MAX){
		webrtc_handle_req_rx(pcmd);
     }else{
    }
	
#endif
}

static void dealwith_mailbox_cpu0_cpu1_resp_msg(webrtc_cmd_t *pcmd)
{
     //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
     
     if(pcmd->mb_cmd.param1>=SOCKET_COM_MIN && pcmd->mb_cmd.param1<SOCKET_COM_MAX){
		socket_handle_resp_rx(pcmd);
     }else if(pcmd->mb_cmd.param1>=VFS_FILE_COM_MIN && pcmd->mb_cmd.param1<VFS_FILE_COM_MAX){
		vfs_file_handle_resp_rx(pcmd);

     }else if(pcmd->mb_cmd.param1>=APP_COM_MIN && pcmd->mb_cmd.param1<APP_COM_MAX){
		webrtc_handle_resp_rx(pcmd);
     }else{
    }
	
#endif
}
void mailbox_cpu0_cpu1_req_data_handle(){
	bk_err_t ret = BK_OK;
	webrtc_cmd_t *pcmd = NULL;
	queue_msg_t msg;
	while(!rtos_is_queue_empty(&mailbox_cpu0_cpu1_msg_queue_req))
		{
			ret = rtos_pop_from_queue(&mailbox_cpu0_cpu1_msg_queue_req, &msg, 0);
			if (kNoErr == ret)
			{
				pcmd = (webrtc_cmd_t *)msg.param;
				if(pcmd!= NULL){
					dealwith_mailbox_cpu0_cpu1_req_msg(pcmd);
									
			        }
			}
			else
			{
				break;
			}
		}
}
void mailbox_cpu0_cpu1_resp_data_handle(){
	bk_err_t ret = BK_OK;
	webrtc_cmd_t *pcmd = NULL;
	queue_msg_t msg;
	while(!rtos_is_queue_empty(&mailbox_cpu0_cpu1_msg_queue_resp))
		{
			ret = rtos_pop_from_queue(&mailbox_cpu0_cpu1_msg_queue_resp, &msg, 0);
			if (kNoErr == ret)
			{
				pcmd = (webrtc_cmd_t *)msg.param;
				if(pcmd!= NULL){
					dealwith_mailbox_cpu0_cpu1_resp_msg(pcmd);				
			        }
			}
			else
			{
				break;
			}
		}
}

void mailbox_cpu0_cpu1_req_thread(void *param){
	//bk_err_t ret = BK_OK;
	mailbox_thread_runing = true;
	while(mailbox_thread_runing){
		rtos_get_semaphore(&mailbox_cpu0_cpu1_req_sem, BEKEN_WAIT_FOREVER);
		if(mailbox_cpu0_cpu1_msg_queue_req!= NULL){
			 mailbox_cpu0_cpu1_req_data_handle();
		}	
	}

	if(mailbox_cpu0_cpu1_req_sem!= NULL){
		rtos_deinit_semaphore(&mailbox_cpu0_cpu1_req_sem);
		mailbox_cpu0_cpu1_req_sem = NULL;
	}
	if(mailbox_cpu0_cpu1_send_msg_mutex!= NULL){
		rtos_deinit_mutex(&mailbox_cpu0_cpu1_send_msg_mutex);
		mailbox_cpu0_cpu1_send_msg_mutex =NULL;
	}
	if (mailbox_cpu0_cpu1_msg_queue_req)
	{
		rtos_deinit_queue(&mailbox_cpu0_cpu1_msg_queue_req);
		mailbox_cpu0_cpu1_msg_queue_req = NULL;
	}

	mailbox_cpu0_cpu1_req_thread_hdl = NULL;
	rtos_delete_thread(NULL);

}

void mailbox_cpu0_cpu1_resp_thread(void *param){
	//bk_err_t ret = BK_OK;
	mailbox_thread_runing = true;
	while(mailbox_thread_runing){
		rtos_get_semaphore(&mailbox_cpu0_cpu1_resp_sem, BEKEN_WAIT_FOREVER);
		if(mailbox_cpu0_cpu1_msg_queue_resp!= NULL){
			 mailbox_cpu0_cpu1_resp_data_handle();
		}	
	}
	if(mailbox_cpu0_cpu1_resp_sem!= NULL){
		rtos_deinit_semaphore(&mailbox_cpu0_cpu1_resp_sem);
		mailbox_cpu0_cpu1_resp_sem = NULL;
	}	
	if (mailbox_cpu0_cpu1_msg_queue_resp!= NULL)
	{
		rtos_deinit_queue(&mailbox_cpu0_cpu1_msg_queue_resp);
		mailbox_cpu0_cpu1_msg_queue_resp = NULL;
	}

	mailbox_cpu0_cpu1_resp_thread_hdl = NULL;
	rtos_delete_thread(NULL);

}
#endif
#if (CONFIG_SYS_CPU1 || CONFIG_SYS_CPU2)
void webrtc_handle_cpu1_cpu2_req_rx(webrtc_cmd_t *pcmd){
#if (CONFIG_SYS_CPU2)
	//LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
#endif
	if(pcmd->mb_cmd.param1 == CPU12_COM_START){
#if (CONFIG_SYS_CPU1)
		cpu2_runing = true;
		LOGW("%s %d cmd = 0x%x ---\n", __func__, __LINE__,pcmd->mb_cmd.param1);
#endif
	}else if(pcmd->mb_cmd.param1 == CPU12_COM_MOTION_DETECTION){
#if (CONFIG_SYS_CPU1)
		uint32_t now = get_cur_timestamp();
		int motion_detection = (int)pcmd->param.param1;
			if(motion_detection>0){
				LOGI("%s %d motion value = %d \n", __func__, __LINE__,motion_detection);
			}
			
		       if(motion_detection_enable){
			       if(motion_detectioning == false){
				      
				       if(motion_detection>10 && webrtc_streamer_current_session_count()==0){
						if(now-motion_detection_starttime>motion_detection_delay){
							if(motion_detection_record == true){

								if( webrtc_recording == false && sdcard_mounted ==1 && motion_detection_recording == false){
								       motion_detection_starttime = now;
									motion_detectioning = true;
									webrtc_recording = true;
									webrtc_record_event = 1;
									webrtc_can_record = true;
									motion_detection_recording = true;
									LOGW("%s %d motion = %d \n", __func__, __LINE__,motion_detection);
									if(motion_detection_alarm && webrtc_streamer_online && webrtc_streamer_current_session_count()==0){
										char sztime[64]={0};
										time_timestr_get_ex(sztime,64);
										cJSON *root = NULL;
										root = cJSON_CreateObject();
										if(root!= NULL){

											cJSON *resource  = cJSON_CreateObject();
											cJSON_AddStringToObject(resource,"small_url","https://cdn.pixabay.com/photo/2017/03/12/11/30/alishan-2136879_1280.jpg");
											cJSON_AddStringToObject(resource,"type","IMAGE");
											cJSON_AddStringToObject(resource,"url","https://api.newaylink.com/logo.png");
											cJSON_AddItemToObject(root,"resource",resource);
											cJSON_AddStringToObject(root,"platform","all");
											cJSON_AddNumberToObject(root,"type",24);
											cJSON_AddStringToObject(root,"title","Motion");
											cJSON_AddStringToObject(root,"detail","Motion Alarm");
											//cJSON_AddStringToObject(root,"when",sztime);
											char *pwbuf = cJSON_Print(root);
											if(pwbuf!=NULL){
											   LOGW("%s %d motion alarm = %s \n", __func__, __LINE__,sztime);
											   webrtc_streamer_publish_message(pwbuf,strlen(pwbuf));
											   os_free(pwbuf);
											   pwbuf = NULL;
											}
											cJSON_Delete(root);
										}

									}
								}
							

							}else{
								motion_detection_starttime = now;
								motion_detectioning = true;
								LOGW("%s %d motion = %d \n", __func__, __LINE__,motion_detection);
								if(motion_detection_alarm && webrtc_streamer_online && webrtc_streamer_current_session_count()==0){
									char sztime[64]={0};
									time_timestr_get_ex(sztime,64);
									cJSON *root = NULL;
									root = cJSON_CreateObject();
									if(root!= NULL){

										cJSON *resource  = cJSON_CreateObject();
										cJSON_AddStringToObject(resource,"small_url","https://cdn.pixabay.com/photo/2017/03/12/11/30/alishan-2136879_1280.jpg");
										cJSON_AddStringToObject(resource,"type","IMAGE");
										cJSON_AddStringToObject(resource,"url","https://api.newaylink.com/logo.png");
										cJSON_AddItemToObject(root,"resource",resource);
										cJSON_AddStringToObject(root,"platform","all");
										cJSON_AddNumberToObject(root,"type",24);
										cJSON_AddStringToObject(root,"title","Motion");
										cJSON_AddStringToObject(root,"detail","Motion Alarm");
										//cJSON_AddStringToObject(root,"when",sztime);
										char *pwbuf = cJSON_Print(root);
										if(pwbuf!=NULL){
										   LOGW("%s %d motion alarm = %s \n", __func__, __LINE__,sztime);
										   webrtc_streamer_publish_message(pwbuf,strlen(pwbuf));
										   os_free(pwbuf);
										   pwbuf = NULL;
										}
										cJSON_Delete(root);
									}

								}
							}
						}

				       }
			       }else{
					if(now-motion_detection_starttime>motion_detection_delay){
						LOGW("%s %d stop motion  \n", __func__, __LINE__);
						
						if(motion_detection_recording == true){
							//LOGW("%s %d stop motion  --%d\n", __func__, __LINE__,webrtc_recording);
							if(webrtc_recording == true){
								
								webrtc_recording = false;
								motion_detectioning = false;
								if(webrtc_record_sem!= NULL){
										int count = rtos_get_semaphore_count(&webrtc_record_sem);
										if(count == 0){				
											    		rtos_set_semaphore(&webrtc_record_sem);
										}
								}
								
							}else{
								
							}				
						}else{
							motion_detectioning = false;
						}
					}
			       }
		       }
/*
		Rectangle* rectangles = (Rectangle*)pcmd->param.param2;
		for (int i = 0; i < count; i++) {
                               LOGW("%s %d Rectangle %d: x=%d, y=%d, width=%d, height=%d\n", __func__, __LINE__, i, rectangles[i].x, rectangles[i].y, rectangles[i].width, rectangles[i].height);
               }
*/
#endif
	}else if(pcmd->mb_cmd.param1 == CPU12_COM_MOTION_RESET){
#if (CONFIG_SYS_CPU2)
		webrtc_motion_reset();
#endif
	}else if(pcmd->mb_cmd.param1 == CPU12_COM_CAPTURE){
#if (CONFIG_SYS_CPU2)
	    LOGW("%s %d cmd = 0x%x ---\n", __func__, __LINE__,pcmd->mb_cmd.param1);
            int i,j; 
	    uint16_t destwidth;
	    uint16_t destheight;
	    uint32_t destsize; 
	    uint8_t *dest = NULL;
	    uint8_t *stc = NULL;
	    uint8_t *frame = (uint8_t *)pcmd->param.param1;
	    uint32_t size = (uint32_t)pcmd->param.param2;
	    uint16_t width = (uint16_t)pcmd->param.param3;
	    uint16_t height = (uint16_t)pcmd->param.param4;
	    uint16_t sequence = (uint16_t)pcmd->param.param5;
	    int fmt = pcmd->param.nparam1;
	    int framedelay = pcmd->param.nparam2;
	   if(size>0 && frame!= NULL && width>0 && height>0 && fmt==5){
		    if(width == 640 && height == 480){
			     destwidth = width/2;
			     destheight = height/2;
			     destsize = destwidth*destheight;
			     uint8_t *yframe = rtc_bk_malloc(destsize);
		    	     if(yframe!= NULL){
				dest = yframe;
				stc = frame;
				for(i = 0;i<destheight;i++){
					for(j = 0;j<destwidth;j++){
						stc= frame+((2*(i*(2*width)+(2*j)))+1);
						*dest = *stc;
						dest++;

					}
				}
				webrtc_motion_input_frame(yframe,destsize,destwidth,destheight,sequence,framedelay);
				if(event_sem!= NULL){
					   int count = rtos_get_semaphore_count(&event_sem);
					   if(count == 0){
					   	rtos_set_semaphore(&event_sem);
					   }
				}
				rtc_bk_free(yframe);
				yframe = NULL;				
			     }

		   }else if(width == 1080 && height == 720){
			     destwidth = width/3;
			     destheight = height/3;
			     destsize = destwidth*destheight;
			     uint8_t *yframe = rtc_bk_malloc(destsize);
		    	     if(yframe!= NULL){
				dest = yframe;
				stc = frame;
				for(i = 0;i<destheight;i++){
					for(j = 0;j<destwidth;j++){
						stc= frame+(3*(i*(2*width)+(2*j)));
						*dest = *stc;
						dest++;

					}
				}
				webrtc_motion_input_frame(yframe,destsize,destwidth,destheight,sequence,framedelay);
				if(event_sem!= NULL){
					   int count = rtos_get_semaphore_count(&event_sem);
					   if(count == 0){
					   	rtos_set_semaphore(&event_sem);
					   }
				}
				rtc_bk_free(yframe);
				yframe = NULL;				
			     }
		   }
	   }

#endif
	}

#if (CONFIG_SYS_CPU1)
	webrtc_mailbox_cpu1_cpu2_send_media_response_msg(pcmd->mb_cmd.param1,pcmd->mb_cmd.param2,pcmd->mb_cmd.param3);
#endif
#if (CONFIG_SYS_CPU2)
	webrtc_mailbox_cpu2_cpu1_send_media_response_msg(pcmd->mb_cmd.param1,pcmd->mb_cmd.param2,pcmd->mb_cmd.param3);
#endif
}
void webrtc_handle_cpu1_cpu2_resp_rx(webrtc_cmd_t *pcmd){
	 //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
		if(pcmd!= NULL && pcmd->sem!= NULL && pcmd->mutex!=NULL){
			while(pcmd->isWaited ==0){
				rtos_delay_milliseconds(1);
			}
			rtos_lock_mutex(&pcmd->mutex);
			pcmd->responseed = 1;
			rtos_unlock_mutex(&pcmd->mutex);
			if(pcmd->isWaited==1){
				int count = rtos_get_semaphore_count(&pcmd->sem);
				if(count == 0){				
			    		rtos_set_semaphore(&pcmd->sem);
				}
			}
		}

}

static void dealwith_mailbox_cpu1_cpu2_req_msg(webrtc_cmd_t *pcmd)
{
     //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
     if(pcmd->mb_cmd.param1>=CPU12_COM_BASE && pcmd->mb_cmd.param1<CPU12_COM_MAX){
		webrtc_handle_cpu1_cpu2_req_rx(pcmd);
     }


}

static void dealwith_mailbox_cpu1_cpu2_resp_msg(webrtc_cmd_t *pcmd)
{
     //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
     if(pcmd->mb_cmd.param1>=CPU12_COM_BASE && pcmd->mb_cmd.param1<CPU12_COM_MAX){
	     webrtc_handle_cpu1_cpu2_resp_rx(pcmd);
     }

}
void mailbox_cpu1_cpu2_req_data_handle(){
	bk_err_t ret = BK_OK;
	webrtc_cmd_t *pcmd = NULL;
	queue_msg_t msg;
	while(!rtos_is_queue_empty(&mailbox_cpu1_cpu2_msg_queue_req))
		{
			ret = rtos_pop_from_queue(&mailbox_cpu1_cpu2_msg_queue_req, &msg, 0);
			if (kNoErr == ret)
			{
				pcmd = (webrtc_cmd_t *)msg.param;
				if(pcmd!= NULL){
					dealwith_mailbox_cpu1_cpu2_req_msg(pcmd);
									
			        }
			}
			else
			{
				break;
			}
		}
}
void mailbox_cpu1_cpu2_resp_data_handle(){
	bk_err_t ret = BK_OK;
	webrtc_cmd_t *pcmd = NULL;
	queue_msg_t msg;
	while(!rtos_is_queue_empty(&mailbox_cpu1_cpu2_msg_queue_resp))
		{
			ret = rtos_pop_from_queue(&mailbox_cpu1_cpu2_msg_queue_resp, &msg, 0);
			if (kNoErr == ret)
			{
				pcmd = (webrtc_cmd_t *)msg.param;
				if(pcmd!= NULL){
					dealwith_mailbox_cpu1_cpu2_resp_msg(pcmd);				
			        }
			}
			else
			{
				break;
			}
		}
}

void mailbox_cpu1_cpu2_req_thread(void *param){
	mailbox_thread_runing = true;
	while(mailbox_thread_runing){
		rtos_get_semaphore(&mailbox_cpu1_cpu2_req_sem, BEKEN_WAIT_FOREVER);
		if(mailbox_cpu1_cpu2_msg_queue_req!= NULL){
			 mailbox_cpu1_cpu2_req_data_handle();
		}	
	}

	if(mailbox_cpu1_cpu2_req_sem!= NULL){
		rtos_deinit_semaphore(&mailbox_cpu1_cpu2_req_sem);
		mailbox_cpu1_cpu2_req_sem = NULL;
	}
	if(mailbox_cpu1_cpu2_send_msg_mutex!= NULL){
		rtos_deinit_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
		mailbox_cpu1_cpu2_send_msg_mutex =NULL;
	}
	if (mailbox_cpu1_cpu2_msg_queue_req)
	{
		rtos_deinit_queue(&mailbox_cpu1_cpu2_msg_queue_req);
		mailbox_cpu1_cpu2_msg_queue_req = NULL;
	}

	mailbox_cpu1_cpu2_req_thread_hdl = NULL;
	rtos_delete_thread(NULL);
}
void mailbox_cpu1_cpu2_resp_thread(void *param){

	mailbox_thread_runing = true;
	while(mailbox_thread_runing){
		rtos_get_semaphore(&mailbox_cpu1_cpu2_resp_sem, BEKEN_WAIT_FOREVER);
		if(mailbox_cpu1_cpu2_msg_queue_resp!= NULL){
			 mailbox_cpu1_cpu2_resp_data_handle();
		}	
	}
	if(mailbox_cpu1_cpu2_resp_sem!= NULL){
		rtos_deinit_semaphore(&mailbox_cpu1_cpu2_resp_sem);
		mailbox_cpu1_cpu2_resp_sem = NULL;
	}	
	if (mailbox_cpu1_cpu2_msg_queue_resp!= NULL)
	{
		rtos_deinit_queue(&mailbox_cpu1_cpu2_msg_queue_resp);
		mailbox_cpu1_cpu2_msg_queue_resp = NULL;
	}

	mailbox_cpu1_cpu2_resp_thread_hdl = NULL;
	rtos_delete_thread(NULL);

}
#if CONFIG_SYS_CPU1

void webrtc_cpu1_yuv_frame_capture(frame_buffer_t *frame,int skip){
        //LOGW("##:  frame fmt:%d, length:%d, frame_addr:%p   (%d * %d)\r\n",frame->fmt,frame->size, frame->frame,frame->width,frame->height);
#ifdef  USE_CPU2
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    pcmd->param.param1 = (u32)frame->frame;
	    pcmd->param.param2 = (u32)frame->size;
	    pcmd->param.param3 = (u32)frame->width;
	    pcmd->param.param4 = (u32)frame->height;
	    pcmd->param.param5 = (u32)frame->sequence;
	    pcmd->param.nparam1 = frame->fmt;
	    pcmd->param.nparam2 = skip;
	    
	    int res =  webrtc_mailbox_cpu1_cpu2_send_media_req_msg(CPU12_COM_CAPTURE,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		   	rtos_unlock_mutex(&pcmd->mutex);
	           	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
#else  
#endif 
}
void webrtc_cpu1_cpu2_motion_reset(){
#ifdef  USE_CPU2
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_cpu1_cpu2_send_media_req_msg(CPU12_COM_MOTION_RESET,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		   	rtos_unlock_mutex(&pcmd->mutex);
	           	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
#else  
#endif 
}
#endif
#if CONFIG_SYS_CPU2
void webrtc_cpu2_start(){

	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    int res =  webrtc_mailbox_cpu2_cpu1_send_media_req_msg(CPU12_COM_START,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		   	rtos_unlock_mutex(&pcmd->mutex);
	           	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}

}
void webrtc_cpu2_motion_detection(int count,void*rectangles){
	webrtc_cmd_t webrtc_cmd = {0};
	webrtc_cmd_t *pcmd = &webrtc_cmd;
	if(pcmd){
	    rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
	    rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
	    rtos_init_mutex(&pcmd->mutex);
	    pcmd->param.param1 = (u32)count;
	    pcmd->param.param2 = (u32)rectangles;
	    int res =  webrtc_mailbox_cpu2_cpu1_send_media_req_msg(CPU12_COM_MOTION_DETECTION,(u32)pcmd,0);
	    if(res  == BK_OK){
		rtos_lock_mutex(&pcmd->mutex);
		if(pcmd->responseed == 0){
			pcmd->isWaited = 1;
		   	rtos_unlock_mutex(&pcmd->mutex);
	           	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
		}else{
			rtos_unlock_mutex(&pcmd->mutex);
		}
	    }
	    destory_mailbox_send_cmd_data(pcmd);
	}
}
#endif
#endif
bk_err_t webrtc_mailbox_init(void){
	LOGD("%s %d \n", __func__, __LINE__);
	g_mailbox_msg_id = 0;
	mailbox_thread_runing = true;
	bk_err_t ret = BK_OK;
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
	rtos_init_mutex(&mailbox_cpu0_cpu1_send_msg_mutex);
	rtos_init_semaphore_ex(&mailbox_cpu0_cpu1_req_sem, 1, 0);
	rtos_init_semaphore_ex(&mailbox_cpu0_cpu1_resp_sem, 1, 0);

	ret = rtos_init_queue(&mailbox_cpu0_cpu1_msg_queue_req,"mailbox_msg_queue_req",sizeof(queue_msg_t),128);
	if (ret != BK_OK)
	{
		LOGE("create mailbox_msg_queue fail\n");
	}
	ret = rtos_init_queue(&mailbox_cpu0_cpu1_msg_queue_resp,"mailbox_msg_queue_resp",sizeof(queue_msg_t),128);
	if (ret != BK_OK)
	{
		LOGE("create mailbox_msg_queue fail\n");
	}
#endif
#if (CONFIG_SYS_CPU1 || CONFIG_SYS_CPU2)
#ifdef USE_CPU2
	rtos_init_mutex(&mailbox_cpu1_cpu2_send_msg_mutex);
	rtos_init_semaphore_ex(&mailbox_cpu1_cpu2_req_sem, 1, 0);
	rtos_init_semaphore_ex(&mailbox_cpu1_cpu2_resp_sem, 1, 0);

	ret = rtos_init_queue(&mailbox_cpu1_cpu2_msg_queue_req,"mailbox_msg_cpu1_cpu2_queue_req",sizeof(queue_msg_t),128);
	if (ret != BK_OK)
	{
		LOGE("create mailbox_msg_queue fail\n");
	}
	ret = rtos_init_queue(&mailbox_cpu1_cpu2_msg_queue_resp,"mailbox_msg_cpu1_cpu2_queue_resp",sizeof(queue_msg_t),128);
	if (ret != BK_OK)
	{
		LOGE("create mailbox_msg_queue fail\n");
	}
#endif
#endif
#if CONFIG_SYS_CPU0
	ret = rtos_create_psram_thread(&mailbox_cpu0_cpu1_req_thread_hdl,
						 5,
						 "mailbox_req0",
						 (beken_thread_function_t)mailbox_cpu0_cpu1_req_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox req task fail \r\n");
		mailbox_cpu0_cpu1_req_thread_hdl = NULL;
	}else{
		
	}
	ret = rtos_create_psram_thread(&mailbox_cpu0_cpu1_resp_thread_hdl,
						 5,
						 "mailbox_resp0",
						 (beken_thread_function_t)mailbox_cpu0_cpu1_resp_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox resp task fail \r\n");
		mailbox_cpu0_cpu1_resp_thread_hdl = NULL;
	}else{
		
	}


#endif
#if CONFIG_SYS_CPU1
	ret = rtos_create_psram_thread(&mailbox_cpu0_cpu1_req_thread_hdl,
						 5,
						 "mailbox_req1",
						 (beken_thread_function_t)mailbox_cpu0_cpu1_req_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox req task fail \r\n");
		mailbox_cpu0_cpu1_req_thread_hdl = NULL;
	}else{
		
	}
	ret = rtos_create_psram_thread(&mailbox_cpu0_cpu1_resp_thread_hdl,
						 5,
						 "mailbox_resp1",
						 (beken_thread_function_t)mailbox_cpu0_cpu1_resp_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox_resp_thread_hdl resp task fail \r\n");
		mailbox_cpu0_cpu1_resp_thread_hdl = NULL;
	}else{
		
	}
#ifdef USE_CPU2
	ret = rtos_create_psram_thread(&mailbox_cpu1_cpu2_req_thread_hdl,
						 5,
						 "mailbox_req12",
						 (beken_thread_function_t)mailbox_cpu1_cpu2_req_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox req task task fail \r\n");
		mailbox_cpu1_cpu2_req_thread_hdl = NULL;
	}else{
		
	}
	ret = rtos_create_psram_thread(&mailbox_cpu1_cpu2_resp_thread_hdl,
						 5,
						 "mailbox_resp12",
						 (beken_thread_function_t)mailbox_cpu1_cpu2_resp_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox resp task fail \r\n");
		mailbox_cpu1_cpu2_resp_thread_hdl = NULL;
	}else{
		
	}
#endif
#endif
#ifdef USE_CPU2
#if CONFIG_SYS_CPU2
	ret = rtos_create_psram_thread(&mailbox_cpu1_cpu2_req_thread_hdl,
						 5,
						 "mailbox_req21",
						 (beken_thread_function_t)mailbox_cpu1_cpu2_req_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox req task task fail \r\n");
		mailbox_cpu1_cpu2_req_thread_hdl = NULL;
	}else{
		
	}
	ret = rtos_create_psram_thread(&mailbox_cpu1_cpu2_resp_thread_hdl,
						 5,
						 "mailbox_resp21",
						 (beken_thread_function_t)mailbox_cpu1_cpu2_resp_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox resp task fail \r\n");
		mailbox_cpu1_cpu2_resp_thread_hdl = NULL;
	}else{
		
	}
#endif
#endif
	//LOGW("%s %d \n", __func__, __LINE__);
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)	
	project_config.mailbox_send_media_req_msg = webrtc_mailbox_send_media_req_msg; 
	project_config.mailbox_send_media_response_msg = webrtc_mailbox_send_media_response_msg;                             
	mb_chnl_open(MB_CHNL_MEDIA, &project_config);
	mb_chnl_ctrl(MB_CHNL_MEDIA, MB_CHNL_SET_RX_ISR, webrtc_mailbox_cpu0_cpu1_rx_isr);
	mb_chnl_ctrl(MB_CHNL_MEDIA, MB_CHNL_SET_TX_ISR, webrtc_mailbox_cpu0_cpu1_tx_isr);
	mb_chnl_ctrl(MB_CHNL_MEDIA, MB_CHNL_SET_TX_CMPL_ISR, webrtc_mailbox_cpu0_cpu1_tx_cmpl_isr);
#endif
#ifdef USE_CPU2
#if (CONFIG_SYS_CPU1)
	project_config_cpu12.mailbox_send_media_req_msg = webrtc_mailbox_cpu1_cpu2_send_media_req_msg; 
	project_config_cpu12.mailbox_send_media_response_msg = webrtc_mailbox_cpu1_cpu2_send_media_response_msg;                             
	mb_chnl_open(CP2_MB_CHNL_MEDIA, &project_config_cpu12);
	mb_chnl_ctrl(CP2_MB_CHNL_MEDIA, MB_CHNL_SET_RX_ISR, webrtc_mailbox_cpu1_cpu2_rx_isr);
	mb_chnl_ctrl(CP2_MB_CHNL_MEDIA, MB_CHNL_SET_TX_ISR, webrtc_mailbox_cpu1_cpu2_tx_isr);
	mb_chnl_ctrl(CP2_MB_CHNL_MEDIA, MB_CHNL_SET_TX_CMPL_ISR, webrtc_mailbox_cpu1_cpu2_tx_cmpl_isr);
#endif
#if (CONFIG_SYS_CPU2)
	project_config_cpu12.mailbox_send_media_req_msg = webrtc_mailbox_cpu2_cpu1_send_media_req_msg; 
	project_config_cpu12.mailbox_send_media_response_msg = webrtc_mailbox_cpu2_cpu1_send_media_response_msg;                             
	mb_chnl_open(CP1_MB_CHNL_MEDIA, &project_config_cpu12);
	mb_chnl_ctrl(CP1_MB_CHNL_MEDIA, MB_CHNL_SET_RX_ISR, webrtc_mailbox_cpu1_cpu2_rx_isr);
	mb_chnl_ctrl(CP1_MB_CHNL_MEDIA, MB_CHNL_SET_TX_ISR, webrtc_mailbox_cpu1_cpu2_tx_isr);
	mb_chnl_ctrl(CP1_MB_CHNL_MEDIA, MB_CHNL_SET_TX_CMPL_ISR, webrtc_mailbox_cpu1_cpu2_tx_cmpl_isr);
#endif
#endif
	LOGD("%s %d  end \n", __func__, __LINE__);
	return ret;

}
bk_err_t webrtc_mailbox_deinit(void){
	mailbox_thread_runing = false;
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
        mb_chnl_close(MB_CHNL_MEDIA);			
	if(mailbox_cpu0_cpu1_req_sem!= NULL){
		int count = rtos_get_semaphore_count(&mailbox_cpu0_cpu1_req_sem);
		if(count == 0){
	   		rtos_set_semaphore(&mailbox_cpu0_cpu1_req_sem);
		}
	}
	if(mailbox_cpu0_cpu1_resp_sem!= NULL){
	   	int count = rtos_get_semaphore_count(&mailbox_cpu0_cpu1_resp_sem);
		if(count == 0){
	   		rtos_set_semaphore(&mailbox_cpu0_cpu1_resp_sem);
		}
	}

#endif
#ifdef USE_CPU2
#if (CONFIG_SYS_CPU1)
        mb_chnl_close(CP2_MB_CHNL_MEDIA);			
	if(mailbox_cpu1_cpu2_req_sem!= NULL){
		int count = rtos_get_semaphore_count(&mailbox_cpu1_cpu2_req_sem);
		if(count == 0){
	   		rtos_set_semaphore(&mailbox_cpu1_cpu2_req_sem);
		}
	}
	if(mailbox_cpu1_cpu2_resp_sem!= NULL){
	   	int count = rtos_get_semaphore_count(&mailbox_cpu1_cpu2_resp_sem);
		if(count == 0){
	   		rtos_set_semaphore(&mailbox_cpu1_cpu2_resp_sem);
		}
	}
#endif
#if (CONFIG_SYS_CPU2)
        mb_chnl_close(CP1_MB_CHNL_MEDIA);			
	if(mailbox_cpu1_cpu2_req_sem!= NULL){
		int count = rtos_get_semaphore_count(&mailbox_cpu1_cpu2_req_sem);
		if(count == 0){
	   		rtos_set_semaphore(&mailbox_cpu1_cpu2_req_sem);
		}
	}
	if(mailbox_cpu1_cpu2_resp_sem!= NULL){
	   	int count = rtos_get_semaphore_count(&mailbox_cpu1_cpu2_resp_sem);
		if(count == 0){
	   		rtos_set_semaphore(&mailbox_cpu1_cpu2_resp_sem);
		}
	}
#endif
#endif
	return BK_OK;

}
void webrtc_dealwith_init(void){
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
	init_sockets(&project_config);
	init_vfs_files(&project_config);
#endif
}
void webrtc_dealwith_uninit(void){
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
	uninit_sockets();
	uninit_vfs_files();
#endif
}

#if CONFIG_SYS_CPU1
static void webrtc_dealwith_audio_data(audio_data *data){

	bk_err_t ret = BK_OK;
	if(audio_opened == true && runhua_runing && webrtc_audio_playing_file == false){
		// LOGE("%s %d   len = %d\n", __func__, __LINE__,data->size);
	         ret = bk_aud_intf_write_spk_data((uint8_t *)data->data, (uint32_t)data->size);
		if (ret != BK_OK) {
			LOGE("write spk data fail \r\n");
		}
	}

}


int app_media_add_record_queue(uint8_t *data, unsigned int len,bool isvideo){
	int result = 0;
	if(webrtc_recording == true  && webrtc_record_queue_mutex!= NULL){
			       rtos_lock_mutex(&webrtc_record_queue_mutex);
			       if(webrtc_can_record == true){
				       if(record_data_list== NULL || rtc_list_size(record_data_list)<8){
					       record_data *padata = (record_data *)rtc_bk_malloc(sizeof(record_data));
					       if(padata!= NULL){
								rtc_bk_memset(padata,0,sizeof(record_data));
								padata->data = (char*)rtc_bk_malloc(len+1);
								if(padata->data!= NULL){
									rtc_bk_memcpy(padata->data,data,len);
									padata->size = len;
									padata->isvideo = isvideo;
									record_data_list = rtc_list_append(record_data_list, (void*)padata);						

								}else{
									rtc_bk_free(padata);
									padata = NULL;
								}
				

					      }
				      }else{
					webrtc_record_skip = true;
					//LOGW("%s %d record list size = %d\n", __func__, __LINE__,rtc_list_size(record_data_list));
				      }
				}
			      rtos_unlock_mutex(&webrtc_record_queue_mutex); 

		}
	return result;
}
void app_media_record_queue_set_event(){
		if(webrtc_record_sem!= NULL){
			int count = rtos_get_semaphore_count(&webrtc_record_sem);
			if(count == 0){
				rtos_set_semaphore(&webrtc_record_sem);
			}

		}

}
int app_media_read_audio_frame(uint8_t *data, unsigned int len){
    //LOGW("app_media_read_audio_frame %d  \r\n",len,wifi_connected,runhua_runing);
    if(runhua_runing){
		if(webrtc_streamer_runing){
         	   webrtc_streamer_input_audio_data((unsigned char *)data,len);
		}
		if(webrtc_recording == true){
		      app_media_add_record_queue(data,len,false);
		      app_media_record_queue_set_event();
		}


    }
    return len;
}
void webrtc_stream_start_audio_stop(void){
		LOGI("-----------webrtc_stream_start_audio_stop-----------------  \n");
		if(audio_opened == true){
			bk_err_t ret = BK_OK;
			ret = bk_aud_intf_voc_stop();
			if (ret != BK_ERR_AUD_INTF_OK) {
				LOGE("bk_aud_intf_voc_stop fail, ret:%d \r\n", ret);
			} else {
				LOGI("bk_aud_intf_voc_stop complete \r\n");
			}

			/* deinit vioce */
			ret = bk_aud_intf_voc_deinit();
			if (ret != BK_ERR_AUD_INTF_OK) {
				LOGE("bk_aud_intf_voc_deinit fail, ret:%d \r\n", ret);
			} else {
				LOGI("bk_aud_intf_voc_deinit complete \r\n");
			}

			/* deinit audio */
			aud_work_mode = AUD_INTF_WORK_MODE_NULL;
			bk_aud_intf_set_mode(aud_work_mode);

			ret = bk_aud_intf_drv_deinit();
			if (ret != BK_ERR_AUD_INTF_OK) {
				LOGE("bk_aud_intf_drv_deinit fail, ret:%d \n", ret);
			} else {
				LOGI("bk_aud_intf_drv_deinit complete \n");
			}
			audio_opened = false;
		}
}
void webrtc_stream_start_audio_start(void) {
		LOGW("-----------webrtc_stream_start_audio_start-----------------  \n");  
#if 1
		if(audio_opened == false){
			bk_err_t ret = BK_OK;
			aud_intf_drv_setup.aud_intf_tx_mic_data = app_media_read_audio_frame;
			ret = bk_aud_intf_drv_init(&aud_intf_drv_setup);
			if (ret != BK_ERR_AUD_INTF_OK) {
				LOGE("bk_aud_intf_drv_init fail, ret:0x%x  \n", ret);
			} else {
				LOGI("bk_aud_intf_drv_init complete \n");
			}

			aud_work_mode = AUD_INTF_WORK_MODE_VOICE;
			ret = bk_aud_intf_set_mode(aud_work_mode);
			if (ret != BK_ERR_AUD_INTF_OK) {
			
				LOGE("bk_aud_intf_set_mode fail, ret:0x%x \n", ret);
				webrtc_stream_start_audio_stop();
		
			} else {
				LOGI("bk_aud_intf_set_mode complete \r\n");
			}
/*
			aud_intf_spk_setup.spk_chl = AUD_INTF_MIC_CHL_MIC1;
			aud_intf_spk_setup.samp_rate = audio_samp_rate;
			aud_intf_spk_setup.frame_size = 320;
			aud_intf_spk_setup.spk_gain = 0x2d;
			ret = bk_aud_intf_spk_init(&aud_intf_spk_setup);
			if (ret != BK_ERR_AUD_INTF_OK) {
				LOGE("bk_aud_intf_spk_init fail, ret:0x%x \n", ret);
			} else {
				LOGE("bk_aud_intf_spk_init complete \n");
			}

*/

			aud_intf_voc_setup.data_type  = AUD_INTF_VOC_DATA_TYPE_PCM;
			aud_intf_voc_setup.spk_mode   = AUD_DAC_WORK_MODE_DIFFEN;
			aud_intf_voc_setup.aec_enable = aec_enable;
			aud_intf_voc_setup.samp_rate = audio_samp_rate;
			aud_intf_voc_setup.spk_gain = 0x2d;
			aud_intf_voc_setup.mic_gain = 0x30;
			if (audio_type == 1) {
				aud_intf_voc_setup.mic_type = AUD_INTF_MIC_TYPE_UAC;
				aud_intf_voc_setup.spk_type = AUD_INTF_MIC_TYPE_UAC;
			} else {
				aud_intf_voc_setup.mic_type = AUD_INTF_MIC_TYPE_BOARD;
				aud_intf_voc_setup.spk_type = AUD_INTF_MIC_TYPE_BOARD;
			}

			ret = bk_aud_intf_voc_init(aud_intf_voc_setup);
			if (ret != BK_ERR_AUD_INTF_OK) {
			
				LOGE("bk_aud_intf_voc_init fail, ret:0x%x \n", ret);
				webrtc_stream_start_audio_stop();
				return;
			} else {
				LOGW("bk_aud_intf_voc_init complete \n");
				ret = bk_aud_intf_voc_start();
				if(ret== BK_ERR_AUD_INTF_OK){
					LOGW("bk_aud_intf_voc_start sucssed \n");
					audio_opened = true;

				}
			}
		}
#else
        int ret;
	//rtos_delay_milliseconds(100);
	aud_intf_voc_setup.aec_enable = aec_enable;
	aud_intf_voc_setup.spk_gain = 0x1f;
	aud_intf_voc_setup.mic_gain = 0x2d;
	aud_intf_voc_setup.samp_rate = audio_samp_rate;
	aud_intf_voc_setup.spk_mode   = AUD_DAC_WORK_MODE_DIFFEN;
	aud_intf_voc_setup.data_type  = AUD_INTF_VOC_DATA_TYPE_PCM;
	aud_intf_drv_setup.aud_intf_tx_mic_data = app_media_read_audio_frame;
	ret = bk_aud_intf_drv_init(&aud_intf_drv_setup);
	if (ret != BK_ERR_AUD_INTF_OK)
	{
		LOGE("bk_aud_intf_drv_init fail, ret:%d\n", ret);
		goto error;
	}

	aud_work_mode = AUD_INTF_WORK_MODE_VOICE;
	ret = bk_aud_intf_set_mode(aud_work_mode);
	if (ret != BK_ERR_AUD_INTF_OK)
	{
		LOGE("bk_aud_intf_set_mode fail, ret:%d\n", ret);
		goto error;
	}


	ret = bk_aud_intf_voc_init(aud_intf_voc_setup);
	if (ret != BK_ERR_AUD_INTF_OK)
	{
		LOGE("bk_aud_intf_voc_init fail, ret:%d\n", ret);
		goto error;
	}

	ret = bk_aud_intf_voc_start();
	if (ret != BK_ERR_AUD_INTF_OK)
	{
		LOGE("bk_aud_intf_voc_start fail, ret:%d\n", ret);
		goto error;
	}
	audio_opened = true;
        return;
error:
	webrtc_stream_start_audio_stop();
#endif
}

void video_read_frame_callback(frame_buffer_t *frame)
{
		webrtc_media_camera_ok = true;
		if(frame!= NULL && frame->length>0){
			if(get_video_frame_count<100){
				get_video_frame_count++;
				if(get_video_frame_count== 5){
					sdcard_can_mount = true;
				}
			}
			video_width = frame->width;
                        video_height = frame->height;
			uint32_t now = get_cur_timestamp();
			calculate_video_fps++;
			if(calculate_fps_starttime == 0){
				calculate_fps_starttime = now;
			}
			if(now-calculate_fps_starttime>=1000){
				video_fps = calculate_video_fps;
				calculate_video_fps = 0;
				calculate_fps_starttime = now;
			}
			
			//uint8_t frametype;
			//frametype = (frame->frame[4]& 0x1f);
			//PrintfH264Nal((uint8_t *)frame->frame,(int)frame->length);
			 //LOGW("##:frametype %d camera_type:%d(1:dvp 2:uvc) frame_id:%d, length:%d, frame_addr:%p \r\n",frametype,frame->type,frame->sequence,frame->length, frame->frame);

			if(frame->length>0 && runhua_runing){
			  if(webrtc_streamer_runing){
			    webrtc_streamer_input_video_data(WEBRTC_STREAM_MAIN,WEBRTC_VIDEO_H264,(unsigned char *)frame->frame,(size_t)frame->length);
			  }
			  if(webrtc_recording == true){
			      app_media_add_record_queue(frame->frame,frame->length,true);
			      app_media_record_queue_set_event();
			  }
		   
			}

		}	
}
void webrtc_yuv420sp_save_jpeg(char *filename,unsigned char* yuvData,int image_width, int image_height, int quality){
			int result;
			unsigned char *outbuf = rtc_bk_malloc(image_width* image_height* 2);
			if(outbuf){
			    result = yuv420sp_to_jpeg(yuvData,outbuf,image_width,image_height,quality);
			    if(result>0){
				LOGW("%s %d webrtc_yuv422_to_jpg outsize = %d\n", __func__, __LINE__,result);
				int fd = -1;
				int ret;
				fd = vfs_file_open(filename, O_RDWR | O_CREAT);
				if(fd>=0){
					ret = vfs_file_write(fd, outbuf, result);
					if(ret<0){
						LOGW("%s %d write err =%d\n", __func__, __LINE__,ret);
					}
					vfs_file_close(fd);
				}
			    }
			    rtc_bk_free(outbuf);
			    outbuf = NULL;
			}

}
void video_read_yuv_frame_callback(frame_buffer_t *frame){
	//LOGW("%s %d \n", __func__, __LINE__);
	get_yuv_frame_count++;
	if(get_yuv_frame_count%5 == 0){
		if(motion_detection_enable == true){
			webrtc_cpu1_yuv_frame_capture(frame,5);

		}
	}

	if(record_snapshot == true && strlen(record_snapshot_filename)>0){
		record_snapshot = false;
		if(frame->fmt == PIXEL_FMT_YUYV){
			if(sdcard_mounted ==1){
				LOGW("%s %d  snapshot = %s \n", __func__, __LINE__,record_snapshot_filename);
		                int destwidth = 80;
				int destheight = 60;
				int destsize = destwidth*destheight*2;;
				int yuv420spsize = frame->width* frame->height* 2;
				unsigned char *yuv420spbuf = rtc_bk_malloc(yuv420spsize);
				if(yuv420spbuf!= NULL){
					vyuy_to_yuv420p(frame->frame,yuv420spbuf,frame->width,frame->height);
					unsigned char * snapshot = rtc_bk_malloc(destsize);
					if(snapshot!= NULL){
						Convert_yuv420sp_scale(yuv420spbuf,frame->width,frame->height,snapshot,destwidth,destheight);
						webrtc_yuv420sp_save_jpeg(record_snapshot_filename,snapshot,destwidth,destheight,75);
						os_memset(record_snapshot_filename,0,sizeof(record_snapshot_filename));
						rtc_bk_free(snapshot);
						snapshot = NULL;
					}
					rtc_bk_free(yuv420spbuf);
					yuv420spbuf = NULL;
				}
			}

		}

        }
	if(yuv_frame_capture){
		yuv_frame_capture = false;
		uint32_t result = 0;
		//LOGW("%s %d \n", __func__, __LINE__);
		if(frame->fmt == PIXEL_FMT_YUYV){
                        if(sdcard_mounted ==1){
				int yuv420spsize = frame->width* frame->height* 2;
				unsigned char *yuv420spbuf = rtc_bk_malloc(yuv420spsize);
				if(yuv420spbuf){
					vyuy_to_yuv420p(frame->frame,yuv420spbuf,frame->width,frame->height);
					char stime[32] ={0};
					char szfilename[64] ={0};
					time_timestr_get(stime,sizeof(stime));
					snprintf(szfilename,sizeof(szfilename),"/sdcard/record/%s.jpg",stime);
					checkMkdir("/sdcard/record/");
					webrtc_yuv420sp_save_jpeg(szfilename,yuv420spbuf,frame->width,frame->height,75);
					rtc_bk_free(yuv420spbuf);
					yuv420spbuf = NULL;
				}
			}

		}
	}
}
void webrtc_media_yuv_thread(void *param){
        webrtc_yuv_thread_runing = true;
	frame_buffer_t * yuv_frame = NULL;
	LOGW("%s %d =======================================================\n", __func__, __LINE__);
	frame_buffer_fb_register(MODULE_LCD, FB_INDEX_DISPLAY);
	while(webrtc_yuv_thread_runing && webrtc_media_thread_runing){

			yuv_frame = frame_buffer_fb_display_pop_wait();
			if(yuv_frame!= NULL){
				video_read_yuv_frame_callback(yuv_frame);
				frame_buffer_fb_free(yuv_frame, MODULE_LCD);
			}
	}
	frame_buffer_fb_deregister(MODULE_LCD,FB_INDEX_DISPLAY);
	if(webrtc_media_yuv_exit_sem!= NULL){
			int count = rtos_get_semaphore_count(&webrtc_media_yuv_exit_sem);
			if(count == 0){				
				rtos_set_semaphore(&webrtc_media_yuv_exit_sem);
			}
	}
	webrtc_media_yuv_thread_hdl = NULL;
	LOGW("%s %d exit\n", __func__, __LINE__);
	rtos_delete_thread(NULL);
}

void webrtc_media_yuv_init(void){
	bk_err_t ret = BK_OK;
        ret = rtos_init_semaphore_ex(&webrtc_media_yuv_exit_sem, 1, 0);
	if(ret!= BK_OK){
		
	}
	ret = rtos_create_psram_thread(&webrtc_media_yuv_thread_hdl,
						 5,
						 "getyuv",
						 (beken_thread_function_t)webrtc_media_yuv_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("createwebrtc_media_yuv_thread  task fail \r\n");
		//webrtc_yuv_thread_runing = false;
		webrtc_media_yuv_thread_hdl = NULL;
	}else{
		
	}
	
}

void webrtc_media_thread(void *param){

	//LOGW("%s %d \n", __func__, __LINE__);
       
	frame_buffer_t * encode_frame = NULL;
	webrtc_media_thread_runing = true;
	frame_buffer_fb_register(MODULE_WIFI, FB_INDEX_H264);
	while(webrtc_media_thread_runing){
			encode_frame = frame_buffer_fb_read(MODULE_WIFI);
			if(encode_frame!= NULL){
			video_read_frame_callback(encode_frame);
			frame_buffer_fb_free(encode_frame, MODULE_WIFI);
			}
			
	}
        LOGW("%s %d \n", __func__, __LINE__);
	frame_buffer_fb_deregister(MODULE_WIFI,FB_INDEX_H264);	
	LOGW("%s %d \n", __func__, __LINE__);
	bk_dvp_camera_close();
	LOGW("%s %d \n", __func__, __LINE__);
	webrtc_stream_start_audio_stop();
	webrtc_yuv_thread_runing = false;	
	if(webrtc_media_yuv_exit_sem!= NULL ){
		if(webrtc_media_yuv_thread_hdl!= NULL){
		    rtos_get_semaphore(&webrtc_media_yuv_exit_sem, BEKEN_WAIT_FOREVER);
		}
		rtos_deinit_semaphore(&webrtc_media_yuv_exit_sem);
		webrtc_media_yuv_exit_sem = NULL;
	}


	webrtc_streamer_webserver_stop();
	webrtc_streamer_uninit();

	if(webrtc_media_exit_sem!= NULL){
			int count = rtos_get_semaphore_count(&webrtc_media_exit_sem);
			if(count == 0){				
				rtos_set_semaphore(&webrtc_media_exit_sem);
			}
	}


	webrtc_streamer_runing = false; 
	webrtc_media_thread_runing = false;
	webrtc_media_thread_hdl = NULL;
	LOGW("%s %d \n", __func__, __LINE__);
	rtos_delete_thread(NULL);

}
void webrtc_media_init(void){
	bk_err_t ret = BK_OK;

        ret = rtos_init_semaphore_ex(&webrtc_media_exit_sem, 1, 0);
	if(ret!= BK_OK){
		
	}

	ret = rtos_create_psram_thread(&webrtc_media_thread_hdl,
						 5,
						 "media",
						 (beken_thread_function_t)webrtc_media_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create mailbox resp task fail \r\n");
		webrtc_media_thread_runing = false;
		webrtc_media_thread_hdl = NULL;
	}else{
		
	}
}
void webrtc_start_media(void){
	bk_err_t ret = BK_OK;
	int count = 0;
	compress_ratio_t compress = {0};
	compress.mode = H264_MODE;
	compress.qp.init_qp = 20;
	compress.qp.i_min_qp = 20;
	compress.qp.i_max_qp = 40;
	compress.qp.p_min_qp = 20;
	compress.qp.p_max_qp = 40;
	compress.imb_bits = 150;
	compress.pmb_bits = 35;
	compress.enable = true;

  
       	//webrtc_stream_start_audio_start();
	ret = bk_dvp_camera_open(&camera_device);
	if(ret== BK_OK){
		//LOGW("%s %d  bk_dvp_camera_open  %d\n", __func__, __LINE__,ret);
			bk_dvp_camera_set_compress(&compress);
			webrtc_media_init();
			if(camera_device.mode == H264_YUV_MODE){
			   webrtc_media_yuv_init();
			}else{
				webrtc_yuv_thread_runing = true;
			}
			
	
	}
}
#define MEDIA_DIR "/flash/media/"
typedef struct tagWAVHEADER {
	uint8_t   ChunkID[4];     
	uint32_t  ChunkSize;      
	uint8_t   Format[4];      
	uint8_t   FmtChunkID[4];  
	uint32_t  FmtChunkSize;   
	uint16_t  AudioFormat;   
	uint16_t  NumChannels;    
	uint32_t  SampleRate;     
	uint32_t  ByteRate;       
	uint16_t  BlockAlign;     
	uint16_t  BitsPerSample;  
	uint8_t   DataChunkID[4];
	uint32_t  DataChunkSize;
} WAVHEADER;
static void webrtc_16k_convert_8k(int16_t *psi_buff,int16_t *psi_output,uint32_t ui_samples)
{
	uint32_t i = 0;
        uint32_t j = 0;
	for(i = 0; i < ui_samples; i += 2)
	{
	    psi_output[j++] = psi_buff[i];
           
	}
}
static void webrtc_stereo_convert_mono(int16_t *psi_buff,int16_t *psi_output,uint32_t ui_samples){
	uint32_t i = 0;
        uint32_t j = 0;
	for(i = 0; i < ui_samples; i += 2)
	{
	    psi_output[j++] = psi_buff[i];
           
	}
}
void webrtc_audio_play_wavfile(void){
	LOGW("%s %d \n", __func__, __LINE__);
	int wavfd =-1;
	size_t size;
	bk_err_t ret = BK_OK;
	char szwavfile[64];
	size_t readsize = 0;
	size_t playsize = 0;
	bool canplay = false;
	uint8_t *readwavdata = NULL;
	uint8_t *tempwavdata = NULL;
	uint8_t *playwavdata = NULL;
	webrtc_audio_playing_file = true;
	WAVHEADER wavheader;
	if(play_file_index == 0){
		snprintf(szwavfile,sizeof(szwavfile),"%s%s",MEDIA_DIR,"config.wav");
	}else if(play_file_index == 1){
		snprintf(szwavfile,sizeof(szwavfile),"%s%s",MEDIA_DIR,"start.wav");
	}else if(play_file_index == 2){
		snprintf(szwavfile,sizeof(szwavfile),"%s%s",MEDIA_DIR,"netfailed.wav");
	}else if(play_file_index == 3){
		snprintf(szwavfile,sizeof(szwavfile),"%s%s",MEDIA_DIR,"reboot.wav");
	}else if(play_file_index == 4){
		snprintf(szwavfile,sizeof(szwavfile),"%s%s",MEDIA_DIR,"wificonnok.wav");
	}else{
		return;
	}
	
	wavfd=vfs_file_open(szwavfile,O_RDONLY);
	if(wavfd>=0){
		size=vfs_file_read(wavfd,&wavheader,sizeof(WAVHEADER));
		if(size>0){
/*
			 LOGW("WAV File Header read:\n");
	    		 LOGW("File Type: %s\n", wavheader.ChunkID);
	   		 LOGW("File Size: %ld\n", wavheader.ChunkSize);
	  		 LOGW("WAV Marker: %s\n", wavheader.Format);
	 		 LOGW("Format Name: %s\n", wavheader.FmtChunkID);
	  		 LOGW("Format Length: %ld\n", wavheader.FmtChunkSize );
	  		 LOGW("Format Type: %hd\n", wavheader.AudioFormat);
	    		 LOGW("Number of Channels: %hd\n", wavheader.NumChannels);
	    		 LOGW("Sample Rate: %ld\n", wavheader.SampleRate);
	    		 LOGW("Sample Rate * Bits/Sample * Channels / 8: %ld\n", wavheader.ByteRate);
	    		 LOGW("Bits per Sample * Channels / wavheader.1: %hd\n", wavheader.BlockAlign);
	    		 LOGW("Bits per Sample: %hd\n", wavheader.BitsPerSample);
*/
			readsize = wavheader.SampleRate*wavheader.NumChannels*(wavheader.BitsPerSample/8)*20/1000;
			playsize = 8000*1*(16/8)*20/1000;
			if(wavheader.SampleRate == 16000 && wavheader.NumChannels == 1 && wavheader.BitsPerSample == 16){			
				canplay = true;
			}else if(wavheader.SampleRate == 16000 && wavheader.NumChannels == 2 && wavheader.BitsPerSample == 16){
				canplay = true;
			}else if(wavheader.SampleRate == 8000 && wavheader.NumChannels == 1 && wavheader.BitsPerSample == 16){
				canplay = true;
			}else if(wavheader.SampleRate == 8000 && wavheader.NumChannels == 2 && wavheader.BitsPerSample == 16){
				canplay = true;
			}else{
				canplay = false;
			}
			if(canplay){
				readwavdata = rtc_bk_malloc(readsize);
				playwavdata = rtc_bk_malloc(playsize);
				tempwavdata = rtc_bk_malloc(readsize);
				if(readwavdata!= NULL && playwavdata!= NULL && tempwavdata!= NULL){
					while(webrtc_audio_play_runing){
						rtc_bk_memset(readwavdata,0,readsize);
						rtc_bk_memset(tempwavdata,0,readsize);
						rtc_bk_memset(playwavdata,0,playsize);
						size=vfs_file_read(wavfd,readwavdata,readsize);
						if(size>0){
							if(wavheader.SampleRate == 16000 && wavheader.NumChannels == 1 && wavheader.BitsPerSample == 16){
								webrtc_16k_convert_8k((int16_t *)readwavdata,(int16_t *)playwavdata,size/2);
								size/=2;

							}else if(wavheader.SampleRate == 16000 && wavheader.NumChannels == 2 && wavheader.BitsPerSample == 16){
								webrtc_16k_convert_8k((int16_t *)readwavdata,(int16_t *)tempwavdata,size/2);
								size/=2;
								webrtc_stereo_convert_mono((int16_t *)tempwavdata,(int16_t *)playwavdata,size/2);
								size/=2;

							}else if(wavheader.SampleRate == 8000 && wavheader.NumChannels == 1 && wavheader.BitsPerSample == 16){
								rtc_bk_memcpy(playwavdata,readwavdata,size);

							}else if(wavheader.SampleRate == 8000 && wavheader.NumChannels == 2 && wavheader.BitsPerSample == 16){
								webrtc_stereo_convert_mono((int16_t *)readwavdata,(int16_t *)playwavdata,size/2);
								size/=2;

							}
					
							if(audio_opened == 1 && runhua_runing && canplay == true){
									//LOGE("%s %d   len = %d\n", __func__, __LINE__,size);
									 ret = bk_aud_intf_write_spk_data((uint8_t *)playwavdata, 320);
									if (ret != BK_OK) {
										LOGE("write spk data fail \r\n");
										break;
									}
							}
						}else{
							if(webrtc_audio_playing_sem!= NULL){
								rtos_get_semaphore(&webrtc_audio_playing_sem, 20);
						        }
							break;
						}
						if(webrtc_audio_playing_sem!= NULL){
							rtos_get_semaphore(&webrtc_audio_playing_sem, 20);
						}
					}
				}
			 }

		}
		vfs_file_close(wavfd);
	}else{
		LOGW("%s %d failed open %s\n", __func__, __LINE__,szwavfile);
	}
	if(readwavdata!= NULL){
		rtc_bk_free(readwavdata);
		readwavdata = NULL;
	}
	if(playwavdata!= NULL){
		rtc_bk_free(playwavdata);
		playwavdata = NULL;
	}
	if(tempwavdata!= NULL){
		rtc_bk_free(tempwavdata);
		tempwavdata = NULL;
	}
	play_file_index = -1;
	webrtc_audio_playing_file = false;
	
}
void webrtc_audio_play_thread(void *param){
        LOGW("%s %d \n", __func__, __LINE__);
        webrtc_audio_play_runing = true;
	while(webrtc_audio_play_runing){
	        rtos_get_semaphore(&webrtc_audio_play_sem, BEKEN_WAIT_FOREVER);
		if(webrtc_audio_play_runing){
		   webrtc_audio_play_wavfile();
		}
		

	}

	if(webrtc_audio_play_exit_sem!= NULL){
			int count = rtos_get_semaphore_count(&webrtc_audio_play_exit_sem);
			if(count == 0){
		           	rtos_set_semaphore(&webrtc_audio_play_exit_sem);
			}
	}
	if(webrtc_audio_playing_sem!= NULL){
		rtos_deinit_semaphore(&webrtc_audio_playing_sem);
		webrtc_audio_playing_sem = NULL;
	}
	if(webrtc_audio_play_sem!= NULL){
		rtos_deinit_semaphore(&webrtc_audio_play_sem);
		webrtc_audio_play_sem = NULL;
	}
	webrtc_audio_play_thread_hdl = NULL;
	LOGW("%s %d exit\n", __func__, __LINE__);
	rtos_delete_thread(NULL);
}
void webrtc_audio_play(){
	if(webrtc_audio_play_sem!= NULL){
			int count = rtos_get_semaphore_count(&webrtc_audio_play_sem);
			if(count == 0){
		           	rtos_set_semaphore(&webrtc_audio_play_sem);
			}
	}
}
#endif

void webrtc_media_uninit(void){
#if CONFIG_SYS_CPU1
	webrtc_media_thread_runing = false;
        if(webrtc_media_exit_sem!=NULL){
		rtos_get_semaphore(&webrtc_media_exit_sem, BEKEN_WAIT_FOREVER);
		rtos_deinit_semaphore(&webrtc_media_exit_sem);
		webrtc_media_exit_sem = NULL;

	}
#endif
}
#if CONFIG_SYS_CPU1

void webrtc_handle_cpu1_test(webrtc_cmd_t *pcmd){
	LOGW("%s %d \n", __func__, __LINE__);
       //webrtc_streamer_stop();
	webrtc_audio_play();
}
#endif
#if (CONFIG_SYS_CPU0 || CONFIG_SYS_CPU1)
void webrtc_handle_req_rx(webrtc_cmd_t *pcmd){
        //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
	if(pcmd->mb_cmd.param1 == APP_COM_START_RECORD){
#if CONFIG_SYS_CPU1
		if(sdcard_mounted ==1){
			webrtc_record_event = 0;
			webrtc_recording = true;
			webrtc_can_record = true;
		}
		//LOGW("%s %d start record\n", __func__, __LINE__);		
#endif		
	}else if(pcmd->mb_cmd.param1 == APP_COM_STOP_RECORD){
#if CONFIG_SYS_CPU1
		webrtc_recording = false;
		webrtc_can_record = false;
		//LOGW("%s %d stop record\n", __func__, __LINE__);
		if(webrtc_record_sem!= NULL){
					int count = rtos_get_semaphore_count(&webrtc_record_sem);
					if(count == 0){				
				    		rtos_set_semaphore(&webrtc_record_sem);
					}
		}
		
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_STOP_CPU1){
#if CONFIG_SYS_CPU1		
		runhua_runing = false;	
#endif

	}else if(pcmd->mb_cmd.param1 == APP_COM_COULD_PUBLISH_START){
#if CONFIG_SYS_CPU1
	    if(webrtc_cpu1_cloud_publish_stream == false){
			webrtc_camera_random_uuid(g_szCouldSessionId,37);
			webrtc_streamer_cloud_publish_realtime_stream(g_szCouldSessionId,strlen(g_szCouldSessionId),WEBRTC_STREAM_MAIN);
			webrtc_cpu1_cloud_publish_stream = true;

	    }
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_COULD_PUBLISH_STOP){
#if CONFIG_SYS_CPU1
	    if(webrtc_cpu1_cloud_publish_stream == true){
			if(strlen(g_szCouldSessionId)>0){
				webrtc_streamer_close_session(g_szCouldSessionId,strlen(g_szCouldSessionId));
			}

	    }
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_TEST){
#if CONFIG_SYS_CPU1
	        webrtc_handle_cpu1_test(pcmd);		
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_WIFI_CONNECT){
#if CONFIG_SYS_CPU1
		if(webrtc_streamer_runing == false && webrtc_streamer_start_cmd == false){
			webrtc_streamer_start_cmd = true;
			if(event_sem!= NULL){
			   	int count = rtos_get_semaphore_count(&event_sem);
			   	if(count == 0){
		           		rtos_set_semaphore(&event_sem);
				}
	                }
		}
	    
	       		
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_WIFI_DISCONNECT){
#if CONFIG_SYS_CPU1
	       		
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_WIFI_CONFIG){
#if CONFIG_SYS_CPU1
	    LPSDK_NET_NETWORK_CFG pnetconfig = (LPSDK_NET_NETWORK_CFG)pcmd->param.param1;  
	    os_memcpy(&runNetworkCfg,pnetconfig,sizeof(SDK_NET_NETWORK_CFG));
	    webrtc_cpu1_save_network = true;	
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_SDCARD_MOUNT){
#if CONFIG_SYS_CPU1 
		sdcard_mounted = pcmd->param.nparam1; 

#endif	
	}else if(pcmd->mb_cmd.param1 == APP_COM_WIFI_CONFIG_OK){
#if CONFIG_SYS_CPU0
	
	    webrtc_wifi_sta_connect((char*)runNetworkCfg.wireless.essid,(char*)runNetworkCfg.wireless.passd);
	
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_CPU1_START){
#if CONFIG_SYS_CPU0              
		cpu1_runing = true;
		if(wifi_conneced_sended == false && wifi_connected ==1){
			if(event_sem!= NULL){
			   int count = rtos_get_semaphore_count(&event_sem);
			   if(count == 0){
		           	rtos_set_semaphore(&event_sem);
			   }
	                }
		}
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_PTZ_CTRL){
#if CONFIG_SYS_CPU0  
		    int direction_x = pcmd->param.nparam1;
                    int direction_y = pcmd->param.nparam2;
		    stepmotor_ptz_control(&g_stepmsg,direction_x,direction_y);
#endif	
	}else if(pcmd->mb_cmd.param1 == APP_COM_CPU1_SDCARD_MOUNT){
#if CONFIG_SYS_CPU0
		g_can_mount_sdcard = true;
#endif
	}else if(pcmd->mb_cmd.param1 == APP_COM_FORMAT_SDCORD){
#if CONFIG_SYS_CPU0
		webrtc_unmount_sdcard();
		webrtc_vfs_format_sdcard();
		webrtc_mount_sdcard();
#endif

	}
	webrtc_mailbox_send_media_response_msg(pcmd->mb_cmd.param1,pcmd->mb_cmd.param2,pcmd->mb_cmd.param3);
}
void webrtc_handle_resp_rx(webrtc_cmd_t *pcmd){
	// LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
		if(pcmd!= NULL && pcmd->sem!= NULL && pcmd->mutex!=NULL){
			while(pcmd->isWaited ==0){
				//delay_us(100);
				rtos_delay_milliseconds(1);
			}
			rtos_lock_mutex(&pcmd->mutex);
			pcmd->responseed = 1;
			rtos_unlock_mutex(&pcmd->mutex);
			if(pcmd->isWaited==1){
				int count = rtos_get_semaphore_count(&pcmd->sem);
				if(count == 0){				
			    		rtos_set_semaphore(&pcmd->sem);
				}
			}
		}

}
#endif
#if CONFIG_SYS_CPU0
void webrtc_wakeup_cpu1(){
	LOGW("%s %d \n", __func__, __LINE__);
	wifi_conneced_sended = false;
	bk_pm_module_vote_boot_cp1_ctrl(PM_BOOT_CP1_MODULE_NAME_APP, PM_POWER_MODULE_STATE_ON);
	LOGW("%s %d \n", __func__, __LINE__);
}
void webrtc_shutdown_cpu1(){
	wifi_conneced_sended = false;
	bk_pm_module_vote_boot_cp1_ctrl(PM_BOOT_CP1_MODULE_NAME_APP, PM_POWER_MODULE_STATE_OFF);
}
void webrtc_wakeup_cpu2(){
	LOGW("%s %d \n", __func__, __LINE__);
	bk_pm_module_vote_boot_cp2_ctrl(PM_BOOT_CP2_MODULE_NAME_APP, PM_POWER_MODULE_STATE_ON);
	LOGW("%s %d \n", __func__, __LINE__);
}
void webrtc_shutdown_cpu2(){
	bk_pm_module_vote_boot_cp2_ctrl(PM_BOOT_CP2_MODULE_NAME_APP, PM_POWER_MODULE_STATE_OFF);
}
#endif 
#if CONFIG_SYS_CPU1

int send_datachannel_response_message(cJSON *root,char *sessionId,int dcstreamid)
{
		int ret = 0;
                int sendsize = 0;
		char *psendbuf;			    				    
		char *buf = cJSON_PrintUnformatted(root);
		if(buf!= NULL)
		{
                        //printf("webrtc send datachannel message   %s\n",buf);
		
			psendbuf = buf;
			sendsize = strlen(buf);
			while(runhua_runing){
							
				ret = webrtc_streamer_datachannel_send_message(sessionId,WEBRTC_DMT_TEXT,dcstreamid,psendbuf,sendsize);
				if(ret == sendsize){								
								
					//printf("webrtc download   %d  %d\n",ret,readed);
					break;
				}else if(ret >=0 && ret<sendsize){
					psendbuf+=ret;
					sendsize-=ret;
					rtos_delay_milliseconds(200);		
				}else if(ret == -7){
								
					rtos_delay_milliseconds(200);
				}else{
								
					printf("webrtc  failed send datachannel message (WEBRTC_DMT_BINARY) %d\n",ret);
					break;
				}
						
			}
			os_free(buf);
                        // printf("webrtc send datachannel message   end\n");
		}
		return ret;

}
void webrtc_get_video_adapt(cJSON *response_msg_resp,cJSON *item){
									

      cJSON *jresps = cJSON_CreateObject();
      cJSON *jrespgva = cJSON_CreateObject();

      cJSON_AddNumberToObject(jrespgva,"code",200);
      cJSON_AddStringToObject(jrespgva,"state","sucessed");
      cJSON_AddStringToObject(jrespgva,"height","");
      cJSON_AddStringToObject(jrespgva,"fit","fill");
      cJSON_AddItemToObject(jresps,"get_video_adapt",jrespgva);
      cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_get_camera_info(cJSON *response_msg_resp,cJSON *item){
									
      cJSON *jresps = cJSON_CreateObject();
      cJSON *jrespgdi = cJSON_CreateObject();

      cJSON_AddNumberToObject(jrespgdi,"code",200);
      cJSON_AddStringToObject(jrespgdi,"state","sucessed");

      cJSON *jcapability = cJSON_CreateObject();
      cJSON_AddNumberToObject(jcapability,"record",1);
      cJSON_AddNumberToObject(jcapability,"led_control",1);
      cJSON_AddNumberToObject(jcapability,"roi_set",1);
      cJSON_AddNumberToObject(jcapability,"face_detect",0);
      cJSON_AddNumberToObject(jcapability,"move_detect",1);
      cJSON_AddItemToObject(jrespgdi,"capability",jcapability);

      cJSON *jsupported_resolutions = cJSON_CreateArray();

      cJSON *jresolution = cJSON_CreateObject();
      cJSON_AddNumberToObject(jresolution,"index",0);
      cJSON_AddStringToObject(jresolution,"source","MainStream");
      cJSON_AddStringToObject(jresolution,"pixel","640*480");
      cJSON_AddStringToObject(jresolution,"text","SD");
      cJSON_AddNumberToObject(jresolution,"enable_preview",1);
      cJSON_AddNumberToObject(jresolution,"enable_record",1);
      int fps[1]={25};
      cJSON *jsupport_fps = cJSON_CreateIntArray(fps,1);
      cJSON_AddItemToObject(jresolution,"support_fps",jsupport_fps);
 


      cJSON_AddItemToArray(jsupported_resolutions,jresolution);

      cJSON_AddItemToObject(jrespgdi,"supported_resolutions",jsupported_resolutions);

      cJSON *jpreview = cJSON_CreateObject();
      cJSON_AddNumberToObject(jpreview,"enable",webrtc_preview_enable);
      cJSON_AddNumberToObject(jpreview,"resolution_index",webrtc_preview_resolution_index);
      cJSON_AddNumberToObject(jpreview,"mirror_h",mirror_h);
      cJSON_AddNumberToObject(jpreview,"mirror_v",mirror_v);
      cJSON_AddItemToObject(jrespgdi,"preview",jpreview);



      cJSON *jrecord_info = cJSON_CreateObject();
      cJSON_AddNumberToObject(jrecord_info,"auto",webrtc_record_auto);
      cJSON_AddStringToObject(jrecord_info,"start_time",webrtc_record_start_time);
      cJSON_AddStringToObject(jrecord_info,"end_time",webrtc_record_end_time);
      cJSON_AddNumberToObject(jrecord_info,"resolution_index",webrtc_record_resolution_index);
      cJSON_AddNumberToObject(jrecord_info,"record_time",webrtc_record_time);
      cJSON_AddNumberToObject(jrecord_info,"record_interval",webrtc_record_interval);
      cJSON_AddNumberToObject(jrecord_info,"cover",webrtc_record_cover);
      cJSON_AddItemToObject(jrespgdi,"record_info",jrecord_info);
/*
      cJSON *jimage_mirror = cJSON_CreateObject();

      cJSON_AddNumberToObject(jimage_mirror,"horizontal",mirror_h);
      cJSON_AddNumberToObject(jimage_mirror,"vertical",mirror_v);

      cJSON_AddItemToObject(jrespgdi,"image_mirror",jimage_mirror);
*/


      cJSON_AddItemToObject(jresps,"get_camera_info",jrespgdi);
      cJSON_AddItemToArray(response_msg_resp,jresps);

}
void webrtc_get_device_info(cJSON *response_msg_resp,cJSON *item){
      cJSON *jresps = cJSON_CreateObject();
      cJSON *jrespgdi = cJSON_CreateObject();

      cJSON *jtimezone = cJSON_CreateObject();

      netif_ip4_config_t ip4_config;
      get_netif_ip4_config(&ip4_config);
      cJSON_AddNumberToObject(jrespgdi,"code",200);
      cJSON_AddStringToObject(jrespgdi,"state","sucessed");
      cJSON_AddStringToObject(jrespgdi,"sn",runSystemCfg.deviceInfo.serialNumber);
      cJSON_AddStringToObject(jrespgdi,"ip",ip4_config.ip);
      cJSON_AddStringToObject(jrespgdi,"mac",ip4_config.mask);


      //LOGW("%s %d  ip:%s\n", __func__, __LINE__,ip4_config.ip);
      //LOGW("%s %d  mask:%s\n", __func__, __LINE__,ip4_config.mask);
      //LOGW("%s %d  gateway:%s\n", __func__, __LINE__,ip4_config.gateway);
      //LOGW("%s %d  dns:%s\n", __func__, __LINE__,ip4_config.dns);

      cJSON_AddStringToObject(jrespgdi,"product_key","webrtc");
      cJSON_AddNumberToObject(jrespgdi,"device_type",1);
      cJSON_AddStringToObject(jrespgdi,"chip_type","bk-7258");
      cJSON_AddStringToObject(jrespgdi,"version","v.0.122");
      cJSON_AddNumberToObject(jrespgdi,"network",1);


	    
       cJSON_AddNumberToObject(jtimezone,"offset",timezone_offset);
       cJSON_AddStringToObject(jtimezone,"abbr",timezone_abbr);
       cJSON_AddNumberToObject(jtimezone,"isdst",timezone_isdst);
       cJSON_AddStringToObject(jtimezone,"name",timezone_name);
       cJSON_AddStringToObject(jtimezone,"text",timezone_text);
    

       cJSON_AddItemToObject(jrespgdi,"timezone",jtimezone);


      cJSON_AddItemToObject(jresps,"get_device_info",jrespgdi);
      cJSON_AddItemToArray(response_msg_resp,jresps);
}

void webrtc_set_timezone(cJSON *response_msg_resp,cJSON *item)
{
    cJSON *set_timezone = cJSON_GetObjectItem(item,"timezone");
    if(set_timezone!= NULL){

	    cJSON *set_ntp = cJSON_GetObjectItem(item,"ntp");

	    cJSON *set_timezone_offset = cJSON_GetObjectItem(set_timezone,"offset");
	    cJSON *set_timezone_abbr = cJSON_GetObjectItem(set_timezone,"abbr");
            cJSON *set_timezone_isdst = cJSON_GetObjectItem(set_timezone,"isdst");
            cJSON *set_timezone_name = cJSON_GetObjectItem(set_timezone,"name");
            cJSON *set_timezone_text = cJSON_GetObjectItem(set_timezone,"text");

	    cJSON *jresps = cJSON_CreateObject();
	    cJSON *jrespgdi = cJSON_CreateObject();
	    cJSON *jtimezone = cJSON_CreateObject();

	    cJSON_AddNumberToObject(jrespgdi,"code",200);
	    cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	    if(set_timezone_offset!= NULL){
	     timezone_offset = (int)cJSON_GetNumberValue(set_timezone_offset);
	     LOGW("%s %d  timezone_offset:%d\n", __func__, __LINE__,timezone_offset);
	     cJSON_AddNumberToObject(jtimezone,"offset",timezone_offset);
	    }
	    if(set_timezone_abbr!= NULL){
	     snprintf(timezone_abbr,sizeof(timezone_abbr),"%s",cJSON_GetStringValue(set_timezone_abbr));
	     cJSON_AddStringToObject(jtimezone,"abbr",timezone_abbr);
	    }
	    if(set_timezone_isdst!= NULL){
	      timezone_isdst = (int)cJSON_GetNumberValue(set_timezone_isdst);
	     cJSON_AddNumberToObject(jtimezone,"isdst",timezone_isdst);
	    }
	    if(set_timezone_name!= NULL){
	     snprintf(timezone_name,sizeof(timezone_name),"%s",cJSON_GetStringValue(set_timezone_name));
	     cJSON_AddStringToObject(jtimezone,"name",timezone_name);
	    }
	    if(set_timezone_text!= NULL){
	     snprintf(timezone_text,sizeof(timezone_text),"%s",cJSON_GetStringValue(set_timezone_text));
	     cJSON_AddStringToObject(jtimezone,"text",timezone_text);
	    }	

	    cJSON_AddItemToObject(jrespgdi,"timezone",jtimezone);

	    if(set_ntp!= NULL){		
		snprintf(timezone_ntp,sizeof(timezone_ntp),"%s",cJSON_GetStringValue(set_ntp));
		cJSON_AddStringToObject(jrespgdi,"ntp",cJSON_GetStringValue(set_ntp));
	    }
	    time_rtc_ntp_sync_update(timezone_offset,timezone_ntp);
	    cJSON_AddItemToObject(jresps,"set_timezone",jrespgdi);
	    cJSON_AddItemToArray(response_msg_resp,jresps);
    }

}

void webrtc_set_mirror(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();
    
	cJSON *hjson = cJSON_GetObjectItem(item,"horizontal");
	cJSON *vjson = cJSON_GetObjectItem(item,"vertical");
        cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	if(hjson!= NULL){
	   
	   mirror_h = (int)cJSON_GetNumberValue(hjson);
	   //LOGW("%s %d  %d\n", __func__, __LINE__,mirror_h);
	   cJSON_AddNumberToObject(jrespgdi,"horizontal",mirror_h);
	}
	if(vjson!= NULL){
	    
	    mirror_v = (int)cJSON_GetNumberValue(vjson);
	    //LOGW("%s %d  %d\n", __func__, __LINE__,mirror_v);
	    cJSON_AddNumberToObject(jrespgdi,"vertical",mirror_v);
	}
	cJSON_AddItemToObject(jresps,"set_image_mirror",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_get_led_info(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

        cJSON_AddNumberToObject(jrespgdi,"onoff",1);
	cJSON_AddItemToObject(jresps,"get_led_info",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_set_record(cJSON *response_msg_resp,cJSON *item)
{
    	cJSON *jresps = cJSON_CreateObject();
    	cJSON *jrespgdi = cJSON_CreateObject();

        cJSON *set_record_auto = cJSON_GetObjectItem(item,"auto");
	cJSON *set_record_start_time = cJSON_GetObjectItem(item,"start_time");
	cJSON *set_record_end_time = cJSON_GetObjectItem(item,"end_time");
	cJSON *set_record_resolution_index = cJSON_GetObjectItem(item,"resolution_index");
	cJSON *set_record_record_time = cJSON_GetObjectItem(item,"record_time");
	cJSON *set_record_record_interval = cJSON_GetObjectItem(item,"record_interval");
	cJSON *set_record_cover = cJSON_GetObjectItem(item,"cover");

    	cJSON_AddNumberToObject(jrespgdi,"code",200);
    	cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	if(set_record_auto!= NULL){
	     webrtc_record_auto = (int)cJSON_GetNumberValue(set_record_auto);
	     cJSON_AddNumberToObject(jrespgdi,"auto",webrtc_record_auto);
	}
	if(set_record_start_time!= NULL){
	     snprintf(webrtc_record_start_time,sizeof(webrtc_record_start_time),"%s",cJSON_GetStringValue(set_record_start_time));
	     cJSON_AddStringToObject(jrespgdi,"start_time",webrtc_record_start_time);
	}
	if(set_record_end_time!= NULL){
	snprintf(webrtc_record_end_time,sizeof(webrtc_record_end_time),"%s",cJSON_GetStringValue(set_record_end_time));
	cJSON_AddStringToObject(jrespgdi,"end_time",webrtc_record_end_time);
	}
	if(set_record_resolution_index!= NULL){
		webrtc_record_resolution_index = (int)cJSON_GetNumberValue(set_record_resolution_index);
		cJSON_AddNumberToObject(jrespgdi,"resolution_index",webrtc_record_resolution_index);
	}
	if(set_record_record_time!= NULL){
	 webrtc_record_time = (int)cJSON_GetNumberValue(set_record_record_time);
	cJSON_AddNumberToObject(jrespgdi,"record_time",webrtc_record_time);
	}
	if(set_record_record_interval!= NULL){
	    webrtc_record_interval = (int)cJSON_GetNumberValue(set_record_record_interval);
	    cJSON_AddNumberToObject(jrespgdi,"record_interval",webrtc_record_interval);
	}
	if(set_record_cover!= NULL){
	     webrtc_record_cover = (int)cJSON_GetNumberValue(set_record_cover);
	     cJSON_AddNumberToObject(jrespgdi,"cover",webrtc_record_cover);
	}
	
    	cJSON_AddItemToObject(jresps,"set_record",jrespgdi);
    	cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_get_tfcard_info(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();
	int total_space = 0;
	int free_space = 0;
	webrtc_sdcard_info("/sdcard/",&total_space,&free_space);
	cJSON_AddNumberToObject(jrespgdi,"code",200);
    	cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddNumberToObject(jrespgdi,"total",total_space);
	cJSON_AddNumberToObject(jrespgdi,"used",total_space-free_space);

	cJSON_AddItemToObject(jresps,"get_tfcard_info",jrespgdi);
    	cJSON_AddItemToArray(response_msg_resp,jresps);
}

void webrtc_format_tfcard(cJSON *response_msg_resp,cJSON *item)
{

	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();


	cJSON_AddNumberToObject(jrespgdi,"code",200);

	if(sdcard_mounted ==1){
		webrtc_recording = false;
	        webrtc_can_record = false;
		webrtc_mailbox_cpu1_sdcard_format();
		cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	}else{
		cJSON_AddStringToObject(jrespgdi,"state","no tfcard");
	}
	cJSON_AddItemToObject(jresps,"format_tfcard",jrespgdi);
    	cJSON_AddItemToArray(response_msg_resp,jresps);

}
void webrtc_set_preview(cJSON *response_msg_resp,cJSON *item)
{

	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON *set_preview_enable = cJSON_GetObjectItem(item,"enable");
        cJSON *set_preview_resolution_index = cJSON_GetObjectItem(item,"resolution_index");

	cJSON_AddNumberToObject(jrespgdi,"code",200);
    	cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	if(set_preview_enable!= NULL){
	    webrtc_preview_enable = (int)cJSON_GetNumberValue(set_preview_enable);
	    cJSON_AddNumberToObject(jrespgdi,"enable",webrtc_preview_enable);
	}
	if(set_preview_resolution_index!= NULL){
	    webrtc_preview_resolution_index = (int)cJSON_GetNumberValue(set_preview_resolution_index);
	    cJSON_AddNumberToObject(jrespgdi,"resolution_index",webrtc_preview_resolution_index);
	}

	cJSON_AddItemToObject(jresps,"set_preview",jrespgdi);
    	cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_get_battery_info(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddNumberToObject(jrespgdi,"electricity",70);

	cJSON_AddItemToObject(jresps,"get_battery_info",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_ptz_control(cJSON *response_msg_resp,cJSON *item){

	cJSON *xjson = cJSON_GetObjectItem(item,"direction_x");
	cJSON *yjson = cJSON_GetObjectItem(item,"direction_y");
	if(xjson!= NULL && yjson!= NULL){
	int direction_x = (int)cJSON_GetNumberValue(xjson);
	int direction_y = (int)cJSON_GetNumberValue(yjson);
	//LOGW("%s %d direction_x = %d \n", __func__, __LINE__,direction_x);
	//LOGW("%s %d direction_y = %d \n", __func__, __LINE__,direction_y);
	webrtc_mailbox_cpu1_send_ptz_ctrl(direction_x,direction_y);
	
	}
}
void webrtc_open_door(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddItemToObject(jresps,"open",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_get_roi_config(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddItemToObject(jresps,"get_roi_config",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_get_move_detect(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();
	cJSON *jsensitivity = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddNumberToObject(jrespgdi,"enable",(int)motion_detection_enable);
	cJSON_AddNumberToObject(jrespgdi,"notify",(int)motion_detection_alarm);
	cJSON_AddStringToObject(jrespgdi,"start_time",motion_detection_start_time);
	cJSON_AddStringToObject(jrespgdi,"end_time",motion_detection_end_time);

	cJSON_AddNumberToObject(jsensitivity,"value",motion_detection_sensitivity);
	cJSON_AddNumberToObject(jsensitivity,"step",1);
        cJSON_AddStringToObject(jsensitivity,"range","1:10");
	cJSON_AddItemToObject(jrespgdi,"sensitivity",jsensitivity);

        

	cJSON_AddItemToObject(jresps,"get_move_detect",jrespgdi);

        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_set_move_detect(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();
	cJSON *jmove_detect_enable = cJSON_GetObjectItem(item,"enable");
	cJSON *jmove_detect_sensitivity = cJSON_GetObjectItem(item,"sensitivity");
	cJSON *jmove_detect_start_time = cJSON_GetObjectItem(item,"start_time");
	cJSON *jmove_detect_end_time = cJSON_GetObjectItem(item,"end_time");
	cJSON *jmove_detect_notify = cJSON_GetObjectItem(item,"notify");
	if(jmove_detect_enable!= NULL){
		motion_detection_enable = (bool)cJSON_GetNumberValue(jmove_detect_enable);
	}
	if(jmove_detect_sensitivity!= NULL){
		motion_detection_sensitivity = (int)cJSON_GetNumberValue(jmove_detect_sensitivity);
	}
	if(jmove_detect_notify!= NULL){
		motion_detection_alarm = (bool)cJSON_GetNumberValue(jmove_detect_notify);
	}

	if(jmove_detect_start_time!= NULL){
	        snprintf(motion_detection_start_time,sizeof(motion_detection_start_time),"%s",cJSON_GetStringValue(jmove_detect_start_time));
	     
	}
	if(jmove_detect_end_time!= NULL){
		snprintf(motion_detection_end_time,sizeof(motion_detection_end_time),"%s",cJSON_GetStringValue(jmove_detect_end_time));
	
	}


	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");

	cJSON_AddNumberToObject(jrespgdi,"enable",(int)motion_detection_enable);
	cJSON_AddNumberToObject(jrespgdi,"notify",(int)motion_detection_alarm);
	cJSON_AddStringToObject(jrespgdi,"start_time",motion_detection_start_time);
	cJSON_AddStringToObject(jrespgdi,"end_time",motion_detection_end_time);
	cJSON_AddNumberToObject(jrespgdi,"sensitivity",(int)motion_detection_sensitivity);

	cJSON_AddItemToObject(jresps,"set_move_detect",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_get_face_detect(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddItemToObject(jresps,"get_face_detect",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_set_person(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddItemToObject(jresps,"set_person",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_reset(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddItemToObject(jresps,"reset",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_system_reboot(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddItemToObject(jresps,"reboot",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_ota_download(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddItemToObject(jresps,"ota_download",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
void webrtc_ota_update(cJSON *response_msg_resp,cJSON *item)
{
	cJSON *jresps = cJSON_CreateObject();
	cJSON *jrespgdi = cJSON_CreateObject();

	cJSON_AddNumberToObject(jrespgdi,"code",200);
        cJSON_AddStringToObject(jrespgdi,"state","sucessed");
	cJSON_AddItemToObject(jresps,"ota_update",jrespgdi);
        cJSON_AddItemToArray(response_msg_resp,jresps);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//remote playback
//////////////////////////////////////////////////////////////////////////////////////////////////
static int find_remoteplay_by_sessionId(const Webrtc_remoteplay *rp, const char *sessionId)
{
	if(rp!=NULL && sessionId!= NULL){
		
		if(strcmp(rp->szsessionId,sessionId) == 0){
		     return 0;
		}
	}
        return 1;
}
Webrtc_remoteplay * get_remoteplay_from_sessionId(char *sessionId,size_t sessionId_len)
{
      Webrtc_remoteplay *remoteplay =NULL;
      if(sessionId!= NULL && sessionId_len>0 && g_webrtc_remoteplay!= NULL){
	          
	      RTCList *l= NULL;
	      l = rtc_list_find_custom(g_webrtc_remoteplay, (RTCCompareFunc)find_remoteplay_by_sessionId, sessionId);
	      if(l!= NULL)
	      {
			remoteplay = (Webrtc_remoteplay *)l->data;		
	      }else{		       
	      }
     }
     return remoteplay;
}

void del_all_remoteplay_message(Webrtc_remoteplay *rp){
	if(rp!= NULL){
		rtc_list_for_each(rp->msg_list, (void (*)(void*))destory_datachannel_message);		
		rtc_list_free(rp->msg_list);
		rp->msg_list = NULL;
	}
}


static const char * jpgbase64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char * webrtc_jpgbase64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = jpgbase64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = jpgbase64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = jpgbase64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = jpgbase64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = jpgbase64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = jpgbase64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

char * webrtc_get_snapshot_base64(char *snapshotfile){

	int fd;
	unsigned int size;   
	char *buffer = NULL;
        char *buffer1 = NULL;
	char * head = "data:image/jpeg;base64,";
	char * temp = NULL;
	size_t result;
	char *ret1; 
	unsigned int length;
	//LOGW("%s %d  file %s \n", __func__, __LINE__,snapshotfile);
	fd = vfs_file_open(snapshotfile, O_RDONLY);
	 if (fd < 0) {
	    LOGE("get_snapshot_base64 open_error\n");
	    return NULL;
        }

	vfs_file_lseek(fd, 0L, SEEK_END);
	size = vfs_file_tell(fd);
	vfs_file_lseek(fd, 0L, SEEK_SET);


	if(size<=0){
		vfs_file_close(fd);
		return NULL;
	}
	buffer = (char *)rtc_bk_malloc(2*size);
	if (NULL == buffer){
	      LOGE("get_snapshot_base64 memory_error\n");
              vfs_file_close(fd);
	     return NULL;
        }
	result = vfs_file_read(fd,buffer,size);
	if (result != size){  
	    vfs_file_close(fd);
            LOGE("get_snapshot_base64 reading_error\n");
	    rtc_bk_free(buffer);  
           return NULL;  
        }  
        vfs_file_close(fd);
	buffer1 = (char *)rtc_bk_malloc(2*size);
	if (NULL == buffer1){
	     LOGE("get_snapshot_base64 memory_error\n");
	     rtc_bk_free(buffer); 
	     return NULL;
        }
	rtc_bk_memset(buffer1,0,2*size);
        sprintf(buffer1,"%s",head);
	temp = buffer1+strlen(head);
        ret1 = webrtc_jpgbase64_encode((const unsigned char*)buffer, temp, size);
        length = strlen(buffer1);
        //LOGW("%d  %d\n\n", size,length);
	rtc_bk_free(buffer);
	return buffer1;

}

char* webrtc_get_file_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename) {
	char * extension = (char *)dot;
        return extension + 1; 
    }
    return ""; 
}
int  webrtc_send_getfilelistItem(int dcstreamid,cJSON *request,char *starttime,char* filename,char *snapshot,int event,int filetime){

        int ret = 0;
        cJSON *response = cJSON_CreateObject();
        cJSON *response_data = cJSON_CreateObject();
        cJSON *response_msg = cJSON_CreateObject();
        cJSON *response_msg_resp = cJSON_CreateArray();


        cJSON *jdata = cJSON_GetObjectItem(request,"data");

	cJSON *jmessageId = cJSON_GetObjectItem(jdata,"messageId");
	cJSON *jfrom = cJSON_GetObjectItem(jdata,"from");
	cJSON *jto = cJSON_GetObjectItem(jdata,"to");
	cJSON *jsessionId = cJSON_GetObjectItem(jdata,"sessionId");
	cJSON *jsessionType = cJSON_GetObjectItem(jdata,"sessionType");
	cJSON *jmessage = cJSON_GetObjectItem(jdata,"message");
        if(jto!= NULL){
	cJSON_AddStringToObject(response_data,"from",cJSON_GetStringValue(jto));
        }
	if(jfrom!= NULL){
	cJSON_AddStringToObject(response_data,"to",cJSON_GetStringValue(jfrom));
	}
	if(jmessageId!= NULL){
	cJSON_AddStringToObject(response_data,"messageId",cJSON_GetStringValue(jmessageId));
	}
	if(jsessionId!= NULL){
	cJSON_AddStringToObject(response_data,"sessionId",cJSON_GetStringValue(jsessionId));
        }
	if(jsessionType!= NULL){
	cJSON_AddStringToObject(response_data,"sessionType",cJSON_GetStringValue(jsessionType));
        }

       cJSON *jgetfilelistobj = cJSON_CreateObject();
 
       cJSON *jgetfilelist = cJSON_CreateObject();
       cJSON *jfilelist = cJSON_CreateArray();
  
   
       cJSON *jfile = cJSON_CreateObject();
       cJSON_AddStringToObject(jfile,"starttime",starttime);
       cJSON_AddStringToObject(jfile,"filename",filename);
       cJSON_AddNumberToObject(jfile,"filetime",filetime);
       cJSON_AddNumberToObject(jfile,"event",event);
       if(snapshot!= NULL){
	          cJSON_AddStringToObject(jfile,"snapshot",snapshot);
       }
       cJSON_AddItemToArray(jfilelist,jfile);
      


      cJSON_AddItemToObject(jgetfilelist,"filelists",jfilelist);
      cJSON_AddNumberToObject(jgetfilelist,"code",200);
      cJSON_AddStringToObject(jgetfilelist,"state","sucessed");
     
      cJSON_AddItemToObject(jgetfilelistobj,"getfilelist",jgetfilelist);
      


      cJSON_AddItemToArray(response_msg_resp,jgetfilelistobj);

     if(response!=NULL){
	   cJSON_AddStringToObject(response,"eventName","_play");
	   cJSON_AddItemToObject(response_msg,"response",response_msg_resp);
	   cJSON_AddItemToObject(response_data,"message",response_msg);
	   cJSON_AddItemToObject(response,"data",response_data);  
	   ret = send_datachannel_response_message(response,cJSON_GetStringValue(jsessionId),dcstreamid);
	  
	  cJSON_Delete(response);
    }
    return ret;

}
int webrtc_avi_get_file_time(char *pfilename){
	int filetime_sec = 0;
        long frames;
	int framew;
	int frameh;
	double framerate; 
        

	int audiobytes ;
	int audiochannels;
	int audiobits;	
	long audiorate;
        int streams = 0;
        webrtc_avi_t *hAviHandle = NULL;
        hAviHandle = WEBRTC_AVI_open_input_file(pfilename,1);
        if(hAviHandle!= NULL){
                frames =WEBRTC_AVI_video_frames(hAviHandle);
		framew = WEBRTC_AVI_video_width(hAviHandle);
		frameh = WEBRTC_AVI_video_height(hAviHandle);
		framerate = WEBRTC_AVI_frame_rate(hAviHandle); 
		if(frames<=0 || framew<=0 || frameh<=0 || framerate<=0){
			if(hAviHandle->anum>0){
				audiobytes = WEBRTC_AVI_audio_bytes(hAviHandle);
				audiochannels= WEBRTC_AVI_audio_channels(hAviHandle);
				audiobits =  WEBRTC_AVI_audio_bits(hAviHandle);	
				audiorate = WEBRTC_AVI_audio_rate(hAviHandle);
				filetime_sec = (int)(2*audiobytes/(audiochannels*(audiobits/8)*(audiorate)));
			}

		}else{
			streams++;
			  if(hAviHandle->anum>0){
				audiobytes = WEBRTC_AVI_audio_bytes(hAviHandle);
				audiochannels= WEBRTC_AVI_audio_channels(hAviHandle);
				audiobits =  WEBRTC_AVI_audio_bits(hAviHandle);	
				audiorate = WEBRTC_AVI_audio_rate(hAviHandle);
				filetime_sec = (int)(2*audiobytes/(audiochannels*(audiobits/8)*(audiorate)));
				//LOGW("%s %d filetime_sec = %d\n", __func__, __LINE__,filetime_sec);
			  }else{
				filetime_sec = (int)(frames/framerate);
				//LOGW("%s %d filetime_sec = %d\n", __func__, __LINE__,filetime_sec);
			  }
		}
		WEBRTC_AVI_close(hAviHandle);
        }
	return filetime_sec;
}
int  webrtc_get_play_files(const char *path,int dcstreamid,cJSON *request,char *endtime) {
    int ret = 0;
    DIR *dir = vfs_file_opendir(path);
    if (dir == NULL) {
        LOGE("failed opendir %s\n",path);
        return -1;
    }
  
    struct dirent *entry;
    while ((entry = vfs_file_readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // �ļ�
	    char snapshot_file_name[128] = {0};
	    char file_name[128] = {0};
	    char starttime[32] = {0};
	    int event = 0;
            int filetime = 1*60;
	    int tm_year;
	    int tm_mon;
	    int tm_mday;
	    int tm_hour;
	    int tm_min;
	    int tm_sec;
	    bool is_current_record = false;
	    if(strcmp(webrtc_get_file_extension(entry->d_name),"avi")==0 || strcmp(webrtc_get_file_extension(entry->d_name),"mp4")==0){
		//LOGW("file: %s/%s\n", path,entry->d_name);
		snprintf(file_name,sizeof(file_name),"%s/%s",path,entry->d_name);
		if(strlen(current_record_filename)>0 && strcmp(current_record_filename,file_name) == 0){
			is_current_record = true;
		}
		if(is_current_record == false){
			char *name = strtok(entry->d_name, ".");
			if(name!= NULL){
				
				int res = sscanf(name, "%02d-%04d%02d%02d%02d%02d%02d", &event,&tm_year,&tm_mon,&tm_mday,&tm_hour,&tm_min,&tm_sec);
				//LOGW("%s %d  %s  %d  %d\n", __func__, __LINE__,entry->d_name,res,event);
				if(res == 7){
					snprintf(snapshot_file_name,sizeof(snapshot_file_name),"%s/%s.jpg",path,name);
					filetime = webrtc_avi_get_file_time(file_name);
					if(filetime>0){
						snprintf(starttime,sizeof(starttime),"%04d-%02d-%02d %02d:%02d:%02d",tm_year,tm_mon,tm_mday,tm_hour,tm_min,tm_sec);
						char *snapshot =  webrtc_get_snapshot_base64(snapshot_file_name);
				
						ret = webrtc_send_getfilelistItem(dcstreamid,request,starttime,file_name,snapshot,event,filetime);
						if(snapshot!= NULL){
							rtc_bk_free(snapshot);
							snapshot = NULL;

						}
						if(ret<0){
						    break;
						}
					        
					}else{
						vfs_file_unlink(file_name);
						vfs_file_unlink(snapshot_file_name);
					}			

				}

			}
		}
	    }
            
        } else if (entry->d_type == DT_DIR) { // Ŀ¼
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char path_buf[128] = {0};
                snprintf(path_buf, sizeof(path_buf), "%s/%s", path, entry->d_name);
                int result = webrtc_get_play_files(path_buf,dcstreamid,request,endtime); // �ݹ��г���Ŀ¼�е��ļ�
		if(result<0){
			break;
		}
            }
        }
    }
    vfs_file_closedir(dir);
    return ret;
}
void webrtc_getfilelist(int dcstreamid,char *starttime,char *endtime,cJSON *request){
	LOGI("%s %d %s %s\n", __func__, __LINE__,starttime,endtime);
	char szpathf[64];
	snprintf(szpathf,64,"/sdcard/record/%s",starttime);
	webrtc_get_play_files(szpathf,dcstreamid,request,endtime);
}

void PlayBack_OpenFile(Webrtc_remoteplay *p,cJSON *response_msg_resp,cJSON *item){
    
      if(item!= NULL){
       char *pfilename = cJSON_GetStringValue(item);
       LOGW("webrtc - PlayBack_OpenFile  %s \n",pfilename);
       char szfile[128];
       cJSON *jresps = cJSON_CreateObject();
       cJSON *jrespopen = cJSON_CreateObject();
       snprintf(szfile,sizeof(szfile),"%s",pfilename);
	if(p->hAviHandle!= NULL){
		//LOGW("webrtc - PlayBack_OpenFile  WEBRTC_AVI_close  %p\n",p->hAviHandle);
		WEBRTC_AVI_close(p->hAviHandle);
		p->hAviHandle = NULL;
		p->streams = 0;

	}
	p->hAviHandle = WEBRTC_AVI_open_input_file(szfile,1);	  
	if(p->hAviHandle != NULL){		
		p->code_type = WEBRTC_VIDEO_H264;
		p->streams = 0;
	        p->seek = false;
                p->seekIndex = 0;
                p->frames =WEBRTC_AVI_video_frames(p->hAviHandle);
		p->framew = WEBRTC_AVI_video_width(p->hAviHandle);
		p->frameh = WEBRTC_AVI_video_height(p->hAviHandle);
		p->framerate = WEBRTC_AVI_frame_rate(p->hAviHandle); 
                LOGI("webrtc -avi video frames %d framew %d frameh %d framerate %d\n",(int)p->frames,p->framew,p->frameh,(int)p->framerate);
		if(p->frames<=0 || p->framew<=0 || p->frameh<=0 || p->framerate<=0){
				WEBRTC_AVI_close(p->hAviHandle);
		                p->hAviHandle = NULL;
				p->bStart = true;    	       
				

		}else{
			p->stopvideo = false;
			p->stopaudio = false;
			p->streams++;
		        if(p->hAviHandle->anum>0){
			        p->streams++;
				p->audiobytes = WEBRTC_AVI_audio_bytes(p->hAviHandle);
				p->audiochannels= WEBRTC_AVI_audio_channels(p->hAviHandle);
				p->audiobits =  WEBRTC_AVI_audio_bits(p->hAviHandle);	
				p->audiorate = WEBRTC_AVI_audio_rate(p->hAviHandle);

				LOGI("webrtc - audiobytes %d audiochannels %d audiobits %d audiorate %d\n",(int)p->audiobytes,p->audiochannels,p->audiobits,(int)p->audiorate);
		                LOGI("webrtc - audio time  %d \n",(int)(p->audiobytes/(p->audiochannels*(p->audiobits/8)*(p->audiorate/1000))));
		                //long msec = (p->audiobytes/(p->audiochannels*(p->audiobits/8)*(p->audiorate/1000)));
				//p->delay =  (msec/p->frames);
				 //LOGW("webrtc - video delay %ld  %ld  %ld  \n",p->frames,p->delay,msec);
				if(p->framerate>0){
					p->delay =  (900/p->framerate);
				}
		        }else{
				p->stopaudio = true;
				p->delay =  (900/p->framerate);
			}	
			WEBRTC_AVI_seek_start(p->hAviHandle);
	                p->bPause = false;
	                p->bStart = false;
		}
  

	      cJSON_AddNumberToObject(jrespopen,"filesize",100);
	      cJSON_AddNumberToObject(jrespopen,"code",200);
	      cJSON_AddStringToObject(jrespopen,"state","sucessed");

	         
	
	}else{
	      cJSON_AddNumberToObject(jrespopen,"filesize",0);
	      cJSON_AddNumberToObject(jrespopen,"code",300);
	      cJSON_AddStringToObject(jrespopen,"state","failed open file");
        }
      cJSON_AddItemToObject(jresps,"open",jrespopen);
      cJSON_AddItemToArray(response_msg_resp,jresps);
      }
}
void PlayBack_Start(Webrtc_remoteplay *p,cJSON *response_msg_resp,cJSON *item){
      LOGI("%s %d \n", __func__, __LINE__);
      int start = 0;
      if(cJSON_IsNumber(item)){
           start = (int)cJSON_GetNumberValue(item);
      }
      p->bStart = true;
      p->lpreshowtime = 0;
      p->lprevideotime = 0;
      p->lpreaudiotime = 0;
      p->lpcurrenttime = 0;
      p->bPause = false;   
      if(start==0){
	      p->seek = false;
	      p->seekIndex = 0;
      }else{
	      p->seek = true;
	      p->seekIndex = start;
      }

      cJSON *jresps = cJSON_CreateObject();
      cJSON *jrespstart = cJSON_CreateObject();

      cJSON_AddNumberToObject(jrespstart,"current",start);
      cJSON_AddNumberToObject(jrespstart,"code",200);
      cJSON_AddStringToObject(jrespstart,"state","sucessed");
      cJSON_AddItemToObject(jresps,"start",jrespstart);
      cJSON_AddItemToArray(response_msg_resp,jresps);
     

}
void PlayBack_Pause(Webrtc_remoteplay *p,cJSON *response_msg_resp,cJSON *item){
      
      if(cJSON_IsTrue(item)){
         p->bPause = true;
      }else{
	 p->bPause = false;
      }

      cJSON *jresps = cJSON_CreateObject();
      cJSON *jresppause = cJSON_CreateObject();

      cJSON_AddNumberToObject(jresppause,"code",200);
      cJSON_AddStringToObject(jresppause,"state","sucessed");
      cJSON_AddItemToObject(jresps,"pause",jresppause);
      cJSON_AddItemToArray(response_msg_resp,jresps);

 
}
void PlayBack_Seek(Webrtc_remoteplay *p,cJSON *response_msg_resp,cJSON *item){
    
      
      if(cJSON_IsNumber(item)){
            p->seekIndex = (int)cJSON_GetNumberValue(item);
	    p->seek = true;
      }
      cJSON *jresps = cJSON_CreateObject();
      cJSON *jrespseek = cJSON_CreateObject();

      cJSON_AddNumberToObject(jrespseek,"code",200);
      cJSON_AddStringToObject(jrespseek,"state","sucessed");
      cJSON_AddItemToObject(jresps,"seek",jrespseek);
      cJSON_AddItemToArray(response_msg_resp,jresps);
}
void PlayBack_Stop(Webrtc_remoteplay *p,cJSON *response_msg_resp,cJSON *item){


      if(p->hAviHandle!= NULL){
		WEBRTC_AVI_close(p->hAviHandle);
		p->hAviHandle = NULL;
		p->streams = 0;
	        p->bStart = false;
	        p->lpreshowtime = 0;
	        p->lprevideotime = 0;
	        p->lpreaudiotime = 0;
	        p->lpcurrenttime = 0;
	        p->seek = false;
	        p->seekIndex = 0;
      }

      cJSON *jresps = cJSON_CreateObject();
      cJSON *jrespseek = cJSON_CreateObject();

      cJSON_AddNumberToObject(jrespseek,"code",200);
      cJSON_AddStringToObject(jrespseek,"state","sucessed");
      cJSON_AddItemToObject(jresps,"stop",jrespseek);
      cJSON_AddItemToArray(response_msg_resp,jresps); 
 
}
void PlayBack_send_step(Webrtc_remoteplay *p,char *eventName,uint32_t now){

        int current = 0;
        if(p->hAviHandle!= NULL){
		
                current =  (int)p->hAviHandle->video_pos*100/(int)p->frames;
		//printf("  message= %d    %ld   %ld  \n",current,p->hAviHandle->video_pos,p->frames);

        }
        cJSON *response = cJSON_CreateObject();
        cJSON *response_data = cJSON_CreateObject();
        cJSON *response_msg = cJSON_CreateObject();
        cJSON *response_msg_resp = cJSON_CreateArray();
        char szmessageId[64] = {0};
        webrtc_camera_random_uuid(szmessageId,37);

	cJSON_AddStringToObject(response_data,"from",p->szto);
	cJSON_AddStringToObject(response_data,"to",p->szfrom);
	cJSON_AddStringToObject(response_data,"messageId",szmessageId);
	cJSON_AddStringToObject(response_data,"sessionId",p->szsessionId);
	cJSON_AddStringToObject(response_data,"sessionType",p->szsessionType);


        cJSON *jcurrentstateobj = cJSON_CreateObject();
        cJSON *jcurrentstate = cJSON_CreateObject();
     
  
   
        cJSON *state = cJSON_CreateObject();
        cJSON_AddNumberToObject(state,"current",current);
        cJSON_AddNumberToObject(state,"total",100);
        cJSON_AddNumberToObject(state,"timestamp",now);
    
    
     
      
       cJSON_AddItemToObject(jcurrentstate,"position",state);
       cJSON_AddNumberToObject(jcurrentstate,"code",200);
       cJSON_AddStringToObject(jcurrentstate,"state","sucessed");

       cJSON_AddItemToObject(jcurrentstateobj,"currentstate",jcurrentstate);

      cJSON_AddItemToArray(response_msg_resp,jcurrentstateobj);

      if(response!=NULL){
	   cJSON_AddStringToObject(response,"eventName",eventName);
	   cJSON_AddItemToObject(response_msg,"response",response_msg_resp);
	   cJSON_AddItemToObject(response_data,"message",response_msg);
	   cJSON_AddItemToObject(response,"data",response_data);
	  
	   send_datachannel_response_message(response,p->szsessionId,p->datachannel_streamid);
	  
	  cJSON_Delete(response);
     }    
 
}
void PlayBack_send_end(Webrtc_remoteplay *p,char *eventName){

        cJSON *response = cJSON_CreateObject();
        cJSON *response_data = cJSON_CreateObject();
        cJSON *response_msg = cJSON_CreateObject();
        cJSON *response_msg_resp = cJSON_CreateArray();
        char szmessageId[64] = {0};
        webrtc_camera_random_uuid(szmessageId,37);
	cJSON_AddStringToObject(response_data,"from",p->szto);
	cJSON_AddStringToObject(response_data,"to",p->szfrom);
	cJSON_AddStringToObject(response_data,"messageId",szmessageId);
	cJSON_AddStringToObject(response_data,"sessionId",p->szsessionId);
	cJSON_AddStringToObject(response_data,"sessionType",p->szsessionType);


 
       cJSON *jstopojb = cJSON_CreateObject();
       cJSON *jstoparray = cJSON_CreateArray();
  
   
       cJSON *jstop = cJSON_CreateObject();
       cJSON_AddStringToObject(jstop,"stop","end");

    
       cJSON_AddItemToArray(jstoparray,jstop);
      
       cJSON_AddItemToObject(jstopojb,"stop",jstoparray);


      cJSON_AddNumberToObject(jstopojb,"code",200);
      cJSON_AddStringToObject(jstopojb,"state","sucessed");
      cJSON_AddItemToArray(response_msg_resp,jstopojb);

      if(response!=NULL){
	   cJSON_AddStringToObject(response,"eventName",eventName);
	   cJSON_AddItemToObject(response_msg,"response",response_msg_resp);
	   cJSON_AddItemToObject(response_data,"message",response_msg);
	   cJSON_AddItemToObject(response,"data",response_data);
	  
	   send_datachannel_response_message(response,p->szsessionId,p->datachannel_streamid);
	  
	  cJSON_Delete(response);
     } 
 
}
void webrtc_failed(cJSON *response_msg_resp,cJSON *item){
									

      cJSON *jresps = cJSON_CreateObject();
      cJSON *jresppreset = cJSON_CreateObject();

      cJSON_AddNumberToObject(jresppreset,"code",500);
      cJSON_AddStringToObject(jresppreset,"state","failed! not supported");
      cJSON_AddItemToObject(jresps,item->string,jresppreset);
      cJSON_AddItemToArray(response_msg_resp,jresps);
}
void remoteplay_messageFun(Webrtc_remoteplay * rp,uint32_t now){
	     int ret = 0;
	     int iCount;
	     int i;
	     bool sendmsg = true;
             if(!rp->stop_ && runhua_runing){
		       
		       if(rp->msg_list!= NULL && rtc_list_size(rp->msg_list)>0){
				datachannel_msg * dcmsg = (datachannel_msg *)rtc_list_nth_data(rp->msg_list,0);
				if(dcmsg!= NULL){
				     //LOGW("remoteplay_messageFun  message= %s \n",dcmsg->msg);
				      cJSON *root =  cJSON_Parse(dcmsg->msg);
				      if(root!= NULL){
						cJSON *response = cJSON_CreateObject();
						cJSON *response_data = cJSON_CreateObject();
						cJSON *response_msg = cJSON_CreateObject();
						cJSON *response_msg_resp = cJSON_CreateArray();

						cJSON *jeventName = cJSON_GetObjectItem(root,"eventName");
						cJSON *jdata = cJSON_GetObjectItem(root,"data");

					        cJSON *jmessageId = cJSON_GetObjectItem(jdata,"messageId");
					        cJSON *jfrom = cJSON_GetObjectItem(jdata,"from");
					        cJSON *jto = cJSON_GetObjectItem(jdata,"to");
					        cJSON *jsessionId = cJSON_GetObjectItem(jdata,"sessionId");
					        cJSON *jsessionType = cJSON_GetObjectItem(jdata,"sessionType");
						cJSON *jmessage = cJSON_GetObjectItem(jdata,"message");
						if(jto!= NULL){
						cJSON_AddStringToObject(response_data,"from",cJSON_GetStringValue(jto));
						}
						if(jfrom!= NULL){
						cJSON_AddStringToObject(response_data,"to",cJSON_GetStringValue(jfrom));
						}
						if(jmessageId!= NULL){
						cJSON_AddStringToObject(response_data,"messageId",cJSON_GetStringValue(jmessageId));
						}
						if(jsessionId!= NULL){
						cJSON_AddStringToObject(response_data,"sessionId",cJSON_GetStringValue(jsessionId));
						}
						if(jsessionType!= NULL){
						cJSON_AddStringToObject(response_data,"sessionType",cJSON_GetStringValue(jsessionType));
						}
						
						
						if(jeventName!= NULL && jdata!= NULL && jmessage!= NULL){
							
							cJSON * jrequest = cJSON_GetObjectItem(jmessage,"request");
							char *peventName = cJSON_GetStringValue(jeventName);
							
							if(peventName!= NULL && strcmp(peventName,"__play")==0 && jrequest!= NULL){
								
								if(jrequest->type == cJSON_Array){
                                                                   
								  iCount = cJSON_GetArraySize(jrequest);
								   
							           if(iCount>0){
							              for( i=0; i<iCount; i++){
									if(cJSON_GetArrayItem(jrequest, i)!= NULL){
										cJSON * item = cJSON_GetArrayItem(jrequest, i)->child;
										if(item!= NULL){
											if(strcmp(item->string,"getfilelist")==0){
												cJSON *jstarttime = cJSON_GetObjectItem(item,"starttime");
												cJSON *jendtime = cJSON_GetObjectItem(item,"endtime");
												if(jstarttime!= NULL && jendtime!= NULL && sdcard_mounted ==1){
												webrtc_getfilelist(dcmsg->streamid,cJSON_GetStringValue(jstarttime),cJSON_GetStringValue(jendtime),root);
												}
																								

											}else if(strcmp(item->string,"open")==0){
												LOGW("remoteplay_messageFun  message=------ %s \n",item->string);
												sendmsg = true;
												PlayBack_OpenFile(rp,response_msg_resp,item);

											}else if(strcmp(item->string,"start")==0){
												LOGW("remoteplay_messageFun  message=------ %s \n",item->string);
												sendmsg = true;
												PlayBack_Start(rp,response_msg_resp,item);

											}else if(strcmp(item->string,"pause")==0){
												LOGW("remoteplay_messageFun  message=------ %s \n",item->string);
												sendmsg = true;
												PlayBack_Pause(rp,response_msg_resp,item);

											}else if(strcmp(item->string,"seek")==0){
												LOGW("remoteplay_messageFun  message=------ %s \n",item->string);
												sendmsg = true;
												PlayBack_Seek(rp,response_msg_resp,item);

											}else if(strcmp(item->string,"stop")==0){
												LOGW("remoteplay_messageFun  message=------ %s \n",item->string);
												sendmsg = true;
												PlayBack_Stop(rp,response_msg_resp,item);

											}else{
												webrtc_failed(response_msg_resp,item);
												LOGW("setup_messageFun  message= %s \n",item->string);
											}
										}
									}
								      }
								   }

								}
								


							}
						}
						if(response!=NULL){
					            cJSON_AddStringToObject(response,"eventName","_play");
						    cJSON_AddItemToObject(response_msg,"response",response_msg_resp);
						    cJSON_AddItemToObject(response_data,"message",response_msg);
						    cJSON_AddItemToObject(response,"data",response_data);
						    if(sendmsg){
						         send_datachannel_response_message(response,cJSON_GetStringValue(jsessionId),rp->datachannel_streamid);
						    }
						    cJSON_Delete(response);
						}
					       cJSON_Delete(root);
					}
			            rp->msg_list = rtc_list_remove(rp->msg_list,dcmsg);
				    destory_datachannel_message(dcmsg);
				}
			}
		}
}

#define PLAYBACK_VIDEO_BUF_SIZE   60*1024
#define PLAYBACK_AUDIO_BUF_SIZE   2*1024
void thread_h264_sd_PlayBack(void *arg){
	
          Webrtc_remoteplay *p = (Webrtc_remoteplay *)arg;
	  //LOGW("%s %d %p\n", __func__, __LINE__,p);
	  p->hAviHandle = NULL;
	  p->stop_ = 0;
	  p->bPause = false;
	  p->bStart = false;
	  uint32_t now = get_cur_timestamp();
	  char *pvideobuf = (char*)rtc_bk_malloc((PLAYBACK_VIDEO_BUF_SIZE)*sizeof(char));
	  char *paudiobuf = (char*)rtc_bk_malloc((PLAYBACK_AUDIO_BUF_SIZE)*sizeof(char));
          char *ppcmbuf = (char*)rtc_bk_malloc((PLAYBACK_AUDIO_BUF_SIZE)*sizeof(char));
	  char *pvbuf;
	  char *pabuf;
	  int ret;
          int i;
          int keyframe = 0;
          long frame = 0;
      
	  while(p->stop_== 0  && runhua_runing){
                       now = get_cur_timestamp();
	               remoteplay_messageFun(p,now);


                       if(p->bStart == false){
				rtos_delay_milliseconds(5);
				continue;
                        }
                        if(p->bPause == true){
				rtos_delay_milliseconds(5);
				continue;
                        }
			//LOGW("%s %d %p\n", __func__, __LINE__,p);
		        if(p->hAviHandle!= NULL){
					pvbuf = pvideobuf;
					pabuf = paudiobuf;
					p->lpcurrenttime = get_cur_timestamp();
					if(p->lpcurrenttime-p->lpreshowtime>1000){
						p->lpreshowtime = p->lpcurrenttime;
						PlayBack_send_step(p,"_play",now);
						p->sended = 0;
					}
					//LOGW("%s %d \n", __func__, __LINE__);
					if(p->lpcurrenttime-p->lprevideotime>p->delay){
						p->lprevideotime = p->lpcurrenttime;

					         if(p->seek== true){
						   p->seek = false;
						   frame =  p->seekIndex*p->frames/100;
						   WEBRTC_AVI_set_video_position(p->hAviHandle,frame);
						   while(1){
						     ret = WEBRTC_AVI_read_frame(p->hAviHandle,pvbuf,&keyframe);
						     if(ret == -1){
							break;
						     }
						     if(keyframe==1){
							webrtc_streamer_player_clean_send_buffer(p->szsessionId,strlen(p->szsessionId));
							if(p->hAviHandle->anum>0 && p->hAviHandle->video_frames>0){
							
							long audioseekpos = p->hAviHandle->video_pos*p->hAviHandle->track[p->hAviHandle->aptr].audio_bytes/p->hAviHandle->video_frames;
							WEBRTC_AVI_set_audio_position(p->hAviHandle,audioseekpos);
							}
							break;
						     }
						   }
					         }else{
						    ret = WEBRTC_AVI_read_frame(p->hAviHandle,pvbuf,&keyframe);
						}
						if(ret == -1){
	
							if(p->stopvideo == false){
								LOGW("webrtc -AVI_read_frame h264 end   \n");
								p->stopvideo = true;
								p->streams--;
								if(p->streams<=0){

									      if(p->hAviHandle!= NULL){
											WEBRTC_AVI_close(p->hAviHandle);
											p->hAviHandle = NULL;
											p->streams = 0;
											p->bStart = false;
											p->lpreshowtime = 0;
											p->lprevideotime = 0;
											p->lpreaudiotime = 0;
											p->lpcurrenttime = 0;
											p->seek = false;
											p->seekIndex = 0;
									      }
									      PlayBack_send_end(p,"_play");
								}
							}
					
						 
						}else if(ret>0){


					
							p->sended+=ret;
							//LOGW("webrtc - AVI_read_frame   %d\n",(int)ret);
							if(pvbuf[0] == 0x0 && pvbuf[1] == 0x0 && pvbuf[2] == 0x0 && pvbuf[3] == 0x01){
				                            webrtc_streamer_player_input_video_data(p->code_type,p->szsessionId,(unsigned char *)pvbuf,ret);
							}else{
							    LOGW("webrtc -AVI_read_frame h264 data err  len = %d\n",ret);
							}

						}

					}
					p->lpcurrenttime = get_cur_timestamp();
					if(p->lpcurrenttime-p->lpreaudiotime>=19){

					  if(p->lpcurrenttime-p->lpreaudiotime>=20){
				 		p->lpreaudiotime = p->lpcurrenttime+1;
                              		  }else{
						 p->lpreaudiotime = p->lpcurrenttime;
					  }
						
						if(p->hAviHandle!= NULL && p->hAviHandle->anum>0){
				                        
							ret = WEBRTC_AVI_read_audio(p->hAviHandle,pabuf,160);
							if(ret==160){

						   		 int16_t* dest = (int16_t*)ppcmbuf;
						  		 uint8_t *data = (uint8_t *)paudiobuf;
								for(i = 0;i<160;i++){
									     *((int16_t*)dest)=alaw_to_s16(*data);
									     data++;
									     dest++;
								}
								p->sended+=320;
								//LOGW("webrtc - AVI_read_audio   %d\n",(int)ret);
								webrtc_streamer_player_input_audio_data(p->szsessionId,(unsigned char *)ppcmbuf,320);


							}else{
								
							        if(p->stopaudio == false){	
									p->stopaudio = true;
									LOGW("webrtc - AVI_read_audio  err %d\n",(int)ret);
									p->streams--;
									if(p->streams<=0){
									     if(p->hAviHandle!= NULL){
											WEBRTC_AVI_close(p->hAviHandle);
											p->hAviHandle = NULL;
											p->streams = 0;
											p->bStart = false;
											p->lpreshowtime = 0;
											p->lprevideotime = 0;
											p->lpreaudiotime = 0;
											p->lpcurrenttime = 0;
											p->seek = false;
											p->seekIndex = 0;
									      }
									      PlayBack_send_end(p,"_play");

									}
								}
						

							}


						}

					}
			}

             
                  rtos_delay_milliseconds(5);	
	  }

	  if(p->hAviHandle!= NULL){
		WEBRTC_AVI_close(p->hAviHandle);
		p->hAviHandle = NULL;
	 }
	  if(pvideobuf!= NULL){
			rtc_bk_free(pvideobuf);
			pvideobuf = NULL;
	  }
	  if(paudiobuf!= NULL){
			rtc_bk_free(paudiobuf);
			paudiobuf = NULL;
	   }
	  if(ppcmbuf!= NULL){
			rtc_bk_free(ppcmbuf);
			ppcmbuf = NULL;
	   }
	  del_all_remoteplay_message(p);
	  g_webrtc_remoteplay = rtc_list_remove(g_webrtc_remoteplay,p);
          rtc_bk_free(p);
          LOGW("webrtc - thread sd PlayBack end\n");
	  rtos_delete_thread(NULL);
}

void webrtc_dealwith_datachannel_message(datachannel_msg *dc_msg){
      cJSON *root =  cJSON_Parse(dc_msg->msg);
      int iCount = 0;
      int i = 0;
      int response_send = 0;
      char *peventName = NULL;
      if(root!= NULL){
		
		cJSON *response = cJSON_CreateObject();
		cJSON *response_data = cJSON_CreateObject();
		cJSON *response_msg = cJSON_CreateObject();
		cJSON *response_msg_resp = cJSON_CreateArray();
		cJSON *jeventName = cJSON_GetObjectItem(root,"eventName");
		cJSON *jdata = cJSON_GetObjectItem(root,"data");

		cJSON *jmessageId = cJSON_GetObjectItem(jdata,"messageId");
		cJSON *jfrom = cJSON_GetObjectItem(jdata,"from");
		cJSON *jto = cJSON_GetObjectItem(jdata,"to");
		cJSON *jsessionId = cJSON_GetObjectItem(jdata,"sessionId");
		cJSON *jsessionType = cJSON_GetObjectItem(jdata,"sessionType");
		cJSON *jmessage = cJSON_GetObjectItem(jdata,"message");
		if(jto!= NULL){
			cJSON_AddStringToObject(response_data,"from",cJSON_GetStringValue(jto));
		}
		if(jfrom!= NULL){
			cJSON_AddStringToObject(response_data,"to",cJSON_GetStringValue(jfrom));
		}
		if(jmessageId!= NULL){
			cJSON_AddStringToObject(response_data,"messageId",cJSON_GetStringValue(jmessageId));
		}
		if(jsessionId!= NULL){
			cJSON_AddStringToObject(response_data,"sessionId",cJSON_GetStringValue(jsessionId));
		}
		if(jsessionType!= NULL){
			cJSON_AddStringToObject(response_data,"sessionType",cJSON_GetStringValue(jsessionType));
		}
	
		if(jeventName!= NULL && jdata!= NULL){
			char * sessionId = cJSON_GetStringValue(jsessionId);
			int sessionId_len = strlen(sessionId);
			cJSON *jmessage = cJSON_GetObjectItem(jdata,"message");
			if(jmessage!= NULL){
				cJSON * jrequest = cJSON_GetObjectItem(jmessage, "request");
				peventName = cJSON_GetStringValue(jeventName);
				//LOGD("%s %d %s \n", __func__, __LINE__,peventName);
				if(peventName!= NULL && strcmp(peventName,"ipc_set")==0 && jrequest!= NULL){
				

						if(jrequest->type == cJSON_Array){

								 iCount = cJSON_GetArraySize(jrequest);
							         if(iCount>0){
							              for( i=0; i<iCount; i++){
									if(cJSON_GetArrayItem(jrequest, i)!= NULL){
										cJSON * item = cJSON_GetArrayItem(jrequest, i)->child;
										if(item!= NULL){
											//LOGW("%s %d %s \n", __func__, __LINE__,item->string);
											if(strcmp(item->string,"get_video_adapt")==0){
												webrtc_get_video_adapt(response_msg_resp,item);
												response_send = 1;

											}else if(strcmp(item->string,"get_device_info")==0){
												webrtc_get_device_info(response_msg_resp,item);
												response_send = 1;

											}else if(strcmp(item->string,"get_camera_info")==0){
												webrtc_get_camera_info(response_msg_resp,item);	
												response_send = 1;
											}else if(strcmp(item->string,"get_tfcard_info")==0){
                                                						webrtc_get_tfcard_info(response_msg_resp,item);
												response_send = 1;
                 
											}else if(strcmp(item->string,"get_battery_info")==0){
                                                						webrtc_get_battery_info(response_msg_resp,item);
                                                						response_send = 1;

											}else if(strcmp(item->string,"set_timezone")==0){
												webrtc_set_timezone(response_msg_resp,item);
                                                						response_send = 1;

											}else if(strcmp(item->string,"set_image_mirror")==0){
                                                						webrtc_set_mirror(response_msg_resp,item);
												response_send = 1;

											}else if(strcmp(item->string,"set_record")==0){
                                                						webrtc_set_record(response_msg_resp,item);
												response_send = 1;

											}else if(strcmp(item->string,"format_tfcard")==0){
												webrtc_format_tfcard(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"set_preview")==0){
												webrtc_set_preview(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"get_led_info")==0){
												webrtc_get_led_info(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"reset")==0){
												webrtc_reset(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"reboot")==0){
												webrtc_system_reboot(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"ota_download")==0){
												webrtc_ota_download(response_msg_resp,item);
												response_send = 0;
											}else if(strcmp(item->string,"ota_update")==0){
												webrtc_ota_update(response_msg_resp,item);
												response_send = 0;

											}else{
												//LOGW("%s %d %s \n", __func__, __LINE__,dc_msg->msg);

											}
										
										}
									}
								      }
								 }

						}
				}else if(peventName!= NULL && strcmp(peventName,"ptz_set")==0 && jrequest!= NULL){
					
					if(jrequest->type == cJSON_Array){
						iCount = cJSON_GetArraySize(jrequest);
							         if(iCount>0){
							              for( i=0; i<iCount; i++){
									if(cJSON_GetArrayItem(jrequest, i)!= NULL){
										cJSON * item = cJSON_GetArrayItem(jrequest, i)->child;
										if(item!= NULL){
											if(strcmp(item->string,"ptz_control")==0){
												webrtc_ptz_control(response_msg_resp,item);
												response_send = 0;
											}else if(strcmp(item->string,"get_ptz_position")==0){
												LOGW("%s %d %s \n", __func__, __LINE__,item->string);
											}else if(strcmp(item->string,"get_cruise_config")==0){
												LOGW("%s %d %s \n", __func__, __LINE__,item->string);
											}else if(strcmp(item->string,"set_cruise_config")==0){
												LOGW("%s %d %s \n", __func__, __LINE__,item->string);
											}else if(strcmp(item->string,"set_cruise_position")==0){
												LOGW("%s %d %s \n", __func__, __LINE__,item->string);
											}else if(strcmp(item->string,"switch_cruise_position")==0){
												LOGW("%s %d %s \n", __func__, __LINE__,item->string);
											}
										}
									}
								      }
								 }
					}
				}else if(peventName!= NULL && strcmp(peventName,"lock_set")==0 && jrequest!= NULL){
					
					if(jrequest->type == cJSON_Array){
						iCount = cJSON_GetArraySize(jrequest);
							         if(iCount>0){
							              for( i=0; i<iCount; i++){
									if(cJSON_GetArrayItem(jrequest, i)!= NULL){
										cJSON * item = cJSON_GetArrayItem(jrequest, i)->child;
										if(item!= NULL){
											if(strcmp(item->string,"open")==0){
												webrtc_open_door(response_msg_resp,item);
												response_send = 1;
											}
										}
									}
								      }
								 }
					}
				}else if(peventName!= NULL && strcmp(peventName,"__play")==0 && jrequest!= NULL && jfrom!= NULL && jto!= NULL && jsessionType!= NULL&& jmessageId!= NULL){
							Webrtc_remoteplay *rp = get_remoteplay_from_sessionId(sessionId,sessionId_len);
							if(rp==NULL){
									//LOGW("%s %d sessionId=%s\n", __func__, __LINE__,sessionId);
									if(rtc_list_size(g_webrtc_remoteplay)<3){ //�������������
										rp = (Webrtc_remoteplay *)rtc_bk_malloc(sizeof(Webrtc_remoteplay));
										if(rp!= NULL){
											rtc_bk_memset(rp,0,sizeof(Webrtc_remoteplay));
											rp->datachannel_streamid = dc_msg->streamid;
											rp->hAviHandle = NULL;
											rp->stop_ = false;
											snprintf(rp->szsessionId,sizeof(rp->szsessionId),"%s",sessionId);
											snprintf(rp->szto,sizeof(rp->szto),"%s",cJSON_GetStringValue(jto));
											snprintf(rp->szfrom,sizeof(rp->szfrom),"%s",cJSON_GetStringValue(jfrom));
											snprintf(rp->szsessionType,sizeof(rp->szsessionType),"%s",cJSON_GetStringValue(jsessionType));
											datachannel_msg *rpmsg = (datachannel_msg *)rtc_bk_malloc(sizeof(datachannel_msg));
											if(rpmsg!= NULL){
													rpmsg->streamid = dc_msg->streamid;
													rpmsg->msg = (char*)rtc_bk_malloc(dc_msg->msgsize);
													if(rpmsg->msg!= NULL){
														rtc_bk_memset(rpmsg->msg,0,dc_msg->msgsize);
														rtc_bk_memcpy(rpmsg->msg,dc_msg->msg,dc_msg->msgsize);
														rpmsg->msgsize = dc_msg->msgsize;

													}
													rp->msg_list = rtc_list_append(rp->msg_list, (void*)rpmsg);

											}													
											g_webrtc_remoteplay = rtc_list_append(g_webrtc_remoteplay, (void*)rp);
														     
						 					int ret = rtos_create_psram_thread(&rp->pthid_,
															 5,
															 "remoteplay",
															 (beken_thread_function_t)thread_h264_sd_PlayBack,
															 16*1024,
															 rp);
									   		if(ret != BK_OK){
												LOGE("err: http post thread create failed\n");
									   		}

										}
									}else{

									}

						}else{
							LOGW("%s %d  sessionId=%s\n", __func__, __LINE__,sessionId); 
							datachannel_msg *rpmsg = (datachannel_msg *)rtc_bk_malloc(sizeof(datachannel_msg));
							if(rpmsg!= NULL){
									rpmsg->streamid = dc_msg->streamid;
									rpmsg->msg = (char*)rtc_bk_malloc(dc_msg->msgsize);
									if(rpmsg->msg!= NULL){
										rtc_bk_memcpy(rpmsg->msg,dc_msg->msg,dc_msg->msgsize);
										rpmsg->msgsize = dc_msg->msgsize;

									}
									rp->msg_list = rtc_list_append(rp->msg_list, (void*)rpmsg);

							}

						}
				}else if(peventName!= NULL && strcmp(peventName,"detection_set")==0 && jrequest!= NULL){
					
					if(jrequest->type == cJSON_Array){
						iCount = cJSON_GetArraySize(jrequest);
							         if(iCount>0){
							              for( i=0; i<iCount; i++){
									if(cJSON_GetArrayItem(jrequest, i)!= NULL){
										cJSON * item = cJSON_GetArrayItem(jrequest, i)->child;
										if(item!= NULL){
											//LOGW("%s %d %s \n", __func__, __LINE__,item->string);
											if(strcmp(item->string,"get_roi_config")==0){
												webrtc_get_roi_config(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"get_move_detect")==0){
												webrtc_get_move_detect(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"set_move_detect")==0){
												webrtc_set_move_detect(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"get_face_detect")==0){
												webrtc_get_face_detect(response_msg_resp,item);
												response_send = 1;
											}else if(strcmp(item->string,"set_person")==0){
												webrtc_set_person(response_msg_resp,item);
												response_send = 1;
											}
										}
									}
								      }
								 }
					}
				}

			}
		}
		if(response_send ==1){
			if(response!=NULL && jsessionId!= NULL && peventName != NULL ){

				cJSON_AddStringToObject(response,"eventName",peventName);
				cJSON_AddItemToObject(response_msg,"response",response_msg_resp);
				cJSON_AddItemToObject(response_data,"message",response_msg);
				cJSON_AddItemToObject(response,"data",response_data);
				send_datachannel_response_message(response,cJSON_GetStringValue(jsessionId),dc_msg->streamid);
				cJSON_Delete(response);
				response = NULL;
			}
		}else{
			if(response!= NULL){
				cJSON_Delete(response);
				response = NULL;
			}
		}
		cJSON_Delete(root);
		root = NULL;

      }
}

static void webrtc_stream_start_play_callback(char *sessionId,size_t sessionId_len,void *user){

}

static void webrtc_stream_event_callback(webrtc_event_type_t event,void *user,int *result)
{
    
     switch(event)
     {
	case WEBRTC_EVENT_ASK_IFRAME:
           //LOGW("-----------WEBRTC_EVENT_ASK_IFRAME-----------------  \n");      
           break;
       case WEBRTC_EVENT_CALL_START:
           //LOGW("-----------WEBRTC_EVENT_CALL_START-----------------  \n");
           break;
       case WEBRTC_EVENT_CALL_LINK:   
            //LOGW("-----------WEBRTC_EVENT_CALL_LINK-----------------  \n");       
           break;
       case WEBRTC_EVENT_CALL_DISCONNECT:
             LOGW("-----------WEBRTC_EVENT_CALL_DISCONNECT-----------------  \n");      
           break;
       case WEBRTC_EVENT_UNLUCK:
           // LOGW("-----------WEBRTC_EVENT_UNLUCK-----------------  \n");
           break;
       case WEBRTC_EVENT_LUCK:
           // LOGW("-----------WEBRTC_EVENT_LUCK-----------------  \n");;
          
           break;
       case WEBRTC_EVENT_LUCK_STATE:
            //LOGW("-----------WEBRTC_EVENT_LUCK_STATE-----------------  \n");
             
           break;
       case WEBRTC_EVENT_LOW_POWER_LEVEL:
            //LOGW("-----------WEBRTC_EVENT_LOW_POWER_LEVEL-----------------  \n");  
           break;
       case WEBRTC_EVENT_CALL_DESTORY:
            //LOGW("-----------WEBRTC_EVENT_CALL_DESTORY-----------------  \n");  
           break;
       case WEBRTC_EVENT_ONLINE:
            LOGW("-----------WEBRTC_EVENT_ONLINE-----------------  \n"); 
	    webrtc_streamer_online = true;  
           break;
       case WEBRTC_EVENT_OFFLINE:
            LOGW("-----------WEBRTC_EVENT_OFFLINE-----------------  \n"); 
	    webrtc_streamer_online = false; 
           break;
       case WEBRTC_EVENT_DATACHANNEL_OPEN:
           // LOGW("-----------WEBRTC_EVENT_DATACHANNEL_OPEN-----------------id = %d  \n",*result);  
           break;
        default:
           break;
     }

}
static void  webrtc_stream_call_income(char *sessionId,size_t sessionId_len,char *szmode,size_t mode_len,char *szsource,size_t source_len,void *user){
      LOGW("webrtc_stream_call_income session Id %s mode = %s szsource = %s\n",sessionId,szmode,szsource);
}
static void  webrtc_stream_call_destory(char *sessionId,size_t len,void *user){
      LOGW("webrtc_stream_call_destory= %s \n",sessionId);

	if(os_strcmp(g_szCouldSessionId,sessionId) == 0){
		memset(g_szCouldSessionId,0,sizeof(g_szCouldSessionId));
		webrtc_cpu1_cloud_publish_stream = false;
		
	}
	Webrtc_remoteplay *rp = get_remoteplay_from_sessionId(sessionId,len);
	if(rp!=NULL){
		rp->stop_= true;
	}
}
static void webrtc_stream_audio_callback(char *data,size_t len,char *sessionId,size_t sessionId_len,void *user){
     // LOGW("sessionId =%s-----------webrtc_stream_audio_callback-----------------%d  \n",sessionId,(int)len);

 
 

}
static void webrtc_stream_mixer_audio_callback(char *data,size_t len,void *user){
      //LOGW("webrtc_stream_mixer_audio_callback     %d\n", (int)len);
	bool single = false;
	rtos_lock_mutex(&audio_mutex);
	if(audio_opened == true && runhua_runing){
	       audio_data *adata = (audio_data *)rtc_bk_malloc(sizeof(audio_data));
	       if(adata!= NULL){
			
				adata->data = (char*)rtc_bk_malloc(len+1);
				if(adata->data!= NULL){
					rtc_bk_memcpy(adata->data,data,len);
					adata->size = len;
					audio_data_list = rtc_list_append(audio_data_list, (void*)adata);
					single = true;

				}else{
					rtc_bk_free(adata);
					adata = NULL;
				}
				

		}
	}
   	rtos_unlock_mutex(&audio_mutex); 
	if(single== true && event_sem!= NULL){
			int count = rtos_get_semaphore_count(&event_sem);
			 if(count == 0){
				   rtos_set_semaphore(&event_sem);
			}
	} 
}
static void webrtc_stream_session_ask_iframe_callback(char *sessionId,size_t sessionId_len,void *user){
     LOGW("webrtc_stream_session_ask_iframe_callback sessionId =%s\n", sessionId);
 
}
static void webrtc_stream_session_pli_callback(char *sessionId,size_t sessionId_len,void *user){
     LOGW("webrtc_stream_session_pli_callback sessionId =%s\n", sessionId);
}
static void webrtc_stream_video_callback(webrtc_stream_type_t stream_type,webrtc_video_code_type_t type,char *data,size_t len,char *sessionId,size_t sessionId_len,void *user){
}
static int webrtc_stream_check_videocode_callback(webrtc_stream_type_t stream_type,char *sessionId,size_t sessionId_len,char *sessionType,size_t sessionType_len,char *szmode,size_t mode_len,char *szsource,size_t source_len,void *user){
    return 0;
}
static void webrtc_stream_message_callback(char *sessionId,size_t sessionId_len,char *ReqMsg,size_t ReqMsg_len,char *RspMsg,size_t *RspMsg_len,void *user){
      LOGW("webrtc_stream_message_callback sessionId = %s \n ReqMsg = %s \n",sessionId,ReqMsg);
}
static void webrtc_stream_datachannel_message_callback(char *sessionId,size_t sessionId_len,webrtc_data_message_type_t type,int streamid,char *Msg,size_t Msg_len,void *user){
     //LOGW("datachannel_message sessionId = %s  Msg = %s \n",sessionId,Msg);
      rtos_lock_mutex(&datachannel_mutex);
       datachannel_msg *dc_msg = (datachannel_msg *)rtc_bk_malloc(sizeof(datachannel_msg));
       if(dc_msg!= NULL){
			dc_msg->streamid = streamid;
			dc_msg->msg = (char*)rtc_bk_malloc(Msg_len+1);
			if(dc_msg->msg!= NULL){
				rtc_bk_memcpy(dc_msg->msg,Msg,Msg_len);
				dc_msg->msgsize = Msg_len;

			}
			dc_msg_list = rtc_list_append(dc_msg_list, (void*)dc_msg);
			if(event_sem!= NULL){
			   int count = rtos_get_semaphore_count(&event_sem);
			   if(count == 0){
		           	rtos_set_semaphore(&event_sem);
			   }
	                }

       }
      rtos_unlock_mutex(&datachannel_mutex);
      
}
static void webrtc_stream_datachnanle_open_callback(char *sessionId,size_t sessionId_len,int streamid,void *user){
      //����ص��Ǹ���datachannel ���¼����û�����ͨ�� webrtc_streamer_datachannel_send_message ��������
      LOGW("webrtc_stream_datachnanle_open_callback sessionId = %s \n",sessionId);
}
static void webrtc_stream_datachnanle_can_add_callback(char *sessionId,size_t sessionId_len,int is_create_offer,void *user)
{  
       //LOGW("webrtc_stream_datachnanle_can_add_callback sessionId = %s is_create_offer = %d\n",sessionId,is_create_offer); 
}
#define WEBRTC_CONFIG_FILE "/flash/cfg/webrtc.conf"
static void  webrtc_streamer_configuration(char *data,size_t len,int reboot){
        int fd = -1;
	int ret;
	fd = vfs_file_open(WEBRTC_CONFIG_FILE, O_RDWR | O_CREAT);
	if (fd < 0) {
		LOGE("open failed  %s, ret=%d\n", WEBRTC_CFG_FILE, fd);
		return;
	}
	ret = vfs_file_write(fd, data, len);
	if(ret<0){
	    	LOGE("write failed to %s, ret=%d\n", WEBRTC_CFG_FILE, ret);
	}else{
		
	}
	 vfs_file_close(fd);
       
}
static int  webrtc_streamer_authentication(char *authdata,size_t authlen,char *password,size_t pwdlen){
	return 1;
}
static void webrtc_stream_alexa_customer_message_callback(char *pnamespace,size_t namespace_len,char *pinstance,size_t instance_len,char *name,size_t name_len,char *alexadirectivemsg,size_t alexadirectivemsg_len,char *resvalue,void *user)
{

}
static void webrtc_stream_webserver_api_callback(const char *url,size_t url_len,const char  *ReqMsg,size_t ReqMsg_len,char *RspMsg,size_t *RspMsg_len,void *user)
{
   LOGW("webrtc_streame_webserver_api_callback %s\n",url);
   if(strcmp((char*)url,"/api/getdevicenumber")==0){
     snprintf((char*)RspMsg,*RspMsg_len,"{\"devicenumber\": \"%s\"}",runSystemCfg.deviceInfo.serialNumber);
     *RspMsg_len = strlen((char*)RspMsg);
   }else{
     snprintf((char*)RspMsg,*RspMsg_len,"%s",ReqMsg);
    *RspMsg_len = strlen((char*)RspMsg);
   }


}
static void webrtc_stream_webserver_websocket_msg_callback(char *sessionId,size_t sessionId_len,char *ReqMsg,size_t ReqMsg_len,char *RspMsg,size_t *RspMsg_len,void *user){
	rtc_bk_memcpy(RspMsg,ReqMsg,ReqMsg_len);
	*RspMsg_len = ReqMsg_len;
}
static void webrtc_stream_network_quality_callback(char *sessionId,size_t sessionId_len,webrtc_network_quality_type_t quality,void *user)
{
     if(quality>WEBRTC_NETWORK_QUALITY_GOOD){
     LOGW("webrtc_stream_network_quality_callback sessionId = %s quality = %d\n",sessionId,quality);
#if 0
     webrtc_streamer_session_info session_info;
     webrtc_streamer_get_session_info(sessionId,&session_info);
     LOGW("\tvideo_send_bitrate = %d\n\taudio_send_bitrate = %d\n\tsend_packets = %d\n\tresend_packets = %d\n\taudio_packet_loss = %d\n\tvideo_packet_loss = %d\n\taudio_current packet_loss = %d\n\tvideo_current packet_loss = %d\n\tcurrent_resend_packets = %d\n",session_info.video_send_bitrate,session_info.audio_send_bitrate,session_info.send_packets,session_info.resend_packets,
    session_info.audio_packet_loss,session_info.video_packet_loss,session_info.audio_current_packet_lost,
    session_info.video_current_packet_lost,session_info.current_resend_packets);
#endif
     }
}
void webrtc_streamer_start(void){

	if(webrtc_streamer_runing == false){
#if 1

        //webrtc_streamer_init("PEU1iWtYJ5uy3NVaEt1EYs997rjvWgvX5lGZVGyCyIBX25+iXq1ihTsub1k6EI7NJku/mjFq8xcWAalAeViohvz32Mr4P+Y/qR8IlKUz9mlDAsC0PAfHZ3wb5NTENUkSnvDeV+lXjAfIgNrZZJL/YsRPJjQjGlW12kTzIFDgR8Q=","",gserialNumber,"webrtc.qq-kan.com","");
        char configuration[256] = {0};
	int fd = -1;
	fd = vfs_file_open(WEBRTC_CONFIG_FILE, O_RDONLY);
        if (fd >= 0) {
		 vfs_file_read(fd, configuration, 256);
		 vfs_file_close(fd);
	}
	webrtc_streamer_init(runSystemCfg.deviceInfo.initString,configuration,runSystemCfg.deviceInfo.serialNumber,runSystemCfg.deviceInfo.serverAddress,"");
	

       //webrtc_streamer_set_log_level_mask(WEBRTC_STREAM_ERROR|WEBRTC_STREAM_FATAL);
	webrtc_streamer_set_log_level_mask(WEBRTC_STREAM_WARNING|WEBRTC_STREAM_ERROR|WEBRTC_STREAM_FATAL);
        //webrtc_streamer_set_log_level_mask(WEBRTC_STREAM_MESSAGE|WEBRTC_STREAM_WARNING|WEBRTC_STREAM_ERROR|WEBRTC_STREAM_FATAL);

        webrtc_streamer_register_event_callback_fun(webrtc_stream_event_callback,NULL);
        webrtc_streamer_register_call_income_callback_fun(webrtc_stream_call_income,NULL);
        webrtc_streamer_register_call_destory_callback_fun(webrtc_stream_call_destory,NULL);
        webrtc_streamer_register_audio_callback_fun(webrtc_stream_audio_callback,NULL);
	webrtc_streamer_register_video_callback_fun(webrtc_stream_video_callback,NULL);
        webrtc_streamer_register_configuration_callback_fun(webrtc_streamer_configuration,NULL);
	webrtc_streamer_register_authentication_callback_fun(webrtc_streamer_authentication,NULL);
	webrtc_streamer_register_message_callback_fun(webrtc_stream_message_callback,4*1024,NULL);
        webrtc_streamer_register_datachannel_message_callback_fun(webrtc_stream_datachannel_message_callback,NULL);
	webrtc_streamer_register_datachannel_open_callback_fun(webrtc_stream_datachnanle_open_callback,NULL);
	webrtc_streamer_register_can_add_datachannel_callback_fun(webrtc_stream_datachnanle_can_add_callback,NULL);
	webrtc_streamer_register_remote_play_start_callback_fun(webrtc_stream_start_play_callback,NULL);
        webrtc_streamer_register_alexa_customer_message_callback_fun(webrtc_stream_alexa_customer_message_callback,NULL);
	webrtc_streamer_register_network_quality_callback_fun(webrtc_stream_network_quality_callback,NULL);
        webrtc_streamer_register_check_videocode_callback_fun(webrtc_stream_check_videocode_callback,NULL);
	webrtc_streamer_register_mixer_audio_callback_fun(webrtc_stream_mixer_audio_callback,NULL);
	webrtc_streamer_register_session_ask_iframe_callback_fun(webrtc_stream_session_ask_iframe_callback,NULL);
        webrtc_streamer_register_session_pli_callback_fun(webrtc_stream_session_pli_callback,NULL);

        webrtc_streamer_set_device_discovery_info("webrtc-webcam","Camera 04","Camera");
#endif
        webrtc_streamer_register_webserver_api_callback_fun(webrtc_stream_webserver_api_callback,4*1024,NULL);
        webrtc_streamer_register_webserver_websocket_messaeg_callback_fun(webrtc_stream_webserver_websocket_msg_callback,4*1024,NULL);
	//webrtc_streamer_webserver_start("/flash/www","/flash/ca.pem","/flash/ca.key",443,80);

        //�ú���������webrtc_streamer_init ���棬webrtc_streamer_init�ỹԭĬ��ֵ

        webrtc_streamer_set_mem_info(2*1024*1024,500*1024,90);
	webrtc_streamer_set_max_channel(3);
	webrtc_streamer_runing = true;
	
	}

}
void webrtc_streamer_stop(void){
	if(webrtc_streamer_runing == true){
		webrtc_media_uninit();
	}

}
#endif
#if CONFIG_SYS_CPU0
static void webrtc_gpio_int_isr(gpio_id_t id)
{
     
    bk_gpio_clear_interrupt(id);
    switch(id)
    {
        case KEY_DEFAULT:
         LOGW("KEY_DEFAULT gpio isr index:%d\n",id);  
        break;
        default:
        break;
    }
}

void webrtc_default_key_init(void)
{
   
    gpio_dev_unmap(KEY_DEFAULT);
    gpio_int_type_t int_type = GPIO_INT_TYPE_FALLING_EDGE;
    gpio_config_t cfg;
    cfg.io_mode =GPIO_INPUT_ENABLE;
    cfg.pull_mode = GPIO_PULL_UP_EN;
    bk_gpio_set_config(KEY_DEFAULT, &cfg);
    bk_gpio_set_interrupt_type(KEY_DEFAULT, int_type);
    bk_gpio_register_isr(KEY_DEFAULT ,webrtc_gpio_int_isr);
    bk_gpio_enable_interrupt(KEY_DEFAULT);

}

static bool is_night_mode()
{
	return g_is_night_mode;
}
static void infrared_lamp_led_init()
{
    	gpio_dev_unmap(IR_LED_PIN_NUMBER);
    	bk_gpio_disable_input(IR_LED_PIN_NUMBER);
    	bk_gpio_enable_output(IR_LED_PIN_NUMBER);
	bk_gpio_set_output_low(IR_LED_PIN_NUMBER);
    	bk_gpio_disable_pull(IR_LED_PIN_NUMBER);
}
static void infrared_lamp_led_on()
{
    	bk_gpio_set_output_high(IR_LED_PIN_NUMBER);
}
static void infrared_lamp_led_off()
{
	bk_gpio_set_output_low(IR_LED_PIN_NUMBER);
}
static void white_light_init(void)
{
	gpio_dev_unmap(FILIGHT_LED_PIN_NUMBER);
    	bk_gpio_disable_input(FILIGHT_LED_PIN_NUMBER);
    	bk_gpio_enable_output(FILIGHT_LED_PIN_NUMBER);
	bk_gpio_set_output_low(FILIGHT_LED_PIN_NUMBER);
    	bk_gpio_disable_pull(FILIGHT_LED_PIN_NUMBER);
}
static void white_light_on(void)
{
	bk_gpio_set_output_high(FILIGHT_LED_PIN_NUMBER);
}
static void white_light_off(void)
{
	bk_gpio_set_output_low(FILIGHT_LED_PIN_NUMBER);
}
static void night_sensor_adc_config(adc_chan_t chan)
{
	adc_config_t config = {0};
	os_memset(&config, 0, sizeof(adc_config_t));

	config.chan = chan;
	config.adc_mode = ADC_CONTINUOUS_MODE;
	config.src_clk = ADC_SCLK_XTAL_26M;
	config.clk = 3203125;
	config.saturate_mode = ADC_SATURATE_MODE_3;
	config.steady_ctrl= 7;
	config.adc_filter = 0;

	BK_LOG_ON_ERR(bk_adc_init(chan));
	BK_LOG_ON_ERR(bk_adc_set_config(&config));
	BK_LOG_ON_ERR(bk_adc_enable_bypass_clalibration());

   
}
static uint32_t adc_get_value(adc_chan_t chan)
{
	uint32_t value = 0;
	night_sensor_adc_config(chan);
    	BK_LOG_ON_ERR(bk_adc_acquire());
	BK_LOG_ON_ERR(bk_adc_start());
	if(bk_adc_set_channel(chan)) {
		BK_LOG_ON_ERR(bk_adc_release());
		return 9999;
	}

	bk_adc_read((uint16_t *)&value, 100);

	//rtos_delay_milliseconds(5);
	BK_LOG_ON_ERR(bk_adc_stop());
	BK_LOG_ON_ERR(bk_adc_release());
	return value;
}

void init_gpio(void){
// ҡͷ��
#if 1
 

	//webrtc_default_key_init();
	gpio_dev_unmap(GPIO_7);
    	bk_gpio_disable_input(GPIO_7);
    	bk_gpio_enable_output(GPIO_7);
    	bk_gpio_set_output_low(GPIO_7);
	//rtos_delay_milliseconds(1);


    	gpio_dev_unmap(GPIO_12);
    	bk_gpio_disable_input(GPIO_12);
    	bk_gpio_enable_output(GPIO_12);
    	bk_gpio_set_output_high(GPIO_12);
	rtos_delay_milliseconds(1);

    	gpio_dev_unmap(SD_CARD_DETECT_PIN);
    	bk_gpio_disable_output(SD_CARD_DETECT_PIN);
    	bk_gpio_enable_input(SD_CARD_DETECT_PIN);
        bk_gpio_enable_pull(SD_CARD_DETECT_PIN);
	bk_gpio_pull_up(SD_CARD_DETECT_PIN);

        //sd_card_detect = bk_gpio_get_value(SD_CARD_DETECT_PIN);

    	gpio_dev_unmap(KEY_DEFAULT);
    	bk_gpio_disable_output(KEY_DEFAULT);
    	bk_gpio_enable_input(KEY_DEFAULT);
        bk_gpio_enable_pull(KEY_DEFAULT);
	bk_gpio_pull_up(KEY_DEFAULT);

        default_key_down = bk_gpio_get_value(KEY_DEFAULT);


	infrared_lamp_led_init();
	infrared_lamp_led_off();

	white_light_init();
	white_light_off();
        rtos_delay_milliseconds(1);

#endif

//������
#if 0
     	gpio_dev_unmap(GPIO_5);
    	bk_gpio_disable_input(GPIO_5);
    	bk_gpio_enable_output(GPIO_5);
    	bk_gpio_set_output_low(GPIO_5);
	delay_us(10);
    	gpio_dev_unmap(GPIO_13);
    	bk_gpio_disable_input(GPIO_13);
    	bk_gpio_enable_output(GPIO_13);
    	bk_gpio_set_output_high(GPIO_13);
	delay_us(10);
#endif


#if 0
	gpio_dev_unmap(GPIO_7);
    	bk_gpio_disable_input(GPIO_7);
    	bk_gpio_enable_output(GPIO_7);
    	bk_gpio_set_output_low(GPIO_7);


    	gpio_dev_unmap(GPIO_13);
    	bk_gpio_disable_input(GPIO_13);
    	bk_gpio_enable_output(GPIO_13);
    	bk_gpio_set_output_high(GPIO_13);
#endif

}
#endif
#if CONFIG_SYS_CPU0
void webrtc_cpu0_list_files(const char *path) {
    DIR *dir = bk_vfs_opendir(path);
    if (dir == NULL) {
        LOGE("failed opendir %s",path);
        return;
    }
    struct dirent *entry;
    while ((entry = bk_vfs_readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // �ļ�
            LOGW("file: %s/%s\n", path,entry->d_name);
        } else if (entry->d_type == DT_DIR) { // Ŀ¼
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char path_buf[256] = {0};
                snprintf(path_buf, sizeof(path_buf), "%s/%s", path, entry->d_name);
                webrtc_cpu0_list_files(path_buf); // �ݹ��г���Ŀ¼�е��ļ�
            }
        }
    }
    bk_vfs_closedir(dir);
}
void webrtc_mount_sdcard(void){
#if CONFIG_VFS
	bk_err_t ret = BK_OK;
	if(sd_card_detect == GPIO_ON && sd_card_mounted == false){
		ret =  tfcard_mount_fatfs("/sdcard/");
		if(ret != BK_OK){
			sd_card_mounted = false;
			LOGE("tfcard_mount_fatfs failed %d\r\n",ret);
		}else{
			sd_card_mounted = true;
			LOGW("tfcard_mount_fatfs sucessed \r\n");
		}
	}
#endif
}
void webrtc_mount_flash(void){
	
#if CONFIG_VFS
	 bk_err_t ret = BK_OK;
 	ret = flash_mount_lfs("/flash/");
	if(ret != BK_OK){
		LOGE("flash_mount_lfs failed then format lfs %d\r\n",ret);
/*
		ret = flash_format_lfs();
		if(ret == BK_OK){
			   //LOGW("flash_mount_lfs OK\r\n");
			   ret = flash_mount_lfs("/flash");
			   if(ret != BK_OK){
				//LOGE("flash_mount_lfs failed %d\r\n",ret);
			   }else{
				LOGW("%s %d flash_mount_lfs sucessed ----\n", __func__, __LINE__);
				delay_us(100);
				CfgInit();
			   }
		}else{
			LOGE("flash_format_lfs failed %d\r\n",ret);
		}
*/

	}else{
		LOGW("%s %d flash_mount_lfs sucessed \n", __func__, __LINE__);
		delay_us(100);
		CfgInit();
		LOGW("%s %d serialNumber = %s\n", __func__, __LINE__,runSystemCfg.deviceInfo.serialNumber);
	}
#endif
}
void webrtc_unmount_sdcard(void){
#if CONFIG_VFS
	 bk_err_t ret = BK_OK;
	if(sd_card_mounted == true){
		ret = umount_vfs("/sdcard/");
		if(ret != BK_OK){
			LOGE("umount_vfs /sdcard/ failed %d\r\n",ret);
		}
		sd_card_mounted = false;
	}
#endif
}
void webrtc_unmount_flash(void){
#if CONFIG_VFS
	bk_err_t ret = BK_OK;
	//CfgUnInit();
        ret = umount_vfs("/flash/");
	if(ret != BK_OK){
		LOGE("umount_vfs /flash/ failed %d\r\n",ret);
		
	}
#endif
}
#endif
#if CONFIG_SYS_CPU1

void webrtc_cpu1_list_files(const char *path) {
    DIR *dir = vfs_file_opendir(path);
    if (dir == NULL) {
        LOGE("failed opendir %s",path);
        return;
    }
 
    struct dirent *entry;
    while ((entry = vfs_file_readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // �ļ�
            LOGW("file: %s/%s\n", path,entry->d_name);
        } else if (entry->d_type == DT_DIR) { // Ŀ¼
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char path_buf[256] = {0};
                snprintf(path_buf, sizeof(path_buf), "%s/%s", path, entry->d_name);
                webrtc_cpu1_list_files(path_buf); // �ݹ��г���Ŀ¼�е��ļ�
            }
        }
    }

    vfs_file_closedir(dir);
}

void webrtc_sdcard_free(const char *path) {

	bk_err_t ret = BK_OK;
	struct statfs statfsinfo ={0};
	os_memset(&statfsinfo,0,sizeof(struct statfs));
	ret = vfs_file_statfs(path,&statfsinfo);
	if(ret==0){
		        //unsigned long total = statfsinfo.f_blocks * statfsinfo.f_bsize; // �ܴ�С
			//unsigned long free = statfsinfo.f_bfree * statfsinfo.f_bsize; // ʣ���С
			//unsigned long used = total - free; // ��ʹ�ô�С
		   	LOGW("webrtc_sdcard_free  f_bsize: %lu\n", statfsinfo.f_bsize);
			LOGW("webrtc_sdcard_free  f_blocks: %lu\n", statfsinfo.f_blocks);
			LOGW("webrtc_sdcard_free  f_bfree: %lu\n", statfsinfo.f_bfree);
			//LOGW("%s Total space: %llu\n", path,total/2);
			//LOGW("%s Free  space: %llu\n", path,free/2);
			//LOGW("%s Used space: %llu\n", path,used);
	}else{
		LOGW("failed statfs %s",path);
	}
}

void webrtc_sdcard_info(const char *path,int *total_space,int *free_space) {
     if(sdcard_mounted == 1){

	bk_err_t ret = BK_OK;
	struct statfs statfsinfo ={0};
	os_memset(&statfsinfo,0,sizeof(struct statfs));
	ret = vfs_file_statfs(path,&statfsinfo);
	if(ret==0){
		        unsigned long total = statfsinfo.f_blocks/1024 * statfsinfo.f_bsize; // �ܴ�С
			unsigned long free = statfsinfo.f_bfree/1024 * statfsinfo.f_bsize; // ʣ���С
			//unsigned long used = (statfsinfo.f_blocks-statfsinfo.f_bfree)/1024 * statfsinfo.f_bsize;
		   	//LOGW("webrtc_sdcard_free  f_bsize: %lu\n", statfsinfo.f_bsize);
			//LOGW("webrtc_sdcard_free  f_blocks: %lu\n", statfsinfo.f_blocks);
			//LOGW("webrtc_sdcard_free  f_bfree: %lu\n", statfsinfo.f_bfree);
			//LOGW("webrtc_sdcard_free  total: %lu\n", total);
			//LOGW("webrtc_sdcard_free  free: %lu\n", free);
			//LOGW("webrtc_sdcard_free  used: %lu\n", used);
			*total_space = (int)(total/1024);
			*free_space = (int)(free/1024);
	}else{
		LOGW("failed statfs %s",path);
	}


     }
}
int isFolderEmpty(const char* path) {
    struct stat st;
    if (vfs_file_stat(path, &st) == -1) {
        LOGW("stat error");
        return -1;
    }
    return (st.st_size == 0) ? 1 : 0;
}
void webrtc_clean_record_queue(){
	bk_err_t ret = BK_OK;
	queue_msg_t msg;
	if(webrtc_record_queue!=NULL ){
		while(!rtos_is_queue_empty(&webrtc_record_queue) ){
			ret = rtos_pop_from_queue(&webrtc_record_queue, &msg, 0);
			if (kNoErr == ret){
				record_data * ptempdata = (record_data *)msg.param;
					if(ptempdata!= NULL){
						destory_record_data(ptempdata);
					}
			}else{
				break;
			}
		}
	}

}

#if CONFIG_WEBRTC_AVI
void webrtc_record_thread(void *param){
	bk_err_t ret = BK_OK;
	webrtc_avi_t *avi_writer = NULL;
	webrtc_record_runing = true;
	char szfilename[128] ={0};
	char szfilepath[128] ={0};
        bool change_file = false;
        bool can_write = false;
	bool is_write_audio = false;
	bool queue_empty = false;
	char stime[32] ={0};
	char sdate[32] ={0};
	uint32_t start_timestamp = get_cur_timestamp();
	uint32_t now;
	uint32_t max_delay = 60*1000;
	uint32_t usetime;
	uint32_t delay;
        int result = 0;
	int audio_outbuf_size = 320;
	uint8_t *audio_outbuf = (uint8_t *)rtc_bk_malloc(audio_outbuf_size);
	if(audio_outbuf!= NULL){
	}
        int i = 0;
	webrtc_can_record = false;
	queue_msg_t msg;
	while(webrtc_record_runing  && webrtc_record_sem!= NULL){
		rtos_get_semaphore(&webrtc_record_sem, BEKEN_WAIT_FOREVER);
		while(webrtc_record_runing){
			rtos_lock_mutex(&webrtc_record_queue_mutex);		
			if(record_data_list == NULL){
				rtos_unlock_mutex(&webrtc_record_queue_mutex);
				break;
			}
			if(webrtc_record_skip == true){
				rtos_unlock_mutex(&webrtc_record_queue_mutex);
				del_all_record_data();
				webrtc_record_skip = false;
				if(can_write == true){
					can_write = false;
				}
				
				break;
			}
			record_data *pdata = (record_data *)rtc_list_nth_data(record_data_list,0);
			if(pdata!= NULL){
				record_data_list = rtc_list_remove(record_data_list,pdata);
			        rtos_unlock_mutex(&webrtc_record_queue_mutex);
				if(pdata!= NULL){

					if(webrtc_recording && sdcard_mounted ==1){
						is_write_audio = false;
						change_file = false;
					        now = get_cur_timestamp();
						if(can_write == true && now-start_timestamp>max_delay &&  pdata->isvideo == true && pdata->data!= NULL && pdata->size>4){
						    if(motion_detection_recording == false){
							    uint8_t frametype;
					                    frametype = (pdata->data[4]& 0x1f);
							    if(frametype == 0x07){
								LOGW("%s %d stop write avi file ---- %s \n", __func__, __LINE__,szfilename);
								rtos_lock_mutex(&webrtc_record_queue_mutex);
								webrtc_can_record = false;
								rtos_unlock_mutex(&webrtc_record_queue_mutex);
								WEBRTC_AVI_close(avi_writer);
								del_all_record_data();
								snprintf(current_record_filename,sizeof(current_record_filename),"%s","");
								avi_writer = NULL;
								can_write = false;
								change_file = true;
								LOGW("%s %d stop write avi file %s \n", __func__, __LINE__,szfilename);
							    }
						   }
						}
						if(avi_writer == NULL){
							//webrtc_sdcard_free("/sdcard/");

							can_write = false;
							time_timestr_get(stime,sizeof(stime));
							time_datestr_get(sdate,sizeof(sdate));
							snprintf(szfilepath,sizeof(szfilepath),"/sdcard/record/%s/",sdate);
							checkMkdir(szfilepath);
							snprintf(szfilename,sizeof(szfilename),"%s%02d-%s.avi",szfilepath,webrtc_record_event,stime);
							avi_writer = WEBRTC_AVI_open_output_file(szfilename);
							if(avi_writer){
								WEBRTC_AVI_set_video(avi_writer,video_width,video_height,video_fps, "H264");
								long mp3rate = audio_samp_rate*16/(1*1000);
								WEBRTC_AVI_set_audio(avi_writer,1,audio_samp_rate,16,WAVE_FORMAT_MULAW,mp3rate);

							
								rtos_lock_mutex(&webrtc_record_queue_mutex);
								webrtc_can_record = true;
								rtos_unlock_mutex(&webrtc_record_queue_mutex);
								start_timestamp = get_cur_timestamp();
								if(record_snapshot == false){
									record_snapshot = true;
									snprintf(record_snapshot_filename,sizeof(record_snapshot_filename),"%s%02d-%s.jpg",szfilepath,webrtc_record_event,stime);
								}
								snprintf(current_record_filename,sizeof(current_record_filename),"%s",szfilename);
								LOGW("%s %d create avi writer %s sucessed \n", __func__, __LINE__,szfilename);
							}

						}
						if(change_file==false && can_write == false && pdata->isvideo == true && pdata->data!= NULL && pdata->size>4){

						    uint8_t frametype;
			                            frametype = (pdata->data[4]& 0x1f);
						    if(frametype == 0x07){
							can_write = true;
							LOGW("%s %d start write avi file %s \n", __func__, __LINE__,szfilename);

						    }

						}
						
						if(can_write == true && avi_writer!= NULL){
							if(pdata->isvideo == true && pdata->size>0){
							     int keyframe = 0;
							     uint8_t frametype;
					                     frametype = (pdata->data[4]& 0x1f);
							     if(frametype == 0x07){
									keyframe = 1;
							     }
								//LOGW("%s %d write video\n", __func__, __LINE__);
								result =WEBRTC_AVI_write_frame(avi_writer,(char *)pdata->data,pdata->size,keyframe);
								if(result<0){
									LOGE("%s %d write video error %d\n", __func__, __LINE__,result);
									webrtc_recording = false;
								}
								
							}else{
								if(pdata->size>0){

									uint8_t * buffer = (uint8_t *)pdata->data;
									if(audio_outbuf==NULL || audio_outbuf_size!= pdata->size){
										if(audio_outbuf!= NULL){
											rtc_bk_free(audio_outbuf);
											audio_outbuf = NULL;
										}
										
										audio_outbuf = (uint8_t *)rtc_bk_malloc(pdata->size);
										if(audio_outbuf!= NULL){
											audio_outbuf_size = pdata->size;
										}
									}
									if(audio_outbuf!=NULL){
										uint8_t *tempbuf = audio_outbuf;
										for (i=0;i<pdata->size/2;i++){
											*tempbuf=s16_to_ulaw(((int16_t*)buffer)[i]);
											 tempbuf++;
											
												       
										}
										//LOGW("%s %d write audio\n", __func__, __LINE__);
										result = WEBRTC_AVI_write_audio(avi_writer,(char *)audio_outbuf,pdata->size/2);
										if(result<0){
											LOGE("%s %d write audio error %d\n", __func__, __LINE__,result);
											webrtc_recording = false;
										}
										is_write_audio = true;
										
									}

								}
								
							}
						}
						

					}else{
						if(avi_writer!=NULL){
							can_write = false;
							rtos_lock_mutex(&webrtc_record_queue_mutex);
							webrtc_can_record = false;
							rtos_unlock_mutex(&webrtc_record_queue_mutex);
							LOGW("%s %d stop write avi file %s \n", __func__, __LINE__,szfilename);
							WEBRTC_AVI_close(avi_writer);
							if(motion_detection_recording){
								motion_detection_recording = false;

							}
							snprintf(current_record_filename,sizeof(current_record_filename),"%s","");
							//webrtc_can_record = true;
							
							avi_writer = NULL;
						}
					}
					destory_record_data(pdata);
					rtos_delay_milliseconds(1);
								
			        }
			}
			else
			{
				rtos_unlock_mutex(&webrtc_record_queue_mutex);
				break;
			}


		}
		if(webrtc_recording == false && avi_writer!=NULL){
				if(avi_writer!=NULL){
						can_write = false;
						LOGW("%s %d stop write avi file %s \n", __func__, __LINE__,szfilename);
						rtos_lock_mutex(&webrtc_record_queue_mutex);
						webrtc_can_record = false;
						rtos_unlock_mutex(&webrtc_record_queue_mutex);
						WEBRTC_AVI_close(avi_writer);
						snprintf(current_record_filename,sizeof(current_record_filename),"%s","");
						avi_writer = NULL;
						if(motion_detection_recording){
						    motion_detection_recording = false;
						}
				}
		}


		
	}
	if(webrtc_record_exit_sem!= NULL){
			int count = rtos_get_semaphore_count(&webrtc_record_exit_sem);
			if(count == 0){				
				rtos_set_semaphore(&webrtc_record_exit_sem);
			}
	}
	LOGW("%s %d thread exit\n", __func__, __LINE__);
	webrtc_recording = false;
	webrtc_can_record = false;
	snprintf(current_record_filename,sizeof(current_record_filename),"%s","");
	if(avi_writer!=NULL){
		WEBRTC_AVI_close(avi_writer);
		avi_writer = NULL;
	}
	if(audio_outbuf!= NULL){
		rtc_bk_free(audio_outbuf);
		audio_outbuf = NULL;
	}
	del_all_record_data();
	if(webrtc_record_sem!= NULL){
		rtos_deinit_semaphore(&webrtc_record_sem);
		webrtc_record_sem = NULL;
	}


	if (webrtc_record_queue!= NULL)
	{
		rtos_deinit_queue(&webrtc_record_queue);
		webrtc_record_queue = NULL;
	}
	if(webrtc_record_queue_mutex!= NULL){
		rtos_deinit_mutex(&webrtc_record_queue_mutex);
		webrtc_record_queue_mutex =NULL;
	}
     
	webrtc_record_thread_hdl = NULL;
	rtos_delete_thread(NULL);




}
#endif
#if CONFIG_WEBRTC_MP4
void webrtc_record_thread(void *param){
	bk_err_t ret = BK_OK;
	webrtc_mp4_writer* mp4_writer = NULL;
	webrtc_record_runing = true;
	char szfilename[128] ={0};
	char szfilepath[128] ={0};
        bool change_file = false;
        bool can_write = false;
	bool is_write_audio = false;
	bool queue_empty = false;
	char stime[32] ={0};
	char sdate[32] ={0};
	uint32_t start_timestamp = get_cur_timestamp();
	uint32_t now;
	uint32_t max_delay = 60*1000;
	uint32_t usetime;
	uint32_t delay;
        int result = 0;
	int audio_outbuf_size = 320;
	uint8_t *audio_outbuf = (uint8_t *)rtc_bk_malloc(audio_outbuf_size);
	if(audio_outbuf!= NULL){
	}

	webrtc_can_record = false;
	queue_msg_t msg;
	while(webrtc_record_runing  && webrtc_record_sem!= NULL){
		rtos_get_semaphore(&webrtc_record_sem, BEKEN_WAIT_FOREVER);
		while(webrtc_record_runing){
			rtos_lock_mutex(&webrtc_record_queue_mutex);		
			if(record_data_list == NULL){
				rtos_unlock_mutex(&webrtc_record_queue_mutex);
				break;
			}
			if(webrtc_record_skip == true){
				rtos_unlock_mutex(&webrtc_record_queue_mutex);
				del_all_record_data();
				webrtc_record_skip = false;
				if(can_write == true){
					can_write = false;
				}
				
				break;
			}
			record_data *pdata = (record_data *)rtc_list_nth_data(record_data_list,0);
			if(pdata!= NULL){
				record_data_list = rtc_list_remove(record_data_list,pdata);
			        rtos_unlock_mutex(&webrtc_record_queue_mutex);
				if(pdata!= NULL){

					if(webrtc_recording && sdcard_mounted ==1){
						is_write_audio = false;
						change_file = false;
					        now = get_cur_timestamp();
						if(can_write == true && now-start_timestamp>max_delay &&  pdata->isvideo == true && pdata->data!= NULL && pdata->size>4){
						    uint8_t frametype;
			                            frametype = (pdata->data[4]& 0x1f);
						    if(frametype == 0x07){
							LOGW("%s %d stop write mp4 file ---- %s \n", __func__, __LINE__,szfilename);
							rtos_lock_mutex(&webrtc_record_queue_mutex);
							webrtc_can_record = false;
							rtos_unlock_mutex(&webrtc_record_queue_mutex);
							webrtc_mp4_writer_destroy(mp4_writer);
							snprintf(current_record_filename,sizeof(current_record_filename),"%s","");
							del_all_record_data();
							mp4_writer = NULL;
							can_write = false;
							change_file = true;
							LOGW("%s %d stop write mp4 file %s \n", __func__, __LINE__,szfilename);
						    }
						}
						if(mp4_writer == NULL){
							//webrtc_sdcard_free("/sdcard/");

							can_write = false;
							time_timestr_get(stime,sizeof(stime));
							time_datestr_get(sdate,sizeof(sdate));
							snprintf(szfilepath,sizeof(szfilepath),"/sdcard/record/%s/",sdate);
							checkMkdir(szfilepath);
							snprintf(szfilename,sizeof(szfilename),"%s%02d-%s.mp4",szfilepath,webrtc_record_event,stime);
							mp4_writer = webrtc_mp4_writer_create(szfilename,video_width,video_height,video_fps,1,16,audio_samp_rate);
							if(mp4_writer!= NULL){

								if(record_snapshot == false){
									record_snapshot = true;
									snprintf(record_snapshot_filename,sizeof(record_snapshot_filename),"%s%02d-%s.jpg",szfilepath,webrtc_record_event,stime);
								}

								rtos_lock_mutex(&webrtc_record_queue_mutex);
								webrtc_can_record = true;
								rtos_unlock_mutex(&webrtc_record_queue_mutex);
								start_timestamp = get_cur_timestamp();
								snprintf(current_record_filename,sizeof(current_record_filename),"%s",szfilename);
								LOGW("%s %d create mp4 writer %s sucessed \n", __func__, __LINE__,szfilename);

							}else{
								webrtc_recording = false;
								LOGE("%s %d failed create mp4 writer %s \n", __func__, __LINE__,szfilename);
							}

						}
						if(change_file==false && can_write == false && pdata->isvideo == true && pdata->data!= NULL && pdata->size>4){

						    uint8_t frametype;
			                            frametype = (pdata->data[4]& 0x1f);
						    if(frametype == 0x07){
							can_write = true;
							LOGW("%s %d start write mp4 file %s \n", __func__, __LINE__,szfilename);

						    }

						}
						
						if(can_write == true && mp4_writer!= NULL){
							if(pdata->isvideo == true && pdata->size>0){
								//LOGW("%s %d write video\n", __func__, __LINE__);
								result =webrtc_mp4_writer_writeframe(mp4_writer,WEBRTC_MP4_OBJECT_H264,(const uint8_t*)pdata->data,pdata->size);
								if(result<0){
									LOGE("%s %d write video error %d\n", __func__, __LINE__,result);
									webrtc_recording = false;
								}
								
							}else{
								if(pdata->size>0){

									uint8_t * buffer = (uint8_t *)pdata->data;
									if(audio_outbuf==NULL || audio_outbuf_size!= pdata->size){
										if(audio_outbuf!= NULL){
											rtc_bk_free(audio_outbuf);
											audio_outbuf = NULL;
										}
										
										audio_outbuf = (uint8_t *)rtc_bk_malloc(pdata->size);
										if(audio_outbuf!= NULL){
											audio_outbuf_size = pdata->size;
										}
									}
									int i;
									if(audio_outbuf!=NULL){
										uint8_t *tempbuf = audio_outbuf;
										for (i=0;i<pdata->size/2;i++){
											*tempbuf=s16_to_ulaw(((int16_t*)buffer)[i]);
											 tempbuf++;
												       
										}
										//LOGW("%s %d write audio\n", __func__, __LINE__);
										result = webrtc_mp4_writer_writeframe(mp4_writer,WEBRTC_MP4_OBJECT_ULAW,(const uint8_t*)audio_outbuf,pdata->size/2);
										if(result<0){
											LOGE("%s %d write audio error %d\n", __func__, __LINE__,result);
											webrtc_recording = false;
										}
										is_write_audio = true;
										
									}

								}
								
							}
						}
						

					}else{
						if(mp4_writer!=NULL){
							can_write = false;
							rtos_lock_mutex(&webrtc_record_queue_mutex);
							webrtc_can_record = false;
							rtos_unlock_mutex(&webrtc_record_queue_mutex);
							LOGW("%s %d stop write mp4 file %s \n", __func__, __LINE__,szfilename);
							webrtc_mp4_writer_destroy(mp4_writer);
							snprintf(current_record_filename,sizeof(current_record_filename),"%s","");
							//webrtc_can_record = true;
							
							mp4_writer = NULL;
						}
					}
					destory_record_data(pdata);
								
			        }
			}
			else
			{
				rtos_unlock_mutex(&webrtc_record_queue_mutex);
				break;
			}


		}
		if(webrtc_recording == false && mp4_writer!=NULL){
				if(mp4_writer!=NULL){
						can_write = false;
						LOGW("%s %d stop write mp4 file %s \n", __func__, __LINE__,szfilename);
						rtos_lock_mutex(&webrtc_record_queue_mutex);
						webrtc_can_record = false;
						rtos_unlock_mutex(&webrtc_record_queue_mutex);
						webrtc_mp4_writer_destroy(mp4_writer);
						snprintf(current_record_filename,sizeof(current_record_filename),"%s","");
						del_all_record_data();
						//webrtc_can_record = true;
						mp4_writer = NULL;
				}
		}


		
	}
	if(webrtc_record_exit_sem!= NULL){
			int count = rtos_get_semaphore_count(&webrtc_record_exit_sem);
			if(count == 0){				
				rtos_set_semaphore(&webrtc_record_exit_sem);
			}
	}
	LOGW("%s %d thread exit\n", __func__, __LINE__);
	webrtc_recording = false;
	webrtc_can_record = false;
	snprintf(current_record_filename,sizeof(current_record_filename),"%s","");
	if(mp4_writer!=NULL){
		webrtc_mp4_writer_destroy(mp4_writer);
		mp4_writer = NULL;
	}
	if(audio_outbuf!= NULL){
		rtc_bk_free(audio_outbuf);
		audio_outbuf = NULL;
	}
	del_all_record_data();
	if(webrtc_record_sem!= NULL){
		rtos_deinit_semaphore(&webrtc_record_sem);
		webrtc_record_sem = NULL;
	}


	if (webrtc_record_queue!= NULL)
	{
		rtos_deinit_queue(&webrtc_record_queue);
		webrtc_record_queue = NULL;
	}
	if(webrtc_record_queue_mutex!= NULL){
		rtos_deinit_mutex(&webrtc_record_queue_mutex);
		webrtc_record_queue_mutex =NULL;
	}
     
	webrtc_record_thread_hdl = NULL;
	rtos_delete_thread(NULL);




}
#endif
#endif
#if CONFIG_SYS_CPU0
static int scan_wifi_event_cb(void *arg, event_module_t event_module,
								  int event_id, void *event_data)
{
	
	wifi_scan_result_t scan_result = {0};

	BK_LOG_ON_ERR(bk_wifi_scan_get_result(&scan_result));
	LOGW("%s %d scan ap = %d\n", __func__, __LINE__,scan_result.ap_num);
	//BK_LOG_ON_ERR(bk_wifi_scan_dump_result(&scan_result));
	bk_wifi_scan_free_result(&scan_result);

	return BK_OK;
}
static void wifi_scan_init(char *ssid)
{
        LOGW("%s %d \n", __func__, __LINE__);
	BK_LOG_ON_ERR(bk_event_register_cb(EVENT_MOD_WIFI, EVENT_WIFI_SCAN_DONE,
							   scan_wifi_event_cb, NULL));
#if 0
	BK_LOG_ON_ERR(bk_wifi_scan_start(NULL));
#else
	wifi_scan_config_t scan_config = {0};
	snprintf(scan_config.ssid,sizeof(scan_config.ssid),"%s",ssid);
	scan_config.duration = 8000;
	BK_LOG_ON_ERR(bk_wifi_scan_start(&scan_config));
#endif
	LOGW("%s %d end\n", __func__, __LINE__);
}
const char cert_rsa[] = {
"-----BEGIN CERTIFICATE-----\r\n"
"MIIEDDCCAvSgAwIBAgIBATANBgkqhkiG9w0BAQsFADB5MQswCQYDVQQGEwJDTjES\r\n"
"MBAGA1UECAwJR3Vhbmdkb25nMREwDwYDVQQHDAhTaGVuemhlbjEYMBYGA1UECgwP\r\n"
"UnVuaHVhbGluayBJbmMuMQ8wDQYDVQQLDAZSdW5odWExGDAWBgNVBAMMD3dlYnJ0\r\n"
"Yy1zdHJlYW1lcjAgFw0yMzAzMDMwNDQyMDdaGA8yMTIzMDIwNzA0NDIwN1oweTEL\r\n"
"MAkGA1UEBhMCQ04xEjAQBgNVBAgMCUd1YW5nZG9uZzERMA8GA1UEBwwIU2hlbnpo\r\n"
"ZW4xGDAWBgNVBAoMD1J1bmh1YWxpbmsgSW5jLjEPMA0GA1UECwwGUnVuaHVhMRgw\r\n"
"FgYDVQQDDA93ZWJydGMtc3RyZWFtZXIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAw\r\n"
"ggEKAoIBAQC7t2mGIanJN7JwRx+TzL3vbpgkcwQ/XxA+Y92gWxVC4WO2GTm2f3/o\r\n"
"h0QIjN9U5WChSVDzP6HHfs3FEPcgWJolNb7okYDbgt/tUz1dSTTz77QflOcOga8f\r\n"
"MCpUYZ4LY+mAxowpn/2UZNt2NE/unu+DPKNZMWa1hpvAftJ1q2QrsxscgSXvomj+\r\n"
"bgDPvt/nLUNbq9qwZix9OljRzZoxClkIKsYNdMJ5sjV5yLfjzb1Pe4+vFHMtdGrm\r\n"
"9KCMXHyLu1vPfYVzM63CrE1GoZRnm7+4GyofvsMvYWoiip8Z7IaZraSqSnMQ6KLb\r\n"
"JJ72nlQXmzKZn4qaihVKoPEz6nPPTQ5jAgMBAAGjgZwwgZkwCQYDVR0TBAIwADAd\r\n"
"BgNVHQ4EFgQUOQxl70tqB5dAckah2XlNK9u7WjwwHwYDVR0jBBgwFoAUcIrVmhTR\r\n"
"c79os+PJ2CxY/aLj/GUwTAYDVR0RBEUwQ4cEfwAAAYcEwKgBeoIqM2FhNmYwNDgt\r\n"
"Y2RkMi00ZTM0LTllZDItYzRlZDAyZWI4ZTNkLmxvY2Fsgglsb2NhbGhvc3QwDQYJ\r\n"
"KoZIhvcNAQELBQADggEBABCsJfqZ1N0GE6g/K3TF+m6+AQtlLty8RvkmkiuTTnx2\r\n"
"Su1nT+g4DfOrFmnKiBvWAfXI9c4IdafoZV/yPjIJrh/TbxdPsUvvEG16TFfYR+ZV\r\n"
"SYERAlC08QopWes1hHvQzfvGw8LCg/uVHk8WDUn8QaZQ/e5vYXdIDRoQYtwaCrMZ\r\n"
"wVdL/BOPiDFeaW2Zy6hvi2saTlCXllIkMoXtoD/kP5ConUr3DNQ8Pgo+uPuptDP3\r\n"
"Ed/+vpdlAmBUo51o1EMhtcSLlmHMP/i6q4tMMiqXSkVvv7CQYTl52V8a9+Lta9B9\r\n"
"Hv5AjDeOgoRA/m0Ma77+2NKEsjCDNAaz+MCAYv0zCgM=\r\n"
"-----END CERTIFICATE-----\r\n"
};
const char cert_key[] = {
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEogIBAAKCAQEAu7dphiGpyTeycEcfk8y9726YJHMEP18QPmPdoFsVQuFjthk5\r\n"
"tn9/6IdECIzfVOVgoUlQ8z+hx37NxRD3IFiaJTW+6JGA24Lf7VM9XUk08++0H5Tn\r\n"
"DoGvHzAqVGGeC2PpgMaMKZ/9lGTbdjRP7p7vgzyjWTFmtYabwH7SdatkK7MbHIEl\r\n"
"76Jo/m4Az77f5y1DW6vasGYsfTpY0c2aMQpZCCrGDXTCebI1eci34829T3uPrxRz\r\n"
"LXRq5vSgjFx8i7tbz32FczOtwqxNRqGUZ5u/uBsqH77DL2FqIoqfGeyGma2kqkpz\r\n"
"EOii2ySe9p5UF5symZ+KmooVSqDxM+pzz00OYwIDAQABAoIBAAgUhAlPB6TurdKh\r\n"
"bR2KkZ5tz0S6YrNrnyJgYBjX8tlXJFihS1ess0/KCsancbGQ0hD/O+nMkmUo8yiJ\r\n"
"a2nMBo6Veqs+ICLSxlrL23dZgFsDtWgqm6uHL4eTJi9NqGTrHxD4+u2LwnmRwdSb\r\n"
"udkWpE6s3IcpVjd4HNozW1HPV9d9ohu6eAJKA5Nee2uUNpikBzqDJXRQ4tYT3Z6Z\r\n"
"aH4Fdcnjy34rtNfPRYhq21ZikPyDHaQnjSk568hw7jloXMIEVRIydvVRTnz0KhIp\r\n"
"ntNkOks5TMCybk8tsKWnajUpQMEP0QkqKy56AuiRvzz8TR0X1CJ3obw9XE+smwuP\r\n"
"0DwI/RECgYEA/MWnQFGkKVAOTN8YviHrlCeDDDJR7arOQhI89SFWM+oQukE5mEvJ\r\n"
"vVLMkLYaKu/vF9GyHdqV540/YtRT5KJVTHiStB9CEggy9QwjJyN9AMzhubEXCru1\r\n"
"MqFS0SzIWOc05ICycsGZ0rgaMe8a0Lndmvx5oGDkOwzPCzhMDAHFh8sCgYEAvh0V\r\n"
"RRsh7MsivUgSj1EK4m1jKRuWLsNLWziHj8mPUY1lg39ZeSyJVZsEKT5oZhTDW1hr\r\n"
"vhEpBC3+UtUcRUvxWZ0yxEerNJ13WI/TL7zmmfGF2eG0+BoDiD09/OGqT8oyI84x\r\n"
"JwAoe4HQF6jnx9hYqOt8G2wYU5F/QJ/55d2SUMkCgYA1pGx+BdVkvwyJ276QevpX\r\n"
"kpsI95TTbzAebWhqTQzSL5YlMLpcS5kgiHXJMBwViJ2g3GuEUmMFpMAS5SR4nMql\r\n"
"U+EuQIPw97R4tH6xS6K3jMNKeP5+1J77g6jjozFRTJ+47mbwW42dXlyQxEFYklkp\r\n"
"DvNwyZ8luO4nX2ckFSwqSQKBgFXbUojECR0kXAr5apBYvD6nwfmFoNx7jCOlMuuH\r\n"
"zna5EZhCQgkMSPVlLYrmyUUYqWUuWHIc09Y0Yz/LSJovAs9Cw/OKnlIDrytKwMg1\r\n"
"Wjs5rQZJ/W1yahfz+HOlAkJIgT5UVzRSyPWGGZEl0Y8aMGgQ+Rp1RcMv2TU5SiU6\r\n"
"XGKpAoGAKoQS8rVxDGg5dj/JGuBT7J6Sl7yhPyqlDsf/TtphFCIr54i4kevwEAPA\r\n"
"UYneoTM9uQ8q734t4hatJ2H9XMqQ83epDW+0iqr1YvUoD8Ms9cAOkAmYwzXndjYe\r\n"
"UjjhEDpSX05Yio90YLo41zsumJRblO/0PiwM762vb07KpbCGMLk=\r\n"
"-----END RSA PRIVATE KEY-----\r\n"
};
#define WWW_GET_NET_CONFIG    1
#define WWW_GET_SERNO         2
typedef void(*www_callback)(void *msg,const char *data,int len);
typedef struct _mg_send_msg{
	char *url;
        char *post_data;
	int   post_size;
	int   message_id;
	bool get;
	uint32_t start;
	uint32_t delay;
	bool done;
	www_callback callback;
	
}mg_send_msg;
static void webrtc_www_callback(void* owner,const char *data,int len){
	mg_send_msg *postmsg = (mg_send_msg *)owner;
	if(postmsg->message_id == WWW_GET_NET_CONFIG){

	}else if(postmsg->message_id == WWW_GET_SERNO){
		cJSON *root =  cJSON_Parse(data);
		if(root!= NULL){
			cJSON *jcode = cJSON_GetObjectItem(root,"code");
			if(jcode!=NULL){
				int code = (int)cJSON_GetNumberValue(jcode);
				if(code == 200){
					cJSON *jresult = cJSON_GetObjectItem(root,"result");
					if(jresult!= NULL){
						cJSON *jinitString = cJSON_GetObjectItem(jresult,"inittsring");
						cJSON *jserno = cJSON_GetObjectItem(jresult,"serno");
						if(jinitString!= NULL && jserno!= NULL){
							char *initString = cJSON_GetStringValue(jinitString);
							char *serno = cJSON_GetStringValue(jserno);
							if(initString!= NULL && serno!= NULL){
								webrtc_write_system_info(initString,"www.qq-kan.com",serno);
							}

						}

					}
				}

			}
			
			cJSON_Delete(root);

		}

	
	}

}
// Print HTTP response and signal that we're done
static void webrtc_mg_send_msg_free(mg_send_msg* postmsg){
	if(postmsg!= NULL){
		if(postmsg->url!= NULL){
		     rtc_bk_free(postmsg->url);
		     postmsg->url = NULL;
		}
		if(postmsg->post_data!= NULL){
		     rtc_bk_free(postmsg->post_data);
		     postmsg->post_data = NULL;
		}
		rtc_bk_free(postmsg);
		postmsg = NULL;

	}

}
static void send_ev_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  
  mg_send_msg *psendmsg = (mg_send_msg *)fn_data;
  if(psendmsg!= NULL){
	  if (ev == MG_EV_CONNECT) {
	    LOGW("%s %d MG_EV_CONNECT \n", __func__, __LINE__);
	    // Connected to server. Extract host name from URL
	    if(psendmsg->url!= NULL){
		    struct mg_str host = mg_url_host(psendmsg->url);
		    if (mg_url_is_ssl(psendmsg->url)) {
		      struct mg_tls_opts opts = {.ca = NULL,
						 .srvname = host,
						 .cert = cert_rsa, 
						 .certkey = cert_key};


		      mg_tls_init(c, &opts);
		    }
		    LOGW("%s %d \n", __func__, __LINE__);
		    // Send request
		    if(psendmsg->get){
			    	    LOGW("%s %d http get %s \n", __func__, __LINE__,mg_url_uri(psendmsg->url));
				    mg_printf(c,
					      "GET %s HTTP/1.0\r\n"
					      "Host: %.*s\r\n"
					      "\r\n",
					      mg_url_uri(psendmsg->url), (int) host.len, host.ptr);
			   
		    }else{
			    if(psendmsg->post_data!= NULL && psendmsg->post_size>0){
				    mg_printf(c,
					      "POST %s HTTP/1.0\r\n"
					      "Content-Type: application/json\r\n"
					      "User-Agent: mongoose/7.4\r\n"
					      "Accept: */*\r\n"
					      "Cache-Control: no-cache\r\n"
					      "Connection: keep-alive\r\n"
					      "Host: %.*s\r\n"
					      "Content-Length: %d\r\n"
					      "\r\n"
					      "%.*s\r\n"
					      "\r\n",
					      mg_url_uri(psendmsg->url),(int) host.len, host.ptr, psendmsg->post_size,psendmsg->post_size,psendmsg->post_data);
			   }


		    }
	   }
	  } else if (ev == MG_EV_HTTP_CHUNK) {
	    //LOGW("%s %d MG_EV_HTTP_CHUNK \n", __func__, __LINE__);
	    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
	    LOGW("%.*s\n", (int) hm->chunk.len, hm->chunk.ptr);
	    mg_http_delete_chunk(c, hm);
	    if (hm->chunk.len == 0) {
		psendmsg->done = true;  // Last chunk
	    }
	  } else if (ev == MG_EV_HTTP_MSG) {
	    //LOGW("%s %d MG_EV_HTTP_MSG \n", __func__, __LINE__);
	    // Response is received. Print it
	    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
	    LOGW("%.*s\n", (int) hm->method.len, hm->method.ptr);
	    LOGW("%.*s\n", (int) hm->uri.len, hm->uri.ptr);
	    LOGW("%.*s\n", (int) hm->query.len, hm->query.ptr);
	    LOGW("%.*s\n", (int) hm->proto.len, hm->proto.ptr);
	    LOGW("%.*s\n", (int) hm->body.len, hm->body.ptr);
	    if(psendmsg!= NULL && psendmsg->callback!= NULL){
		psendmsg->callback(psendmsg,hm->body.ptr,hm->body.len);
	    }
	    c->is_closing = 1;         // Tell mongoose to close this connection
	    psendmsg->done = true;  // Tell event loop to stop
	    g_wifi_config_finished = true;
	  } else if (ev == MG_EV_ERROR) {
	    LOGW("%s %d MG_EV_ERROR %s\n", __func__, __LINE__,(char*)ev_data);
	    psendmsg->done = true;  // Error, tell event loop to stop
	  } else if (ev == MG_EV_CLOSE) {
	     //LOGW("%s %d MG_EV_CLOSE \n", __func__, __LINE__);
	    psendmsg->done = true;
	  } else if (ev == MG_EV_READ) {
	    //LOGW("%s %d MG_EV_READ \n", __func__, __LINE__);
	  } else if (ev == MG_EV_WRITE) {
	    //LOGW("%s %d MG_EV_WRITE \n", __func__, __LINE__);
	  } else if (ev == MG_EV_POLL) {
		uint32_t now = get_cur_timestamp();
		if(now-psendmsg->start>psendmsg->delay){
			LOGW("%s %d timeout \n", __func__, __LINE__);
			psendmsg->done = true;
		}

	  } else if (ev == MG_EV_RESOLVE) {
	  }else{
	       LOGW("%s %d  ev = %d\n", __func__, __LINE__,ev);
	  }
  }
}

static void send_http_request(void *param)
{
	//LOGW("%s %d \n", __func__, __LINE__);
	mg_send_msg *postmsg = (mg_send_msg *)param;
	struct mg_mgr mgr;
	struct mg_connection* nc;
	mg_mgr_init(&mgr);
	//LOGW("%s %d   %s\n", __func__, __LINE__,postmsg->url);
	nc = mg_http_connect(&mgr, postmsg->url,send_ev_handler, postmsg);
	while (!postmsg->done) {
		//LOGW("%s %d mg_mgr_poll\n", __func__, __LINE__);
		mg_mgr_poll(&mgr, 1000);
		
	}
	LOGW("%s %d end\n", __func__, __LINE__);
	webrtc_mg_send_msg_free(postmsg);
	mg_mgr_free(&mgr);
	rtos_delete_thread(NULL);
}

void webrtc_http_post(char *url,char *post_data){
   bk_err_t ret = BK_OK;
   if(url!= NULL && post_data!= NULL){
    	mg_send_msg *psendmsg  = (mg_send_msg *)rtc_bk_malloc(sizeof(mg_send_msg));
    	   if(psendmsg!= NULL){
		    int post_size = strlen(post_data);
		    int url_size = strlen(url);
		    psendmsg->url = (char*)rtc_bk_malloc(url_size+8);
		    psendmsg->post_data = (char*)rtc_bk_malloc(post_size+8);
		    if(psendmsg->url!= NULL && psendmsg->post_data!= NULL){
			    snprintf(psendmsg->url,url_size+1,"%s",url);
			    snprintf(psendmsg->post_data,post_size+1,"%s",post_data);
			    psendmsg->post_size = post_size;
			    psendmsg->get = false;
			    psendmsg->done = false;
			    psendmsg->start  = get_cur_timestamp();
			    psendmsg->delay = 20000;
			    beken_thread_t  http_post_thread_hal;
			    ret = rtos_create_psram_thread(&http_post_thread_hal,
									 5,
									 "httppost",
									 (beken_thread_function_t)send_http_request,
									 16*1024,
									 psendmsg);
			   if(ret != BK_OK){
				LOGE("err: http post thread create failed\n");
			   }
		    }else{
			webrtc_mg_send_msg_free(psendmsg);
		   }
	    }
    }

}
void webrtc_http_get_net_config(char *url){
   bk_err_t ret = BK_OK;
   if(url!= NULL ){
    	mg_send_msg *psendmsg  = (mg_send_msg *)rtc_bk_malloc(sizeof(mg_send_msg));
    	   if(psendmsg!= NULL){
		    psendmsg->post_size = 0;
		    psendmsg->post_data = NULL;
		    int url_size = strlen(url);
		    psendmsg->url = (char*)rtc_bk_malloc(url_size+8);
		    if(psendmsg->url!= NULL){
			    snprintf(psendmsg->url,url_size+1,"%s",url);
			    psendmsg->message_id = WWW_GET_NET_CONFIG;
			    psendmsg->callback = webrtc_www_callback;			    
			    psendmsg->get = true;
			    psendmsg->done = false;
			    psendmsg->start  = get_cur_timestamp();
			    psendmsg->delay = 20000;
			    beken_thread_t  http_post_thread_hal;
			    ret = rtos_create_psram_thread(&http_post_thread_hal,
									 5,
									 "httppost",
									 (beken_thread_function_t)send_http_request,
									 16*1024,
									 psendmsg);
			   if(ret != BK_OK){
				LOGE("err: http post thread create failed\n");
			   }
		    }else{
			webrtc_mg_send_msg_free(psendmsg);
		   }
	    }
    }

}
void webrtc_http_get_net_serno(char *url){
   bk_err_t ret = BK_OK;
   if(url!= NULL ){
    	mg_send_msg *psendmsg  = (mg_send_msg *)rtc_bk_malloc(sizeof(mg_send_msg));
    	   if(psendmsg!= NULL){
		    psendmsg->post_size = 0;
		    psendmsg->post_data = NULL;
		    int url_size = strlen(url);
		    psendmsg->url = (char*)rtc_bk_malloc(url_size+8);
		    if(psendmsg->url!= NULL){
			    snprintf(psendmsg->url,url_size+1,"%s",url);
			    psendmsg->message_id = WWW_GET_SERNO;
			    psendmsg->callback = webrtc_www_callback;			    
			    psendmsg->get = true;
			    psendmsg->done = false;
			    psendmsg->start  = get_cur_timestamp();
			    psendmsg->delay = 20000;
			    beken_thread_t  http_post_thread_hal;
			    ret = rtos_create_psram_thread(&http_post_thread_hal,
									 5,
									 "httppost",
									 (beken_thread_function_t)send_http_request,
									 16*1024,
									 psendmsg);
			   if(ret != BK_OK){
				LOGE("err: http post thread create failed\n");
			   }
		    }else{
			webrtc_mg_send_msg_free(psendmsg);
		   }
	    }
    }
}
void light_sensor_main(void *param)
{
    uint32_t data;
    int check_adc_count = 0;
     
    light_sensor_runing = true;
    while(light_sensor_runing){
		rtos_get_semaphore(&light_sensor_sem, 1000);
		if(light_sensor_runing){
			data = adc_get_value(15);
			LOGW("value: %d\n",data);
			if(data>IR_GAP_VALUE){
				if(g_is_night_mode == false){
					check_adc_count++;
					if(check_adc_count>20){
						g_is_night_mode = true;
						LOGW("led on\n");
					}
				}

			}else{
			   if(g_is_night_mode == true){
				g_is_night_mode = false;
				LOGW("led off\n");

			   }
			   check_adc_count = 0;

			}
		}
		
    }
    if(light_sensor_sem!= NULL){
		rtos_deinit_semaphore(&light_sensor_sem);
		light_sensor_sem = NULL;
    }
    light_sensor_runing = false;
    light_sensor_thread_hal = NULL;
    rtos_delete_thread(NULL);
}

void light_sensor_start(void){
    bk_err_t ret = BK_OK;
    if(light_sensor_runing){
        LOGW("---------%s: nightadc is already start\n",__func__);
        return;
    }
    ret = rtos_init_semaphore_ex(&light_sensor_sem, 1, 0);
    if(ret!= BK_OK){
		return;
    }   
    ret = rtos_create_psram_thread(&light_sensor_thread_hal,
						 5,
						 "light",
						 (beken_thread_function_t)light_sensor_main,
						 4*1024,
						 NULL);
    if(ret != BK_OK){
	    if(light_sensor_sem!= NULL){
			rtos_deinit_semaphore(&light_sensor_sem);
			light_sensor_sem = NULL;
	    }
        LOGE("err: light_sensor thread create failed\n");
    }

}
void light_sensor_stop(void){
	 light_sensor_runing = false;
	 if(light_sensor_sem!= NULL){
			   int count = rtos_get_semaphore_count(&light_sensor_sem);
			   if(count == 0){
		           	rtos_set_semaphore(&light_sensor_sem);
			   }

	 }
}
void webrtc_network_configure_update(){

   char szurl[256]={0};
   if(strlen(config_wifi_name)>0){
          snprintf(szurl,256,"https://www.newaylink.com:8010/v1/device/bind?key=%s&name=%s",config_wifi_key,config_wifi_name);
   }else{
	 snprintf(szurl,256,"https://www.newaylink.com:8010/v1/device/bind?key=%s",config_wifi_key);
   }
   webrtc_http_get_net_config(szurl);


}
void webrtc_getdevice(){
	char szurl[256]={0};
	snprintf(szurl,256,"%s/api/getdevice","http://192.168.101.107:7088");
	webrtc_http_get_net_serno(szurl);
        //webrtc_write_system_info("PEU1iWtYJ5uy3NVaEt1EYs997rjvWgvX5lGZVGyCyIBX25+iXq1ihTsub1k6EI7NJku/mjFq8xcWAalAeViohvz32Mr4P+Y/qR8IlKUz9mlDAsC0PAfHZ3wb5NTENUkSnvDeV+lXjAfIgNrZZJL/YsRPJjQjGlW12kTzIFDgR8Q=","www.qq-kan.com","RHZL-00-6FZ7-FFYS-00002528");
}
void webrtc_network_configure_callback(char *ssid,char *password,char *key,char *name){

	if(key!= NULL){
		snprintf(config_wifi_key,64,"%s",key);
	}else{
		snprintf(config_wifi_key,64,"%s","");
	}
	if(name!= NULL){
		snprintf(config_wifi_name,64,"%s",name);
	}else{
		snprintf(config_wifi_name,64,"%s","");
	}
        configure_wifi_update = true;
	runNetworkCfg.wireless.isConfig = 0;
	snprintf((char*)runNetworkCfg.wireless.essid,32,"%s",ssid);
	snprintf((char*)runNetworkCfg.wireless.passd,32,"%s",password);
	LOGW("%s %d \n", __func__, __LINE__);
	webrtc_mailbox_send_wifi_config_msg();

}
void webrtc_network_configure_ble_disconnect_callback(int state){
	network_configure_restart_state = true;
	
}
void webrtc_erase_system_info(){
	bk_err_t ret = BK_OK;
	ret = bk_flash_partition_erase(BK_PARTITION_OTA,0,sizeof(system_info_data_));
	if(ret!= BK_OK){
		LOGE("%s %d \n", __func__, __LINE__);
	}else{
		LOGW("%s %d erase OK\n", __func__, __LINE__);
		
	}
}
void webrtc_write_system_info(char *initstring,char* serveraddr,char*serialnumber){
	bk_err_t ret = BK_OK;
	webrtc_erase_system_info();
	system_info_data_ sys_info_data;
	os_memset(&sys_info_data,0,sizeof(system_info_data_));
        LOGW("%s %d %s  %s \n", __func__, __LINE__,serveraddr,serialnumber);
	snprintf(sys_info_data.initString,256,"%s",initstring);
	snprintf(sys_info_data.serverAddr,128,"%s",serveraddr);
	snprintf(sys_info_data.serialNumber,64,"%s",serialnumber);
	sys_info_data.head = SYSTEM_INFO_HEAD;
	ret = bk_flash_partition_write(BK_PARTITION_OTA,(const uint8_t *)&sys_info_data,0,sizeof(system_info_data_));
	if(ret== BK_OK){
		os_memset(&sys_info_data,0,sizeof(system_info_data_));
		ret = bk_flash_partition_read(BK_PARTITION_OTA,(uint8_t *)&sys_info_data,0,sizeof(system_info_data_));
		if(ret== BK_OK){
			LOGW("%s %d head = 0x%x  0x%x   %s\n", __func__, __LINE__,sys_info_data.head,SYSTEM_INFO_HEAD,sys_info_data.serialNumber);
			if(sys_info_data.head == SYSTEM_INFO_HEAD){
				webrtc_system_info_inited = true;
				if(strcmp(sys_info_data.serialNumber,runSystemCfg.deviceInfo.serialNumber)==0){
				}else{
				      snprintf(runSystemCfg.deviceInfo.initString,256,"%s",sys_info_data.initString);
				      snprintf(runSystemCfg.deviceInfo.serverAddress,128,"%s",sys_info_data.serverAddr);
				      snprintf(runSystemCfg.deviceInfo.serialNumber,64,"%s",sys_info_data.serialNumber);
				      SystemCfgSave();
				}
			}
		}else{
			LOGE("%s %d failed \n", __func__, __LINE__);
		}

	}else{
	     LOGE("%s %d failed \n", __func__, __LINE__);
	}

}

void webrtc_read_system_info(){
#if 0
	bk_err_t ret = BK_OK;
	system_info_data_ sys_info_data;
	os_memset(&sys_info_data, 0, sizeof(system_info_data_));
	bk_logic_partition_t *pt = bk_flash_partition_get_info(BK_PARTITION_OTA);
	if (pt != NULL)
	{
		ret = bk_flash_partition_read(BK_PARTITION_OTA, (uint8_t *)&sys_info_data, 0, sizeof(system_info_data_));
		if (ret == BK_OK)
		{
			LOGW("%s %d head = 0x%x  0x%x   %s\n", __func__, __LINE__, sys_info_data.head, SYSTEM_INFO_HEAD, sys_info_data.serialNumber);
#if 1
			if (sys_info_data.head == SYSTEM_INFO_HEAD)
			{
				webrtc_system_info_inited = true;
				LOGW("%s %d  init  %s\n", __func__, __LINE__, sys_info_data.serialNumber);
				if (strcmp(sys_info_data.serialNumber, runSystemCfg.deviceInfo.serialNumber) == 0)
				{
				}
				else
				{
					snprintf(runSystemCfg.deviceInfo.initString, 256, "%s", sys_info_data.initString);
					snprintf(runSystemCfg.deviceInfo.serverAddress, 128, "%s", sys_info_data.serverAddr);
					snprintf(runSystemCfg.deviceInfo.serialNumber, 64, "%s", sys_info_data.serialNumber);
					SystemCfgSave();
				}
			}
			else
			{
				webrtc_erase_system_info();
				os_memset(&sys_info_data, 0, sizeof(system_info_data_));

				LOGE("%s %d no init\n", __func__, __LINE__);
			}
#endif
		}
		else
		{
			LOGE("%s %d \n", __func__, __LINE__);
		}
	}
#endif
	webrtc_system_info_inited = true;
	snprintf(runSystemCfg.deviceInfo.initString, 256, "%s", "PEU1iWtYJ5uy3NVaEt1EYs997rjvWgvX5lGZVGyCyIBX25+iXq1ihTsub1k6EI7NJku/mjFq8xcWAalAeViohvz32Mr4P+Y/qR8IlKUz9mlDAsC0PAfHZ3wb5NTENUkSnvDeV+lXjAfIgNrZZJL/YsRPJjQjGlW12kTzIFDgR8Q=");
	snprintf(runSystemCfg.deviceInfo.serverAddress, 128, "%s", "webrtc.qq-kan.com");
	snprintf(runSystemCfg.deviceInfo.serialNumber, 64, "%s", "RHZL-00-AD5A-PWV9-00002749");
}
#endif
void runhua_main(void)
{
	
	bk_err_t ret = BK_OK;
	uint32_t timestamp = get_cur_timestamp();
	uint32_t now = timestamp;
	LOGW("%s %d timestamp = %d\n", __func__, __LINE__,now);
	runhua_runing = true;

        ret = rtos_init_semaphore_ex(&event_sem, 1, 0);
	if(ret!= BK_OK){
		goto err_exit1;
	}
#if 1
        webrtc_dealwith_init();
	webrtc_mailbox_init();
#if CONFIG_SYS_CPU1
	motion_detection_starttime = now;
	bool send_cpu1_started = false;
	rtos_init_mutex(&datachannel_mutex);
	rtos_init_mutex(&audio_mutex);
#endif
#if CONFIG_SYS_CPU0
	int check_adc_count = 0;
	uint32_t check_adc_timestamp = now;
	uint32_t sd_card_detect_value = 0;
	uint32_t default_key_down_value = 0;
	init_gpio();
	sd_card_detect_value = sd_card_detect;
	default_key_down_value = default_key_down;

	webrtc_mount_flash();
	webrtc_read_system_info();
	stepmotor_init(&g_stepmsg);
	//light_sensor_start();	
	rwnxl_set_video_transfer_flag(true);
        pm_mailbox_communication_state_e cp1_state;
	if(runNetworkCfg.wireless.isConfig ==1 || strlen((char*)runNetworkCfg.wireless.essid) == 0 || strlen((char*)runNetworkCfg.wireless.passd) == 0){
		system_run_state = SYSTEM_WIFI_CONFIG;	   	
	}
	if(system_run_state != SYSTEM_WIFI_CONFIG){
#ifdef CONFIG_BT_REUSE_MEDIA_MEMORY
#if CONFIG_BLUETOOTH
		bk_bluetooth_deinit();
#endif
#endif
	}
	

#endif

#if CONFIG_SYS_CPU1
	int motion_detection = 0;
	media_debug = (media_debug_t *)os_malloc(sizeof(media_debug_t));		
	CfgInit();
	webrtc_stream_start_audio_start();
	webrtc_motion_init();		
#endif

	LOGW(" runhua app runing \r\n");
#if CONFIG_SYS_CPU2
	int motion_detection = 0;
	webrtc_motion_init();
        webrtc_cpu2_start();
	LOGW(" runhua webrtc_cpu2_start \r\n");
#endif	
	while (runhua_runing) {
		rtos_get_semaphore(&event_sem, 10);
#if CONFIG_SYS_CPU2
		//LOGW(" runhua ------== \r\n");
	         now = get_cur_timestamp();
		if(now-timestamp>=1000){
			timestamp = now;	
			//LOGW(" runhua ------ %d   %d\r\n",test,motion_detection);	
		 }
		motion_detection = webrtc_motion_detection();

		if(motion_detection>0){
			void * rectangles = (void*)webrtc_motion_rectangles();
			webrtc_cpu2_motion_detection(motion_detection,rectangles);
		}
#endif
#if CONFIG_SYS_CPU1
		      now = get_cur_timestamp();
		      if(now-timestamp>=1000){
			timestamp = now;		
		       }
#ifndef  USE_CPU2

#else
			if(motion_detection_enable){
				if(motion_detectioning == true ){
					if(now-motion_detection_starttime>motion_detection_delay){
						LOGW("%s %d stop motion  \n", __func__, __LINE__);
						
						if(motion_detection_recording == true){
							//LOGW("%s %d stop motion  --%d\n", __func__, __LINE__,webrtc_recording);
							if(webrtc_recording == true){
								
								webrtc_recording = false;
								motion_detectioning = false;
								if(webrtc_record_sem!= NULL){
										int count = rtos_get_semaphore_count(&webrtc_record_sem);
										if(count == 0){				
											    		rtos_set_semaphore(&webrtc_record_sem);
										}
								}
								
							}else{
								
							}				
						}else{
							motion_detectioning = false;
						}
					}
				}
			}
#endif
		       if(send_cpu1_started == false){
				send_cpu1_started = true;
				webrtc_mailbox_send_cpu1_start_msg();

			}
		       if(webrtc_cpu1_save_network == true){
				 webrtc_cpu1_save_network = false;
				 NetworkCfgSave();
				 webrtc_mailbox_send_wifi_config_ok_msg();
			}	   	
			if(webrtc_audio_playing_init == false && audio_opened ==true && runSystemCfg.deviceInfo.playSound == 1){
				webrtc_audio_playing_init = true;
				if(runNetworkCfg.wireless.isConfig == 0){
					play_file_index = 1;
				}else{
					play_file_index = 0;
				}
				
				webrtc_audio_play();

			}
			if(webrtc_streamer_runing == false && webrtc_streamer_start_cmd == true){
			    webrtc_streamer_start();
			    time_rtc_ntp_sync_init(timezone_offset,timezone_ntp);
			}
			// LOGW(" runhua app task runing \r\n");
			rtos_lock_mutex(&datachannel_mutex);
			if(dc_msg_list!= NULL && rtc_list_size(dc_msg_list)>0){
					datachannel_msg *dc_msg = (datachannel_msg *)rtc_list_nth_data(dc_msg_list,0);
					if(dc_msg!= NULL){
						webrtc_dealwith_datachannel_message(dc_msg);
						dc_msg_list = rtc_list_remove(dc_msg_list,dc_msg);
						destory_datachannel_message(dc_msg);
					}
			}
			rtos_unlock_mutex(&datachannel_mutex);
			rtos_lock_mutex(&audio_mutex);
			if(audio_data_list!= NULL && rtc_list_size(audio_data_list)>0){

				        audio_data *data = (audio_data *)rtc_list_nth_data(audio_data_list,0);
					if(data!= NULL){
						webrtc_dealwith_audio_data(data);
						audio_data_list = rtc_list_remove(audio_data_list,data);
						destory_audio_data(data);
					}

			}
			rtos_unlock_mutex(&audio_mutex);

			if(webrtc_record_queue_mutex!= NULL && webrtc_recording == false){
				
				
					rtos_lock_mutex(&webrtc_record_queue_mutex);
					if(webrtc_record_queue!= NULL && !rtos_is_queue_empty(&webrtc_record_queue)){
						rtos_unlock_mutex(&webrtc_record_queue_mutex);
						if(webrtc_record_sem!= NULL){
							int count = rtos_get_semaphore_count(&webrtc_record_sem);
							if(count == 0){				
						    		rtos_set_semaphore(&webrtc_record_sem);
							}
						}

					}else{
						rtos_unlock_mutex(&webrtc_record_queue_mutex);
					}
					
				
				
			}

			if(webrtc_streamer_online == true && webrtc_streamer_media_start == false){
				webrtc_streamer_media_start = true;
				webrtc_start_media(); 

			}
			if(sdcard_can_mount == true){
				sdcard_can_mount = false;
				webrtc_mailbox_cpu1_sdcard_mount();
			}
#endif
#if CONFIG_SYS_CPU0
	       now = get_cur_timestamp();
	       if(now-check_adc_timestamp>=1000){
#if 0
		   if(cpu1_runing == true){
			uint32_t adcdata = adc_get_value(15);
			if(adcdata<IR_GAP_VALUE){
				if(g_is_night_mode == false){
					check_adc_count++;
					if(check_adc_count>10){
						g_is_night_mode = true;
						LOGW("led on\n");
					}
				}

			}else{
			   if(g_is_night_mode == true){
				g_is_night_mode = false;
				LOGW("led off\n");

			   }
			   check_adc_count = 0;

			}
		  }
#endif			
		  check_adc_timestamp = now;

	       }
	       
	       if(wakeup_cpu1 == false){
			wakeup_cpu1 = true;
			webrtc_wakeup_cpu1();
	       }
	       if(cpu1_runing == true && wakeup_cpu2 == false){
			wakeup_cpu2 = true;
#ifdef USE_CPU2
			webrtc_wakeup_cpu2();
#endif
	       }	      
#if CONFIG_WEBRTC_MDNS
		if(mdns_runing == false && wifi_connected==1){
			start_mdns();
			webrtc_webserver_start("/flash/www","/flash/ca.pem","/flash/ca.key",443,80);

		}else if(mdns_runing == true && wifi_connected==0){
			stop_mdns();
		}
#endif
		if(webrtc_system_info_inited == false && wifi_connected==1 && webrtc_system_info_geting == false){
			webrtc_system_info_geting = true;
			webrtc_getdevice();

		}
		if(configure_wifi_update == true && wifi_connected==1 && system_run_state == SYSTEM_WIFI_CONFIG){
				configure_wifi_update = false;
				webrtc_network_configure_update();
		}
	       if(network_configure_restart_state == true ){
			network_configure_restart_state = false;
			if(wifi_connected==0){
			    network_configure_restart();
			}else{

			}
	        }
	       if(g_can_mount_sdcard == true){
		       sd_card_detect_value = bk_gpio_get_value(SD_CARD_DETECT_PIN);
		       if(sd_card_detect_value !=sd_card_detect){
					LOGW("sd card online state  %d\n",sd_card_detect_value);
					sd_card_detect = sd_card_detect_value;
					if(sd_card_detect == GPIO_ON){
						webrtc_mount_sdcard();
						webrtc_mailbox_sdcard_mount(1);
					}else{
						webrtc_unmount_sdcard();
						webrtc_mailbox_sdcard_mount(0);
					}
				

			}
		}
		default_key_down_value = bk_gpio_get_value(KEY_DEFAULT);
	       if(default_key_down_value !=default_key_down){
				LOGW("default key  %d\n",default_key_down_value);
				default_key_down = default_key_down_value;
				if(default_key_down == GPIO_ON){
					default_key_down_timestamp = now;
				}else{
					uint32_t default_delay;
					if(default_key_down_timestamp!= 0){
						default_delay = now-default_key_down_timestamp;
						if(default_delay>=4000){
							LOGW("set default delay time  %d\n",default_delay);
							runNetworkCfg.wireless.isConfig = 1;
							snprintf((char*)runNetworkCfg.wireless.essid,32,"%s","");
							snprintf((char*)runNetworkCfg.wireless.passd,32,"%s","");


							 NetworkCfgSave();
							 system_reboot = true;
							 runhua_runing = false;
							 LOGW("%s %d \n", __func__, __LINE__);
							 continue;
							
							//rtos_delay_milliseconds(500);
						 }else{
						   LOGW("set default delay time  %d\n",now-default_key_down_timestamp);
						}
					}else{
						 LOGW("set default delay time  %d\n",default_key_down_timestamp);
					}

				}
				

		}
		cp1_state = bk_pm_cp1_work_state_get();
	        if(cp1_state == PM_MAILBOX_COMMUNICATION_INIT){
	        LOGW(" runhua cpu1  run state %d \r\n",(int)cp1_state);
	        }else{
			 if(wifiorble_start == false && cpu1_runing == 1){
				wifiorble_start = true;

				if(system_run_state == SYSTEM_WIFI_CONFIG){
					network_configure_start(runSystemCfg.deviceInfo.serialNumber,1,webrtc_network_configure_callback,webrtc_network_configure_ble_disconnect_callback);
				}else{
					webrtc_wifi_sta_connect((char*)runNetworkCfg.wireless.essid,(char*)runNetworkCfg.wireless.passd);
				}


			 }
		}
		if(wifi_conneced_sended == false && cpu1_runing == 1 && wifi_connected == 1 && webrtc_system_info_inited == true){
			    //LOGW("webrtc_mailbox_send_wifi_connect_msg\r\n");
			   
			   if(system_run_state == SYSTEM_WIFI_CONFIG){
			    if(g_wifi_config_finished == true && cp1_state == PM_MAILBOX_COMMUNICATION_FINISH){
				    wifi_conneced_sended = true;
#ifdef CONFIG_BT_REUSE_MEDIA_MEMORY
#if CONFIG_BLUETOOTH
				    bk_bluetooth_deinit();
#endif
#endif
				    webrtc_mailbox_send_wifi_connect_msg();
			    }
			   }else{
				if(cp1_state == PM_MAILBOX_COMMUNICATION_FINISH){
				    wifi_conneced_sended = true;
				    webrtc_mailbox_send_wifi_connect_msg();
			        }
			   }
		}

                
#endif 

		
	}
	LOGW("%s %d \n", __func__, __LINE__);
#if CONFIG_SYS_CPU0
	webrtc_unmount_sdcard();
	webrtc_mailbox_send_stop_cpu1_msg();
	LOGW("%s %d \n", __func__, __LINE__);
	rtos_delay_milliseconds(500);
	webrtc_shutdown_cpu1();
	LOGW("%s %d \n", __func__, __LINE__);
	while(1){
		cp1_state = bk_pm_cp1_work_state_get();
		if(cp1_state == PM_MAILBOX_COMMUNICATION_INIT){
			 LOGW("%s %d \n", __func__, __LINE__);
			 rtos_delay_milliseconds(200);

		}else{
		   LOGW("%s %d \n", __func__, __LINE__);
		   break;

		}
	}
	LOGW("%s %d \n", __func__, __LINE__);
	stop_mdns();
	LOGW("%s %d \n", __func__, __LINE__);
#endif
#if CONFIG_SYS_CPU1
	webrtc_streamer_stop();
	webrtc_record_runing = false;
	if(webrtc_record_sem!= NULL){			
		rtos_set_semaphore(&webrtc_record_sem);
		if(webrtc_record_exit_sem){
		  rtos_get_semaphore(&webrtc_record_exit_sem, BEKEN_WAIT_FOREVER);
		  rtos_deinit_semaphore(&webrtc_record_exit_sem);
		  webrtc_record_exit_sem = NULL;
		}
					
	}
	webrtc_audio_play_runing = false;
	if(webrtc_audio_play_sem!= NULL){
		rtos_set_semaphore(&webrtc_audio_play_sem);
		if(webrtc_audio_play_exit_sem){
		  rtos_get_semaphore(&webrtc_audio_play_exit_sem, BEKEN_WAIT_FOREVER);
		  rtos_deinit_semaphore(&webrtc_audio_play_exit_sem);
		  webrtc_audio_play_exit_sem = NULL;
		}

	}
	LOGW("%s %d \n", __func__, __LINE__);
	

	LOGW("%s %d \n", __func__, __LINE__);
#endif
	rwnxl_set_video_transfer_flag(false);
	webrtc_mailbox_deinit();
	webrtc_dealwith_uninit();
#if CONFIG_SYS_CPU2
	webrtc_motion_uninit();
#endif	
#if CONFIG_SYS_CPU1
	time_rtc_ntp_sync_stop();
	webrtc_motion_uninit();
	del_all_audio_data();
	del_all_datachannel_message();
	rtos_deinit_mutex(&datachannel_mutex);
	rtos_deinit_mutex(&audio_mutex);
	
#endif
#if CONFIG_SYS_CPU0
	stepmotor_uninit(&g_stepmsg);
	light_sensor_stop();
        webrtc_unmount_flash();

	
#endif
	if(event_sem!= NULL){
		rtos_deinit_semaphore(&event_sem);
		event_sem = NULL;
	}
#if CONFIG_SYS_CPU1
	frame_buffer_deinit();
	if(media_debug!=NULL){
		os_free(media_debug);
		media_debug = NULL;
		
	}
	LOGE(" runhua app task exit \r\n");
#endif
#if CONFIG_SYS_CPU0
	if(system_reboot){
	   bk_reboot();
	}
#endif
#endif
err_exit1:

	runhua_runing = false;
	runhua_thread_hdl = NULL;
	rtos_delete_thread(NULL);

}

void webrtc_run(){
	LOGW("%s %d \n", __func__, __LINE__);
	bk_err_t ret = BK_OK;
#if CONFIG_SYS_CPU1
	bk_peripheral_init();
	frame_buffer_init();


	ret = bk_video_osi_funcs_init();

	if (ret != kNoErr)
	{
		LOGE("%s, bk_video_osi_funcs_init failed\n", __func__);
		
	}
	ret = bk_audio_osi_funcs_init();

	if (ret != kNoErr)
	{
		LOGE("%s, bk_audio_osi_funcs_init failed\n", __func__);
		
	}
        ret = rtos_init_semaphore_ex(&webrtc_record_sem, 1, 0);
	if(ret!= BK_OK){
		LOGE("create rtos_init_semaphore_ex fail\n");
	}
        ret = rtos_init_semaphore_ex(&webrtc_record_exit_sem, 1, 0);
	if(ret!= BK_OK){
		LOGE("create rtos_init_semaphore_ex fail\n");
	}

	ret = rtos_init_queue(&webrtc_record_queue,"record_queue",sizeof(queue_msg_t),16);
	if (ret != BK_OK){
		LOGE("create webrtc_record_queue fail\n");
	}
	ret = rtos_init_mutex(&webrtc_record_queue_mutex);
	if (ret != BK_OK){
		LOGE("create webrtc_record_queue_mutex fail\n");
	}

        ret = rtos_init_semaphore_ex(&webrtc_audio_play_sem, 1, 0);
	if(ret!= BK_OK){
		LOGE("create rtos_init_semaphore_ex fail\n");	
	}
        ret = rtos_init_semaphore_ex(&webrtc_audio_play_exit_sem, 1, 0);
	if(ret!= BK_OK){
		LOGE("create rtos_init_semaphore_ex fail\n");	
	}

        ret = rtos_init_semaphore_ex(&webrtc_audio_playing_sem, 1, 0);
	if(ret!= BK_OK){
		LOGE("create rtos_init_semaphore_ex fail\n");
	}



        
#endif
	
	ret = rtos_create_psram_thread(&runhua_thread_hdl,
						 5,
						 "runhua",
						 (beken_thread_function_t)runhua_main,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create runhua app task fail \r\n");
		runhua_thread_hdl = NULL;
	}
#if 1
#if CONFIG_SYS_CPU1
	
	ret = rtos_create_psram_thread(&webrtc_record_thread_hdl,
						 5,
						 "record",
						 (beken_thread_function_t)webrtc_record_thread,
						 16*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create record  task fail \r\n");
		webrtc_record_thread_hdl = NULL;
	}

	ret = rtos_create_psram_thread(&webrtc_audio_play_thread_hdl,
						 5,
						 "audioplay",
						 (beken_thread_function_t)webrtc_audio_play_thread,
						 8*1024,
						 NULL);
	if (ret != kNoErr) {
		LOGE("create record  task fail \r\n");
		webrtc_audio_play_thread_hdl = NULL;
	}


#endif
#endif
	LOGW("create runhua app task complete \r\n");

}

