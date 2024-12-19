#include "bk_private/bk_init.h"
#include <components/system.h>
#include <os/os.h>
#include <components/shell_task.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <os/str.h>
#include <stdlib.h>

extern void user_app_main(void);
extern void rtos_set_user_app_entry(beken_thread_function_t entry);
extern void webrtc_run();

#if CONFIG_SYS_CPU1
#include "cli.h"
#endif
// #if CONFIG_SYS_CPU0
#include "cli.h"
extern void webrtc_test(char *a,char *b);
extern void webrtc_reboot();
extern void webrtc_write_bin(); // 加的+++++
extern void webrtc_wakeup_cpu1();
extern void webrtc_shutdown_cpu1();
extern void webrtc_set_mac_address();
extern void webrtc_set_default_system_info();
extern void webrtc_mailbox_send_start_record_msg(void); // CPU0发送开始录音请求
extern void webrtc_mailbox_send_stop_record_msg(void);	// CPU0发送停止录音请求
extern void webrtc_connect_msg(char *ssid, char *key);	////配置并保存Wi-Fi连接信息
extern void webrtc_ls_files(char *path);				// 不同cpu调用不同函数列出指定路径下的文件
extern void webrtc_vfs_statfs(char *path);				// 获取指定路径的文件系统状态信息
extern void webrtc_vfs_del_file(char *path);			// 从文件系统删除一个文件
extern void webrtc_vfs_mkdir(char *path);				// 从文件系统创建一个目录
extern void webrtc_vfs_rmdir(char *path);				// 从文件系统删除一个目录
extern void webrtc_vfs_format_sdcard();					// 格式化SD卡
extern void webrtc_could_publish_stream_start();		// 启动WebRTC流发布
extern void webrtc_could_publish_stream_stop();			// 停止WebRTC流发布
extern void webrtc_capture();							// 开启YUV帧的捕获
extern void webrtc_read_device_info();
extern int webrtc_mount_sdcard(int a);
extern void webrtc_mailbox_sdcard_mount(int mounted);
extern void webrtc_burn(char *serialnumber, char *serveraddress); // 写入序列号到配置文件
#if CONFIG_SYS_CPU0
extern void webrtc_mailbox_send_irled_off_msg(void);
extern int MAX_DELAy,rtos_delay;
extern void webrtc_mailbox_send_irled_on_msg(void);
extern void low_pwr_mode(void);
extern void reset_wifi();
#endif
// 处理命令行界面发送来的指令
static void cli_webrtc_cmd(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	if (argc > 1)
	{
		if (os_strcmp(argv[1], "reset") == 0)
		{
#if CONFIG_SYS_CPU0
			// webrtc_wakeup_cpu1();
			reset_wifi();
			// printf("////////////////////");
#endif
		}

		if (os_strcmp(argv[1], "start") == 0)
		{
			// webrtc_wakeup_cpu1();
			webrtc_read_device_info();
			//printf("////////////////////");
		}
		else if (os_strcmp(argv[1], "stop") == 0)
		{
			// webrtc_shutdown_cpu1();
#if CONFIG_SYS_CPU0
			webrtc_mailbox_send_irled_off_msg();	
#endif		
		}
		else if (os_strcmp(argv[1], "lp") == 0)
		{
			// webrtc_shutdown_cpu1();
#if CONFIG_SYS_CPU0
			low_pwr_mode();
#endif		
		}
		else if (os_strcmp(argv[1], "test") == 0)
		{
			
			if (argc == 4)
			{
				webrtc_test(argv[2],argv[3]);
			}else{
				os_printf("格式:webrtc test 数字 数字\n");
			}			
			
		}
		else if (os_strcmp(argv[1], "burn") == 0)
		{

			if (argc == 4)
			{
				webrtc_burn(argv[2],argv[3]);
			}else{
				os_printf("格式:webrtc burn 序列号 websocket服务器地址\n");
			}
		}
		else if (os_strcmp(argv[1], "reboot") == 0)
		{
			webrtc_reboot();
		}
		else if (os_strcmp(argv[1], "open") == 0)
		{
#if CONFIG_SYS_CPU0
			webrtc_mailbox_send_irled_on_msg();
#endif
			//webrtc_write_bin();
		}
		else if (os_strcmp(argv[1], "connect") == 0)
		{
			if (argc == 4)
			{
				webrtc_connect_msg(argv[2], argv[3]);
			}
			else
			{
				// webrtc_connect_msg("8A","Yibao520");
				webrtc_connect_msg("newway", "01234567890");
				// webrtc_connect_msg("haichenghuang123","hhc120402");
			}
		}
		else if (os_strcmp(argv[1], "ls") == 0)
		{
			if (argc == 3)
			{
				webrtc_ls_files(argv[2]);
			}
		}
		else if (os_strcmp(argv[1], "del") == 0)
		{
			if (argc == 3)
			{
				webrtc_vfs_del_file(argv[2]);
			}
		}
		else if (os_strcmp(argv[1], "statfs") == 0)
		{
			if (argc == 3)
			{
				webrtc_vfs_statfs(argv[2]);
			}
		}
		else if (os_strcmp(argv[1], "mkdir") == 0)
		{
			if (argc == 3)
			{
				webrtc_vfs_mkdir(argv[2]);
			}
		}
		else if (os_strcmp(argv[1], "rmdir") == 0)
		{
			if (argc == 3)
			{
				webrtc_vfs_rmdir(argv[2]);
			}
		}
		else if (os_strcmp(argv[1], "sdcard") == 0)
		{
			if (argc == 3)
			{
				if (os_strcmp(argv[2], "format") == 0)
				{
					webrtc_vfs_format_sdcard();
				}
			}
		}
		else if (os_strcmp(argv[1], "mount") == 0)
		{
#if CONFIG_SYS_CPU0
			if (webrtc_mount_sdcard(1) != -1)
			{
				webrtc_mailbox_sdcard_mount(1);
			}
			else
				bk_printf("sd card driver mount failed!\n");
#endif

		}
		else if (os_strcmp(argv[1], "unmount") == 0)
		{
		}
		else if (os_strcmp(argv[1], "startrecord") == 0)
		{
			webrtc_mailbox_send_start_record_msg();
		}
		else if (os_strcmp(argv[1], "stoprecord") == 0)
		{
			webrtc_mailbox_send_stop_record_msg();
		}
		else if (os_strcmp(argv[1], "send") == 0)
		{
		}
		else if (os_strcmp(argv[1], "cp") == 0)
		{
			if (argc == 3)
			{
				if (os_strcmp(argv[2], "start") == 0)
				{
					webrtc_could_publish_stream_start();
				}
				else if (os_strcmp(argv[2], "stop") == 0)
				{
					webrtc_could_publish_stream_stop();
				}
			}
		}
		else if (os_strcmp(argv[1], "capture") == 0)
		{
			webrtc_capture();
		}
		else if (os_strcmp(argv[1], "sysdefault") == 0)
		{
			webrtc_set_default_system_info();
		}
	}
}
#define WEBRTC_CMD_CNT (sizeof(s_webrtc_commands) / sizeof(struct cli_command))
static const struct cli_command s_webrtc_commands[] = {
	{"webrtc", "start|stop|connect|free|top", cli_webrtc_cmd},
};
int cli_webrtc_init(void)
{
	return cli_register_commands(s_webrtc_commands, WEBRTC_CMD_CNT);
}
// #endif

void user_app_main(void)
{
}

int main(void)
{

	bk_printf("=============================main start in here=========================================\n");
#if (CONFIG_SYS_CPU0)
	rtos_set_user_app_entry((beken_thread_function_t)user_app_main);
	bk_set_printf_sync(true);		  // 设置bk输出同步
	shell_set_log_level(BK_LOG_WARN); // 设置日志等级 采取恢复措施的错误条件
#endif
	bk_init(); // 设置和初始化各种系统组件

// 初始化注册相关命令  使用注册命令s函数
#if (CONFIG_SYS_CPU0)					  // CPU0
	cli_webrtc_init();					  // webrtc命令初始化
#if (CONFIG_FATFS && CONFIG_FATFS_SDCARD) // FAT 文件系统
	cli_fatfs_init();					  // fatfs文件系统命令初始化   SD卡
#endif
	cli_mem_init();	  // 内存命令初始化
	cli_os_init();	  // 系统指令初始化
	cli_pwr_init();	  // 电源模式
	cli_flash_init(); // 设置闪存驱动，使闪存操作能够通过 CLI 被访问

#if CONFIG_OTA_HTTP
	cli_ota_init();
#endif
#if CONFIG_VFS
	cli_vfs_init(); // 虚拟文件系统初始化
#endif
#endif

#if (CONFIG_SYS_CPU1)
#if (CONFIG_FATFS && CONFIG_FATFS_SDCARD)
	cli_fatfs_init(); // fatfs文件系统命令初始化   SD卡
#endif
#if CONFIG_VFS // 虚拟文件系统初始化
	cli_vfs_init();
#endif
	cli_webrtc_init(); // webrtc命令初始化
	cli_mem_init();	   // 内存命令初始化
	cli_os_init();	   // 系统指令初始化

#endif
#if (CONFIG_SYS_CPU2)
	cli_mem_init();
	cli_os_init();
#endif
	webrtc_run();
	return 0;
}
