

#ifndef WEBRTC_STREAMER_H_3060898B846849FF9F88F5DB59B5950C
#define WEBRTC_STREAMER_H_3060898B846849FF9F88F5DB59B5950C

#ifdef __cplusplus
#include <cstddef>
#include <cstdarg>
#include <sys/types.h>
extern "C" {
#else
#include <stdarg.h>
#include <sys/types.h>
#endif


typedef enum {
    WEBRTC_EVENT_NULL             = 0,
    WEBRTC_EVENT_ASK_IFRAME       = 1,
    WEBRTC_EVENT_LUCK             = 2,   //回调值result 返回锁状态   0 开 ，1是关
    WEBRTC_EVENT_UNLUCK           = 3,   //回调值result 返回锁状态   0 开 ，1是关
    WEBRTC_EVENT_LUCK_STATE       = 4,   //回调值result 返回锁状态   0 开 ，1是关
    WEBRTC_EVENT_CALL_START       = 5,
    WEBRTC_EVENT_CALL_LINK        = 6,
    WEBRTC_EVENT_CALL_DISCONNECT  = 7,
    WEBRTC_EVENT_CALL_DESTORY     = 8,
    WEBRTC_EVENT_ONLINE           = 9,
    WEBRTC_EVENT_OFFLINE          = 10,
    WEBRTC_EVENT_DATACHANNEL_OPEN          = 11,
    WEBRTC_EVENT_LOW_POWER_LEVEL       = 12,  
    WEBRTC_EVENT_MAX_CHANNEL       = 13,
    WEBRTC_EVENT_CONNECT_FAILD     = 14, 
    WEBRTC_EVENT_CONNECT_RECONNECT = 15, 
    WEBRTC_EVENT_CALL_FAILD = 16,
    WEBRTC_EVENT_PLI = 17,
    WEBRTC_EVENT_SLI = 18, 
} webrtc_event_type_t;

typedef enum {
    WEBRTC_CALL_EVENT_DOORBELL_PRESS            = 0,
    WEBRTC_CALL_EVENT_PIR            = 1,
    WEBRTC_CALL_EVENT_CUSTOM            = 2,
} webrtc_call_event_type_t;
typedef enum {
    WEBRTC_STREAM_MAIN =0,
    WEBRTC_STREAM_SUB =1,
    WEBRTC_STREAM_2 =2,
    WEBRTC_STREAM_3 =3,
    WEBRTC_STREAM_4 =4,
    WEBRTC_STREAM_5 =5,
    WEBRTC_STREAM_6 =6,
    WEBRTC_STREAM_7 =7,
    WEBRTC_STREAM_8 =8,
    WEBRTC_STREAM_9 =9,
    WEBRTC_STREAM_10 =10,
    WEBRTC_STREAM_11 =11,
    WEBRTC_STREAM_12 =12,
    WEBRTC_STREAM_13 =13,
    WEBRTC_STREAM_14 =14,
    WEBRTC_STREAM_15 =15,
    WEBRTC_STREAM_PLAY =256,
} webrtc_stream_type_t;

typedef enum {
    WEBRTC_NETWORK_QUALITY_GOOD =0,
    WEBRTC_NETWORK_QUALITY_MIDDLE =1,
    WEBRTC_NETWORK_QUALITY_LOW =2,
} webrtc_network_quality_type_t;


typedef enum {
    WEBRTC_ERR_SUCCESS              = 0,
    WEBRTC_ERR_INITSTRING            = -1,
    WEBRTC_ERR_SERNO               = -2,
} webrtc_error_t;

typedef enum  {
  WEBRTC_DMT_BINARY = 0,
  WEBRTC_DMT_TEXT = 1,
}webrtc_data_message_type_t;
typedef enum {
	WEBRTC_STREAM_DEBUG=1,
	WEBRTC_STREAM_MESSAGE=1<<1,
	WEBRTC_STREAM_WARNING=1<<2,
	WEBRTC_STREAM_ERROR=1<<3,
	WEBRTC_STREAM_FATAL=1<<4,
	WEBRTC_STREAM_TRACE=1<<5,
	WEBRTC_STREAM_LOGLEV_END=1<<6
} webrtc_LogLevel_t;
typedef enum {
        WEBRTC_CLOUD_UPLOADSPEED_NORMAL              = 0,
	WEBRTC_CLOUD_UPLOADSPEED_FAST              = 1,
} webrtc_clouduploadspeed_t;
typedef struct _webrtc_streamer_session_info{
	webrtc_stream_type_t start_stream_type;
        webrtc_stream_type_t stream_type;
        char szmode[64];
	int connect_time;        
	int session_time;  
        int video_send_bitrate;  //视频的实际发送码率  KB/S  (The actual transmission rate of the video)
        int audio_send_bitrate;  //音频的实际发送码率  KB/S  (The actual transmission rate of the audio)
	int send_packets;      //发送包数  (Number of sent packets)
        int resend_packets;      //重传包数  (Number of retransmitted packets)
        int current_resend_packets;  //当前重传包数 (Current number of retransmitted packets)
        int video_recv_bitrate;  //视频的实际接收码率  KB/S (The actual received bit rate of the video)
        int audio_recv_bitrate;  //音频的实际接收码率  KB/S (The actual received bit rate of the audio)
        int transmission_mode;   //网络传输方式    0 未连接 ,1 p2p ,2 relay (Network transmission method)
        int audio_packet_loss;   //音频总丢包数 (Total audio packet loss)
        int video_packet_loss;   //视频总丢包数 (Total video packet loss)
	int audio_current_packet_lost;  //音频当前丢包数 (Current number of audio packet losses)
	int video_current_packet_lost;  //视频当前丢包数 (Current number of video packet losses)
        int current_bandwidth_kps;  //当前带宽 (Current bandwidth kps)
        int current_pli_count;

} webrtc_streamer_session_info;

typedef enum  {
	WEBRTC_VIDEO_H264 = 0,
        WEBRTC_VIDEO_H265 = 1,
        WEBRTC_VIDEO_VP8 = 2,
        WEBRTC_VIDEO_VP9 = 3,
	WEBRTC_VIDEO_AV1 = 4,
}webrtc_video_code_type_t;


typedef enum {
    WEBRTC_CLOULD_PUB_SUCCESS              = 0,
    WEBRTC_CLOULD_PUB_FILE_ERR_NETWORK     = -1,
    WEBRTC_CLOULD_PUB_FILE_ERR_OPEN_FILE     = -2,
    WEBRTC_CLOULD_PUB_NO_SERVER_ADDR    = -3,
    WEBRTC_CLOULD_PUB_NO_SERVER_SPACE    = -4,
    WEBRTC_CLOULD_PUB_NO_PERSONAL_SPACE    = -5,
} webrtc_cloud_publish_error_t;


typedef void(*webrtc_streamer_call_income_callback)(char *sessionId,size_t sessionId_len,char *szmode,size_t mode_len,char *szsource,size_t source_len,void *user);
typedef void(*webrtc_streamer_call_destory_callback)(char *sessionId,size_t sessionId_len,void *user);
typedef void(*webrtc_streamer_call_failed_callback)(char *sessionId,size_t sessionId_len,void *user);
typedef void(*webrtc_streamer_call_disconnect_callback)(char *sessionId,size_t sessionId_len,void *user);
typedef void(*webrtc_streamer_event_callback)(webrtc_event_type_t event,void *user,int *result);
typedef void(*webrtc_streamer_audio_callback)(char *data,size_t len,char *sessionId,size_t sessionId_len,void *user);
typedef void(*webrtc_streamer_configuration_callback)(char *data,size_t len,int reboot);
typedef int(*webrtc_streamer_authentication_callback)(char *authdata,size_t authlen,char *password,size_t pwdlen);
typedef void(*webrtc_streamer_message_callback)(char *sessionId,size_t sessionId_len,char *ReqMsg,size_t ReqMsg_len,char *RspMsg,size_t *RspMsg_len,void *user);
typedef void(*webrtc_streamer_datachannel_message_callback)(char *sessionId,size_t sessionId_len,webrtc_data_message_type_t type,int streamid,char *Msg,size_t Msg_len,void *user);
typedef void(*webrtc_streamer_datachnanle_open_callback)(char *sessionId,size_t sessionId_len,int streamid,void *user);
typedef void(*webrtc_streamer_can_add_datachnanle_callback)(char *sessionId,size_t sessionId_len,int is_create_offer,void *user);
typedef void(*webrtc_streamer_remote_play_start_callback)(char *sessionId,size_t sessionId_len,void *user);
typedef void(*webrtc_streamer_session_ask_iframe_callback)(char *sessionId,size_t sessionId_len,void *user);
typedef void(*webrtc_streamer_session_pli_callback)(char *sessionId,size_t sessionId_len,void *user);
typedef void(*webrtc_streamer_mixer_audio_callback)(char *data,size_t len,void *user);

/*
//could upload callback
*/
typedef void(*webrtc_streamer_cloud_publish_file_start_callback)(char *sessionId,size_t sessionId_len,char *filename,size_t filename_len,char *clouldfilename,size_t clouldfilename_len,void *user);
typedef void(*webrtc_streamer_cloud_publish_file_end_callback)(char *sessionId,size_t sessionId_len,char *filename,size_t filename_len,void *user);
typedef void(*webrtc_streamer_cloud_publish_file_error_callback)(char *sessionId,size_t sessionId_len,int error,void *user);
typedef void(*webrtc_streamer_cloud_publish_file_step_callback)(char *sessionId,size_t sessionId_len,int step,void *user);

typedef void(*webrtc_streamer_cloud_publish_realtime_stream_error_callback)(char *sessionId,size_t sessionId_len,int error,void *user);




typedef void(*webrtc_streamer_video_callback)(webrtc_stream_type_t stream_type,webrtc_video_code_type_t type,char *data,size_t len,char *sessionId,size_t sessionId_len,void *user);


/*
sdk 查询该会话的视频压缩类型  返回  webrtc_video_code_type_t
*/
typedef int(*webrtc_streamer_check_videocode_callback)(webrtc_stream_type_t stream_type,char *sessionId,size_t sessionId_len,char *sessionType,size_t sessionType_len,char *szmode,size_t mode_len,char *szsource,size_t source_len,void *user);


typedef void(*webrtc_streamer_call_income_callback_ex)(char *sessionId,size_t sessionId_len,char *sessionType,size_t sessionType_len,char *szmode,size_t mode_len,char *szsource,size_t source_len,void *user);

/*
当Alexa使用定制的语音控制开关命令的回调函数
*/
typedef void(*webrtc_streamer_alexa_customer_message_callback)(char *pnamespace,size_t namespace_len,char *pinstance,size_t instance_len,char *name,size_t name_len,char *alexadirectivemsg,size_t alexadirectivemsg_len,char *resvalue,void *user);
/*
提供一个回调函数。告诉调用方现在会话的通讯质量
*/
typedef void(*webrtc_streamer_network_quality_callback)(char *sessionId,size_t sessionId_len,webrtc_network_quality_type_t quality,void *user);
/*
该函数是为了例如 liteos 等用到lwip的。提供一个回调给客户从lwip里获取本地IP地址的接口，可以更快的获取本地IP地址
*/
typedef void(*webrtc_streamer_get_network_info_callback)(char *ip,char* gw,char*mask,void *user);

typedef int(*webrtc_streamer_authentication_ex_callback)(char *sessionId,size_t sessionId_len,char *sessionType,size_t sessionType_len,char *authdata,size_t authlen,char *password,size_t pwdlen);
int webrtc_streamer_register_authentication_ex_callback_fun(webrtc_streamer_authentication_ex_callback callback,void *user);



typedef void(*webrtc_streamer_callstate_callback)(char *sessionId,size_t sessionId_len,char *szstate,void *user);

/*
设置log 输出 level
*/

int webrtc_streamer_set_log_level_mask(webrtc_LogLevel_t lev);

/*
设置最大链接数，默认值是32
*/

int webrtc_streamer_set_max_channel(int max_channel);
/*
初始化函数
*/
int webrtc_streamer_init(char *initstring,char *configuration,char *serno,char *servers,char *customerserno);

/*
反初始化函数
*/
int webrtc_streamer_uninit(void);

/*
获取当前连接会话数
*/
int webrtc_streamer_current_session_count(void);
/*
设置使用 mdns 发现设备内容
参数 mdnsservername      设置服务名称  设置后服务名称是   _XXXXXXXX._tcp.local.
参数 name                设置 mdns  txt 信息总  name 的内容
参数 type                设置 mdns  txt 信息总  type 的内容
*/
int webrtc_streamer_set_device_discovery_info(char *mdnsservername,char *name,char*type);
/*
使能mdns 是否发送的 txt 内容  默认是 1
为了安全问题。可以在发现设备期间 设置为 1 平时设置为 0
*/
int webrtc_streamer_enable_device_discovery_info(int enable);




/*
函数说明:
      webrtc_streamer_set_mem_info::
     
      max_sdk_use_mem            max memory use for sdk                                    defualt     10*1024*1024
      max_ssession_use_mem       max memory use for every ssession                         defualt     300*1024
      max_ssession_buffer_size   max memory use for every ssession cache buffer size       defualt     60
 
第一个参数：max_sdk_use_mem 是sdk的总内存限制。超出就会丢弃。
第二个参数：max_ssession_use_mem 每个会话的内存限制，一般300-600K看用户设置的码流。
第三个参数：max_ssession_buffer_size 是视频缓冲RTP网络打包数，用于重传。算法： 缓冲包*1500，这个数。应该比第二个参数小。大概是第二个参数一半，其他用于网络收包和音频包。   
*/
int webrtc_streamer_set_mem_info(int max_sdk_use_mem,int max_ssession_use_mem,int max_ssession_buffer_size);

/*
注册回调函数
*/
int webrtc_streamer_register_call_income_callback_fun(webrtc_streamer_call_income_callback callback,void *user);
int webrtc_streamer_register_call_destory_callback_fun(webrtc_streamer_call_destory_callback callback,void *user);
int webrtc_streamer_register_call_failed_callback_fun(webrtc_streamer_call_failed_callback callback,void *user);
int webrtc_streamer_register_call_disconnect_callback_fun(webrtc_streamer_call_disconnect_callback callback,void *user);
int webrtc_streamer_register_event_callback_fun(webrtc_streamer_event_callback callback,void *user);
int webrtc_streamer_register_audio_callback_fun(webrtc_streamer_audio_callback callback,void *user);
int webrtc_streamer_register_configuration_callback_fun(webrtc_streamer_configuration_callback callback,void *user);
int webrtc_streamer_register_authentication_callback_fun(webrtc_streamer_authentication_callback callback,void *user);
int webrtc_streamer_register_message_callback_fun(webrtc_streamer_message_callback callback,int MaxRspBufSize,void *user);
int webrtc_streamer_register_datachannel_open_callback_fun(webrtc_streamer_datachnanle_open_callback callback,void *user);
int webrtc_streamer_register_can_add_datachannel_callback_fun(webrtc_streamer_can_add_datachnanle_callback callback,void *user);
int webrtc_streamer_register_datachannel_message_callback_fun(webrtc_streamer_datachannel_message_callback callback,void *user);
int webrtc_streamer_register_remote_play_start_callback_fun(webrtc_streamer_remote_play_start_callback callback,void *user);
int webrtc_streamer_register_alexa_customer_message_callback_fun(webrtc_streamer_alexa_customer_message_callback callback,void *user);
int webrtc_streamer_register_network_quality_callback_fun(webrtc_streamer_network_quality_callback callback,void *user);
int webrtc_streamer_register_get_network_info_callback_fun(webrtc_streamer_get_network_info_callback callback,void *user);
int webrtc_streamer_register_session_ask_iframe_callback_fun(webrtc_streamer_session_ask_iframe_callback callback,void *user);
int webrtc_streamer_register_session_pli_callback_fun(webrtc_streamer_session_pli_callback callback,void *user);
int webrtc_streamer_register_mixer_audio_callback_fun(webrtc_streamer_mixer_audio_callback callback,void *user);
int webrtc_streamer_register_call_income_callback_ex_fun(webrtc_streamer_call_income_callback_ex callback,void *user);
int webrtc_streamer_register_video_callback_fun(webrtc_streamer_video_callback callback,void *user);
int webrtc_streamer_register_check_videocode_callback_fun(webrtc_streamer_check_videocode_callback callback,void *user);
// 用于设置主动呼叫函数 webrtc_streamer_call 的状态回调（offline serno error）
int webrtc_streamer_register_callstate_callback_fun(webrtc_streamer_callstate_callback callback,void *user);


int webrtc_streamer_register_cloud_publish_file_start_callback_fun(webrtc_streamer_cloud_publish_file_start_callback callback,void *user);
int webrtc_streamer_register_cloud_publish_file_end_callback_fun(webrtc_streamer_cloud_publish_file_end_callback callback,void *user);
int webrtc_streamer_register_cloud_publish_file_error_callback_fun(webrtc_streamer_cloud_publish_file_error_callback callback,void *user);
int webrtc_streamer_register_cloud_publish_file_step_callback_fun(webrtc_streamer_cloud_publish_file_step_callback callback,void *user);

int webrtc_streamer_register_cloud_publish_realtime_stream_error_callback_fun(webrtc_streamer_cloud_publish_realtime_stream_error_callback callback,void *user);



/*
实时流音频输入函数  
注意:对于摄像机等设备声音源只有一个的。采用该函数。该函数会自动分发到各个实时流回话
     该函数不要跟webrtc_streamer_input_audio_data_ex 混用，如果混用会导致声音重复传输
*/
int webrtc_streamer_input_audio_data(unsigned char *data,size_t len);

/*
实时流音频输入函数
注意：对于NVR等应用声音源有多个的，可以采用该函数，改函数里面分发到各个请求该通道的实时回话
     该函数不要跟webrtc_streamer_input_audio_data 混用，如果混用会导致声音重复传输
*/
int webrtc_streamer_input_audio_data_ex(webrtc_stream_type_t type,unsigned char *data,size_t len);

/*
实时流视频输入函数
*/
int webrtc_streamer_input_video_data(webrtc_stream_type_t type,webrtc_video_code_type_t code_type,unsigned char *data,size_t len);


/*
用于动态修改实时视频的通道号(使用场景：例如清晰度切换等等)
*/
int webrtc_streamer_set_session_streamtype(char *sessionId,webrtc_stream_type_t type);


int webrtc_streamer_set_session_realstream(char *sessionId,size_t sessionId_len,int enable);



int webrtc_streamer_event_call(webrtc_call_event_type_t event);
/*
关闭回话函数
*/
int webrtc_streamer_close_session(char *sessionId,size_t sessionId_len);
/*
通过信令通道发送消息函数
会话 (sessionId )一定要存在会话列表里面（就是要发起 offer 或者call 信令的会话）,否则发送失败
*/
int webrtc_streamer_send_message(char *sessionId,char *data,size_t len);
/*
*/
int webrtc_streamer_network_reset();
/*
 播放数据输入接口，需要带 webrtc_streamer_call_income_callback 回调回来的 sessionId
*/
int webrtc_streamer_player_input_audio_data(char *sessionId,unsigned char *data,size_t len);
int webrtc_streamer_player_input_video_data(webrtc_video_code_type_t code_type,char *sessionId,unsigned char *data,size_t len);
int webrtc_streamer_player_clean_send_buffer(char *sessionId,size_t sessionId_len);

/*
建立数据通道
在 webrtc_streamer_can_add_datachnanle_callback 回调后可以调用
参数： sessionId       会话ID
       streamid        datachnanle  流 id  （0-32）
       dcname          datachnanle lable 名称
*/

int webrtc_streamer_add_datachannel(char *sessionId,int streamid,char *dcname,size_t dclen);

/*
通过datachannel 发送消息函数
*/
int webrtc_streamer_datachannel_send_message(char *sessionId,webrtc_data_message_type_t type,int streamid,char *data,size_t len);
/*
获取会话信息函数
*/
int webrtc_streamer_get_session_info(char *sessionId,webrtc_streamer_session_info *sessioninfo);



int webrtc_streamer_set_audio_mixer_session_mute(char *sessionId,int mute);
int webrtc_streamer_get_audio_mixer_session_mute(char *sessionId);
int webrtc_streamer_set_audio_mixer_mute(int mute);
int webrtc_streamer_get_audio_mixer_mute(void);
/*
推送消息 当客户端订阅的时候
*/
int webrtc_streamer_publish_message(char *data,size_t len);

typedef void(*webrtc_streamer_webserver_api_callback)(const char *url,size_t url_len,const char *ReqMsg,size_t ReqMsg_len,char *RspMsg,size_t *RspMsg_len,void *user);
typedef void(*webrtc_streamer_webserver_websocket_message_callback)(char *sessionId,size_t sessionId_len,char *ReqMsg,size_t ReqMsg_len,char *RspMsg,size_t *RspMsg_len,void *user);

int webrtc_streamer_register_webserver_api_callback_fun(webrtc_streamer_webserver_api_callback callback,int MaxRspBufSize,void *user);
int webrtc_streamer_register_webserver_websocket_messaeg_callback_fun(webrtc_streamer_webserver_websocket_message_callback callback,int MaxRspBufSize,void *user);

int webrtc_streamer_webserver_start(char *rootpath,char *certpath,char *certkeypath,int https_port,int http_port);
int webrtc_streamer_webserver_stop(void);




int webrtc_streamer_call(webrtc_stream_type_t type,char *sessionId,size_t sessionId_len,char * to,size_t to_len,char *audio,char *video,int datachennel);

/*
通过信令通道发送消息函数
会话（sessionId）不一定要在会话列表中存在，需要多个发送对方的ID参数
*/

int webrtc_streamer_send_message_ex(char *sessionId,char* sessionType,char*to,char *data,size_t len);


/*
***********************************************************************************************
****************************             云存接口                 *****************************
***********************************************************************************************
*/

/*
推送文件
参数： 
char *filename   文件全路径   
size_t filename_len 文件全路径长度
*/

int webrtc_streamer_cloud_publish_file(char *sessionId,size_t sessionId_len,char *filepath,size_t filepath_len,char *filename,size_t filename_len,char *cloud_filename,size_t cloud_filename_len,webrtc_clouduploadspeed_t speed,int event);
/*
推送实时流 
参数：webrtc_stream_type_t type 推送哪路流
*/
int webrtc_streamer_cloud_publish_realtime_stream(char *sessionId,size_t sessionId_len,webrtc_stream_type_t type);





#ifdef __cplusplus
}
#endif

#endif
