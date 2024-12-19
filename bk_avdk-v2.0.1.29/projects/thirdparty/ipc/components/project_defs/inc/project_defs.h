
#ifndef _PROJECT_DEFS_H
#define _PROJECT_DEFS_H
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>

typedef struct
{
	void *data;
	size_t len;
	size_t size;
	int flags;
        uint8_t fromaddr[96];
	int fromlen;
}recv_data_t;
typedef struct
{
	void *data;
	size_t len;
	size_t size;
	int flags;
        uint8_t toaddr[96];
	int tolen;
}send_data_t;
typedef struct
{
	int result;
	int resultlen;
	uint8_t         szresult[128]; 
}result_data_t;
typedef struct
{
	u32		param1;
	u32		param2;
	u32		param3;
}chnl_cmd_t;
typedef struct
{
	u32		param1;
	u32		param2;
	u32		param3;
	u32		param4;
	u32		param5;
	int		nparam1;
	int		nparam2;
	int		nparam3;
	long		lparam;
	int             szparamlen;
	uint8_t         szparam[128];           
}param_data_t;
typedef struct
{
	u32		msgid;
	u8		hdrcmd;
	u8		txstate;
	u8              responseed;
	u8              isWaited;
	beken_semaphore_t sem;
	beken_mutex_t 	mutex;
	chnl_cmd_t      mb_cmd;
	param_data_t    param;
	result_data_t   result;
	recv_data_t     recvdata;
	send_data_t     senddata;
} webrtc_cmd_t;

typedef struct
{
	void* param;
} queue_msg_t;

typedef struct
{
	int (*mailbox_send_media_req_msg) (u32 param1,u32 param2,u32 param3);
	int (*mailbox_send_media_response_msg) (u32 param1,u32 param2,u32 param3);
} project_config_t;

#define  MESSAGE_HEAD_ID                  0x00ff23ff

#define  MESSAGE_HEAD_COM_REQ             0x00
#define  MESSAGE_HEAD_COM_RESP            0x01
#define  MESSAGE_HEAD_COM_MASK            0x01

#define  MESSAGE_HEAD_COM_APP             0x01<<1
#define  MESSAGE_HEAD_COM_SOCKET          0x01<<2


#define  MESSAGE_COM_HEAD(req,type)  req | type
#define  MESSAGE_COM_IS_REQ(cmd)     cmd & MESSAGE_HEAD_COM_MASK


/////////////////////////////////////////////////////////////////////////
//              resp
////////////////////////////////////////////////////////////////////////
#define  APP_COM_BASE    	        0x01

#define  APP_COM_MIN    	        APP_COM_BASE
#define  APP_COM_STOP_CPU1   	        APP_COM_BASE+1
#define  APP_COM_START_RECORD     	APP_COM_BASE+2
#define  APP_COM_STOP_RECORD    	APP_COM_BASE+3
#define  APP_COM_TEST            	APP_COM_BASE+4
#define  APP_COM_PTZ_CTRL            	APP_COM_BASE+5

#define  APP_COM_CPU1_START             APP_COM_BASE+6
#define  APP_COM_CPU1_WEBRTC_START      APP_COM_BASE+7
#define  APP_COM_CPU1_WEBRTC_STOP       APP_COM_BASE+8
#define  APP_COM_WIFI_CONNECT           APP_COM_BASE+9
#define  APP_COM_WIFI_DISCONNECT        APP_COM_BASE+10
#define  APP_COM_WIFI_CONFIG            APP_COM_BASE+11
#define  APP_COM_WIFI_CONFIG_OK         APP_COM_BASE+12
#define  APP_COM_SDCARD_MOUNT      	APP_COM_BASE+13
#define  APP_COM_CPU1_SDCARD_MOUNT      APP_COM_BASE+14
#define  APP_COM_COULD_PUBLISH_START    APP_COM_BASE+15
#define  APP_COM_COULD_PUBLISH_STOP     APP_COM_BASE+16
#define  APP_COM_FORMAT_SDCORD          APP_COM_BASE+17


#define  APP_COM_MAX    	        0xff

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
#define  SOCKET_COM_BASE   	        APP_COM_MAX
#define  SOCKET_COM_MIN    	        SOCKET_COM_BASE
#define  SOCKET_COM_ERROR  		SOCKET_COM_BASE+1
#define  SOCKET_COM_CREATE  		SOCKET_COM_BASE+2
#define  SOCKET_COM_CLOSE    		SOCKET_COM_BASE+3
#define  SOCKET_COM_BIND     		SOCKET_COM_BASE+4
#define  SOCKET_COM_SEND     		SOCKET_COM_BASE+5
#define  SOCKET_COM_RECV     		SOCKET_COM_BASE+6
#define  SOCKET_COM_INETADDR    	SOCKET_COM_BASE+7
#define  SOCKET_COM_IOCTRL     		SOCKET_COM_BASE+8
#define  SOCKET_COM_FCNTL   		SOCKET_COM_BASE+9
#define  SOCKET_COM_GETPEERNAME    	SOCKET_COM_BASE+10
#define  SOCKET_COM_GETSOCKNAME    	SOCKET_COM_BASE+11
#define  SOCKET_COM_GETSOCKOPT    	SOCKET_COM_BASE+12
#define  SOCKET_COM_SETSOCKOPT     	SOCKET_COM_BASE+13
#define  SOCKET_COM_SHUTDOWN    	SOCKET_COM_BASE+14
#define  SOCKET_COM_GETHOSTBYNAME  	SOCKET_COM_BASE+15
#define  SOCKET_COM_GETADDRINFO   	SOCKET_COM_BASE+16
#define  SOCKET_COM_FREEADDRINFO  	SOCKET_COM_BASE+17
#define  SOCKET_COM_CONNECT   		SOCKET_COM_BASE+18
#define  SOCKET_COM_LISTEN    		SOCKET_COM_BASE+19
#define  SOCKET_COM_GETLOCALADDR   	SOCKET_COM_BASE+20
#define  SOCKET_COM_INET_NTOA     	SOCKET_COM_BASE+21
#define  SOCKET_COM_INET_ATON    	SOCKET_COM_BASE+22
#define  SOCKET_COM_ACCEPT    		SOCKET_COM_BASE+23
#define  SOCKET_COM_GET_NET_CONFIG   	SOCKET_COM_BASE+24

#define  SOCKET_COM_MAX    	        SOCKET_COM_BASE+0xff

#define  VFS_FILE_COM_BASE   	        SOCKET_COM_MAX
#define  VFS_FILE_COM_MIN    	        VFS_FILE_COM_BASE

#define  VFS_FILE_COM_OPEN  		VFS_FILE_COM_BASE+1
#define  VFS_FILE_COM_READ  		VFS_FILE_COM_BASE+2
#define  VFS_FILE_COM_WRITE  		VFS_FILE_COM_BASE+3
#define  VFS_FILE_COM_CLOSE  		VFS_FILE_COM_BASE+4
#define  VFS_FILE_COM_UNLINK  		VFS_FILE_COM_BASE+5
#define  VFS_FILE_COM_LSEEK  		VFS_FILE_COM_BASE+6
#define  VFS_FILE_COM_TELL  		VFS_FILE_COM_BASE+7
#define  VFS_FILE_COM_STAT  		VFS_FILE_COM_BASE+8
#define  VFS_FILE_COM_MKDIR  		VFS_FILE_COM_BASE+9
#define  VFS_FILE_COM_FSTAT  		VFS_FILE_COM_BASE+10
#define  VFS_FILE_COM_RENAME  		VFS_FILE_COM_BASE+11
#define  VFS_FILE_COM_FSYNC  		VFS_FILE_COM_BASE+12
#define  VFS_FILE_COM_FTRUNCATE  	VFS_FILE_COM_BASE+13
#define  VFS_FILE_COM_FCNTL		VFS_FILE_COM_BASE+14
#define  VFS_FILE_COM_RMDIR		VFS_FILE_COM_BASE+15
#define  VFS_FILE_COM_OPENDIR		VFS_FILE_COM_BASE+16
#define  VFS_FILE_COM_CLOSEDIR		VFS_FILE_COM_BASE+17
#define  VFS_FILE_COM_READDIR		VFS_FILE_COM_BASE+18
#define  VFS_FILE_COM_SEEKDIR		VFS_FILE_COM_BASE+19
#define  VFS_FILE_COM_TELLDIR		VFS_FILE_COM_BASE+20
#define  VFS_FILE_COM_REWINDDIR		VFS_FILE_COM_BASE+21
#define  VFS_FILE_COM_STATFS		VFS_FILE_COM_BASE+22

#define  VFS_FILE_COM_MAX    	        VFS_FILE_COM_BASE+0xff




#define  CPU12_COM_BASE    	        VFS_FILE_COM_MAX

#define  CPU12_COM_START   	        CPU12_COM_BASE+1
#define  CPU12_COM_CAPTURE     		CPU12_COM_BASE+2
#define  CPU12_COM_MOTION_DETECTION    	CPU12_COM_BASE+3
#define  CPU12_COM_MOTION_RESET    	CPU12_COM_BASE+4

#define  CPU12_COM_MAX    	        CPU12_COM_BASE+0xff



#endif /* _PROJECT_DEFS_H */
