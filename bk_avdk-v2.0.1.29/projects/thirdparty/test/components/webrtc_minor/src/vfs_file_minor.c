#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include "vfs_file_minor.h"
#include <errno.h>

#define TAG "minor_vfs_file"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)
extern void delay_us(UINT32 us);
static project_config_t *project_config = NULL; 
void init_vfs_files(project_config_t *config){
	project_config = config;
}
void uninit_vfs_files(){
	project_config = NULL;
}
static void destory_mailbox_send_cmd_data(webrtc_cmd_t *cmd)
{  
	//LOGW("%s %d\n", __func__, __LINE__);
	   if(cmd){
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
}
int vfs_file_open(const char *pathname, int flags){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.nparam1 = flags;
				snprintf((char*)pcmd->param.szparam,sizeof(pcmd->param.szparam),"%s",pathname);
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_OPEN,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_read(int fd, void *buffer, int size){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.nparam1 = fd;
				pcmd->param.param1 = (u32)buffer;
				pcmd->param.param2 = size;
				
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_READ,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_write(int fd, const void *buf, int count){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.nparam1 = fd;
				pcmd->param.param1 = (u32)buf;
				pcmd->param.param2 = count;
				
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_WRITE,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}

int vfs_file_close(int fd){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.nparam1 = fd;
				
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_CLOSE,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_lseek(int fd, int offset, int whence){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.nparam1 = fd;
				pcmd->param.nparam2 = offset;
				pcmd->param.nparam3 = whence;
				
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_LSEEK,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_tell(int fd){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.nparam1 = fd;
				
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_TELL,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_unlink(const char *pathname){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				snprintf((char*)pcmd->param.szparam,sizeof(pcmd->param.szparam),"%s",pathname);
				
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_UNLINK,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_stat(const char *pathname, struct stat *buf){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				snprintf((char*)pcmd->param.szparam,sizeof(pcmd->param.szparam),"%s",pathname);
				pcmd->param.param1 = (u32)buf;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_STAT,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}

int vfs_file_fstat(int fd, struct stat *statbuf){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)fd;
				pcmd->param.param2 = (u32)statbuf;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_FSTAT,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_rename(const char *oldpath, const char *newpath){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)oldpath;
				pcmd->param.param2 = (u32)newpath;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_RENAME,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}

int vfs_file_fsync(int fd){
        int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)fd;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_FSYNC,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_ftruncate(int fd, off_t offset){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)fd;
				pcmd->param.param2 = (u32)offset;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_FTRUNCATE,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_fcntl(int fd, int cmd, void *arg){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)fd;
				pcmd->param.param2 = (u32)cmd;
				pcmd->param.param3 = (u32)arg;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_FCNTL,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}

int vfs_file_mkdir(const char *pathname, mode_t mode){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)pathname;
				pcmd->param.param2 = (u32)mode;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_MKDIR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_rmdir(const char *pathname){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)pathname;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_RMDIR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}

DIR *vfs_file_opendir(const char *name){
        DIR *result = NULL;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)name;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_OPENDIR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = (DIR *)pcmd->result.result;
				}else{
					result = NULL;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
int vfs_file_closedir(DIR *dirp){
	int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)dirp;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_CLOSEDIR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;
}
struct dirent *vfs_file_readdir(DIR *dirp){
        struct dirent  *result = NULL;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)dirp;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_READDIR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = (struct dirent *)pcmd->result.result;
				}else{
					result = NULL;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }
     return result;
}
void vfs_file_seekdir(DIR *dirp, long loc){
       int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)dirp;
				pcmd->param.lparam = loc;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_SEEKDIR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }
	
}
long vfs_file_telldir(DIR *dirp){
        long result = 0;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)dirp;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_TELLDIR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }

        return result;

}
void vfs_file_rewinddir(DIR *dirp){
        int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)dirp;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_REWINDDIR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }



}
int vfs_file_statfs(const char *path, struct statfs *buf){
        int result = -1;
	if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.param1 = (u32)path;
				pcmd->param.param2 = (u32)buf;
				int res = project_config->mailbox_send_media_req_msg(VFS_FILE_COM_STATFS,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					result = pcmd->result.result;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
	  }
	return result;
}
void vfs_file_handle_req_rx(webrtc_cmd_t *pcmd){
        if(project_config && project_config->mailbox_send_media_response_msg!= NULL){
		 	project_config->mailbox_send_media_response_msg(pcmd->mb_cmd.param1,pcmd->mb_cmd.param2,pcmd->mb_cmd.param3);
        }
}
void vfs_file_handle_resp_rx(webrtc_cmd_t *pcmd){
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

