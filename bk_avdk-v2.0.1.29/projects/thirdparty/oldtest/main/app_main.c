#include "bk_private/bk_init.h"
#include <components/system.h>
#include <os/os.h>
#include <components/shell_task.h>



extern void user_app_main(void);
extern void rtos_set_user_app_entry(beken_thread_function_t entry);
extern void webrtc_run();

#if CONFIG_SYS_CPU1
#include "cli.h"
#endif
//#if CONFIG_SYS_CPU0
#include "cli.h"
extern void webrtc_test();
extern void webrtc_reboot();
extern void webrtc_wakeup_cpu1();
extern void webrtc_shutdown_cpu1();
extern void webrtc_set_mac_address();
extern void webrtc_set_default_system_info();
extern void webrtc_mailbox_send_start_record_msg(void);
extern void webrtc_mailbox_send_stop_record_msg(void);
extern void webrtc_connect_msg(char *ssid, char *key);
extern void webrtc_ls_files(char *path);
extern void webrtc_vfs_statfs(char *path);
extern void webrtc_vfs_del_file(char *path);
extern void webrtc_vfs_mkdir(char *path);
extern void webrtc_vfs_rmdir(char *path);
extern void webrtc_vfs_format_sdcard();
extern void webrtc_could_publish_stream_start();
extern void webrtc_could_publish_stream_stop();
extern void webrtc_capture();
static void cli_webrtc_cmd(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	if(argc>1){
		if (os_strcmp(argv[1], "start") == 0) {
			   //webrtc_wakeup_cpu1();		
		}else if (os_strcmp(argv[1], "stop") == 0) {
			//webrtc_shutdown_cpu1();
		}else if (os_strcmp(argv[1], "test") == 0) {
			webrtc_test();
		}else if (os_strcmp(argv[1], "reboot") == 0) {
			webrtc_reboot();

		}else if (os_strcmp(argv[1], "connect") == 0) {
			if(argc== 4){
				webrtc_connect_msg(argv[2],argv[3]);
			}else{
			//webrtc_connect_msg("8A","Yibao520");
			 // webrtc_connect_msg("runhua","01234567890");
			//webrtc_connect_msg("haichenghuang123","hhc120402");
			}
		}else if (os_strcmp(argv[1], "ls") == 0) {
		       if(argc== 3){
			  webrtc_ls_files(argv[2]);
		       }
		}else if (os_strcmp(argv[1], "del") == 0) {
		       if(argc== 3){
			  webrtc_vfs_del_file(argv[2]);
		       }
		}else if (os_strcmp(argv[1], "statfs") == 0) {
		       if(argc== 3){
			  webrtc_vfs_statfs(argv[2]);
		       }
		}else if (os_strcmp(argv[1], "mkdir") == 0) {
		       if(argc== 3){
			  webrtc_vfs_mkdir(argv[2]);
		       }
		}else if (os_strcmp(argv[1], "rmdir") == 0) {
		       if(argc== 3){
			  webrtc_vfs_rmdir(argv[2]);
		       }
		}else if (os_strcmp(argv[1], "sdcard") == 0) {
		       if(argc== 3){
			    if(os_strcmp(argv[2], "format") == 0) {
			        webrtc_vfs_format_sdcard();
			    }
		       }
		}else if (os_strcmp(argv[1], "mount") == 0) {
	      
		}else if (os_strcmp(argv[1], "unmount") == 0) {
	 

		}else if (os_strcmp(argv[1], "startrecord") == 0) {
			webrtc_mailbox_send_start_record_msg();
		}else if (os_strcmp(argv[1], "stoprecord") == 0) {
			webrtc_mailbox_send_stop_record_msg();
		}else if (os_strcmp(argv[1], "send") == 0) {
		}else if (os_strcmp(argv[1], "cp") == 0) {
			if(argc == 3){
				if (os_strcmp(argv[2], "start") == 0) {
					webrtc_could_publish_stream_start();
				}else if (os_strcmp(argv[2], "stop") == 0) {
					webrtc_could_publish_stream_stop();
				}
			}

		}else if (os_strcmp(argv[1], "capture") == 0) {
			webrtc_capture();
		}else if (os_strcmp(argv[1], "sysdefault") == 0){
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
//#endif

void user_app_main(void)
{
	
}

int main(void)
{	
#if (CONFIG_SYS_CPU0)
	rtos_set_user_app_entry((beken_thread_function_t)user_app_main);
	bk_set_printf_sync(true);
	shell_set_log_level(BK_LOG_WARN);
#endif
       bk_init();
#if (CONFIG_SYS_CPU0)
	cli_webrtc_init();
#if (CONFIG_FATFS && CONFIG_FATFS_SDCARD)
 	cli_fatfs_init();
#endif
	cli_mem_init();
	cli_os_init();
	cli_pwr_init();
	cli_flash_init();
#if CONFIG_VFS
	cli_vfs_init();
#endif
#endif
	
#if (CONFIG_SYS_CPU1)
#if (CONFIG_FATFS && CONFIG_FATFS_SDCARD)
 	cli_fatfs_init();
#endif
#if CONFIG_VFS
	cli_vfs_init();
#endif
	cli_webrtc_init();
	cli_mem_init();
	cli_os_init();
	
#endif
#if (CONFIG_SYS_CPU2)
	cli_mem_init();
	cli_os_init();
#endif
	webrtc_run();
	return 0;
}
