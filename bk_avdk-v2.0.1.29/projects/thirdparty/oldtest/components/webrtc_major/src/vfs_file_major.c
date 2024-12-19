#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include "vfs_file_major.h"
#include <errno.h>
#if CONFIG_VFS
#include "bk_vfs.h"
#include "bk_filesystem.h"
#endif

#define TAG "major_vfs_file"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)
extern void delay_us(UINT32 us);
static project_config_t *project_config = NULL;

static beken_thread_t globle_vfs_file_pid = NULL;
static beken_semaphore_t globle_vfs_file_sem = NULL;
static beken_semaphore_t globle_vfs_file_exit_sem = NULL;
static beken_mutex_t  globle_vfs_file_mutex = NULL;
static beken_queue_t globle_data_queue = NULL;
static bool globle_vfs_file_runing = false;
extern void delay_us(UINT32 us);
void vfs_file_handle_globel_req(webrtc_cmd_t *pcmd);
void vfs_file_handdle_globle_data_handle(void){
	 bk_err_t ret = BK_OK;
	 queue_msg_t msg;
	if(globle_vfs_file_mutex!=NULL){
		rtos_lock_mutex(&globle_vfs_file_mutex);
		while(rtos_is_queue_empty(&globle_data_queue) == false){
				ret = rtos_pop_from_queue(&globle_data_queue, &msg, 0);
				if (kNoErr == ret){       
					webrtc_cmd_t *pcmd = (webrtc_cmd_t *)msg.param;
					if(pcmd!= NULL){
						vfs_file_handle_globel_req(pcmd);			
					}
				}
		}
		rtos_unlock_mutex(&globle_vfs_file_mutex);
	}

}

void vfs_file_globle_thread(void *param){
	
	if(globle_vfs_file_sem==NULL || globle_data_queue ==NULL || globle_vfs_file_mutex ==NULL){
		goto exit_0;
	}
	bk_err_t ret = BK_OK;
	globle_vfs_file_runing = true;
	while(globle_vfs_file_runing){
		rtos_get_semaphore(&globle_vfs_file_sem, BEKEN_NEVER_TIMEOUT);
		vfs_file_handdle_globle_data_handle();
	}
	LOGW("%s %d exit thread \n", __func__, __LINE__);
	vfs_file_handdle_globle_data_handle();
	if(globle_vfs_file_exit_sem!= NULL){
			int count = rtos_get_semaphore_count(&globle_vfs_file_exit_sem);
			if(count == 0){				
				rtos_set_semaphore(&globle_vfs_file_exit_sem);
			}
	}
exit_0:
	if(globle_vfs_file_sem){
		rtos_deinit_semaphore(&globle_vfs_file_sem);
		globle_vfs_file_sem = NULL;
	}
	if (globle_data_queue)
	{
		rtos_deinit_queue(&globle_data_queue);
		globle_data_queue = NULL;
	}
        if(globle_vfs_file_mutex!= NULL){
        rtos_deinit_mutex(&globle_vfs_file_mutex);
	globle_vfs_file_mutex = NULL;
        }
	globle_vfs_file_pid = NULL;
	rtos_delete_thread(NULL);
}
 
void init_vfs_files(project_config_t *config){
	bk_err_t ret = BK_OK;
	project_config = config;
        rtos_init_mutex(&globle_vfs_file_mutex);
	if(globle_vfs_file_sem == NULL){
		    ret = rtos_init_semaphore_ex(&globle_vfs_file_sem, 1, 0);	
		    if (ret != BK_OK){
				LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
		    }
	}
	if(globle_vfs_file_exit_sem == NULL){
		    ret = rtos_init_semaphore_ex(&globle_vfs_file_exit_sem, 1, 0);	
		    if (ret != BK_OK){
				LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
		    }
	}

	    if(globle_data_queue == NULL){		
	   	   ret = rtos_init_queue(&globle_data_queue,"globle_data_queue",sizeof(queue_msg_t),128);
		    if (ret != BK_OK){
				LOGE("create mailbox_msg_queue fail\n");
		    }
	    }
	    if(globle_vfs_file_pid == NULL){
		    ret = rtos_create_psram_thread(&globle_vfs_file_pid,
							5,
							"glable_vfs_file",
							(beken_thread_function_t)vfs_file_globle_thread,
							16*1024,
							NULL);
		    if (ret != kNoErr) {
				LOGE("create sd file task fail \r\n");
					
		    }
	    }
}
void uninit_vfs_files(){
      globle_vfs_file_runing =false;
      if(globle_vfs_file_sem!= NULL){
	 rtos_set_semaphore(&globle_vfs_file_sem);
	if(globle_vfs_file_exit_sem){
		  rtos_get_semaphore(&globle_vfs_file_exit_sem, BEKEN_WAIT_FOREVER);
		  rtos_deinit_semaphore(&globle_vfs_file_exit_sem);
		  globle_vfs_file_exit_sem = NULL;
	}
      }
	project_config = NULL;
}
static void vfs_file_open(webrtc_cmd_t *pcmd){

    int fd = -1;
    if(pcmd!=NULL){
	    fd = bk_vfs_open((const char *)pcmd->param.szparam, pcmd->param.nparam1);
	    if (fd < 0) {
			LOGE("can't open %s\n", (char*)pcmd->param.szparam);
	    }
	    pcmd->result.result = fd;
    }

}
static void vfs_file_read(webrtc_cmd_t *pcmd){
    if(pcmd!= NULL){
	    int fd = pcmd->param.nparam1;
	    void *data = (void *)pcmd->param.param1;
	    size_t size = (size_t)pcmd->param.param2;
	    int len = bk_vfs_read(fd, data, size);
	    if(len<0){
		LOGE("read failed %d\n", len);
		
	    }
	    pcmd->result.result =len;
    }
}
static void vfs_file_write(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
	    int fd = pcmd->param.nparam1;
	    void *data = (void *)pcmd->param.param1;
	    char *writedata = (char*)data;
	    size_t length = (size_t)pcmd->param.param2;
	    size_t writelen = length;
	    int ret = length;
#if 0
	    while(length>0){
		    if(length>512){
			writelen = 512;
		    }else{
			writelen = length;
		    }
		    ret = bk_vfs_write(fd, writedata, writelen);
		    if(ret<0){
			LOGE("write failed %d\n", ret);
			break;
		
		    }
		    writedata+=ret;
		    length-=ret;
		    if(length>0){
		    	//delay_us(5);
		    }
	    }
#else
	     ret = bk_vfs_write(fd, writedata, writelen);
	     if(ret<0){
			LOGE("write failed %d\n", ret);		
	     }
#endif
	     pcmd->result.result =ret;
	}
}
static void vfs_file_close(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
	    int fd = pcmd->param.nparam1;
	    int ret = bk_vfs_close(fd);
	    if(ret<0){
		LOGE("close failed %d\n", ret);	
	    }
	     pcmd->result.result =ret;
	}
}
static void vfs_file_lseek(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int fd = pcmd->param.nparam1;
		int offset = pcmd->param.nparam2;
		int whence = pcmd->param.nparam3;
		int ret = bk_vfs_lseek(fd, offset, whence);
	    	if(ret<0){
			LOGE("lseek failed %d\n", ret);	
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_tell(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
	    int fd = pcmd->param.nparam1;
	    off_t ret = bk_vfs_tell(fd);
	    if(ret<0){
		LOGE("close failed %d\n", ret);	
	    }
	     pcmd->result.result = (int)ret;
	}
}
static void vfs_file_unlink(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		
		int ret = bk_vfs_unlink((const char *)pcmd->param.szparam);
	    	if(ret<0){
			LOGE("unlink failed %d\n", ret);	
	    	}
		 pcmd->result.result =ret;
	}
}

static void vfs_file_stat(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		
		int ret = bk_vfs_stat((const char *)pcmd->param.szparam,(struct stat *)pcmd->param.param1);
	    	if(ret<0){
			LOGE("stat failed %d \n", ret);	
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_mkdir(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int ret = bk_vfs_mkdir((const char *)pcmd->param.param1,(mode_t)pcmd->param.param2);
	    	if(ret<0){
			LOGE("mkdir failed %d\n", ret);
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_fstat(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int ret = bk_vfs_fstat((int)pcmd->param.param1,(struct stat *)pcmd->param.param2);
	    	if(ret<0){
			LOGE("fstat file %d failed %d\n", pcmd->param.param1,ret);
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_rename(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int ret = bk_vfs_rename((const char *)pcmd->param.param1,(const char  *)pcmd->param.param2);
	    	if(ret<0){
			LOGE("rename failed %d\n", ret);
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_fsync(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int ret = bk_vfs_fsync((int)pcmd->param.param1);
	    	if(ret<0){
			LOGE("fsync failed %d\n", ret);
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_ftruncate(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int ret = bk_vfs_ftruncate((int)pcmd->param.param1,(off_t)pcmd->param.param2);
	    	if(ret<0){
			LOGE("ftruncate failed %d\n", ret);
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_fcntl(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int ret = bk_vfs_fcntl((int)pcmd->param.param1,(int)pcmd->param.param2,(void *)pcmd->param.param3);
	    	if(ret<0){
			LOGE("fcntl failed %d\n", ret);
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_rmdir(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int ret = bk_vfs_rmdir((const char *)pcmd->param.param1);
	    	if(ret<0){
			LOGE("rmdir failed %d\n", ret);
	    	}
		 pcmd->result.result =ret;
	}
}
static void vfs_file_opendir(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		DIR * ret = bk_vfs_opendir((const char *)pcmd->param.param1);
	    	if(ret==NULL){
			LOGE("opendir failed \n");
	    	}
		 pcmd->result.result = (int)ret;
	}
}
static void vfs_file_closedir(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		int ret = bk_vfs_closedir((DIR *)pcmd->param.param1);
	    	if(ret<0){
			LOGE("closedir failed %d\n",ret);
	    	}
		 pcmd->result.result = (int)ret;
	}
}
static void vfs_file_readdir(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		struct dirent *ret = bk_vfs_readdir((DIR *)pcmd->param.param1);
	    	if(ret==NULL){
			//LOGE("readdir failed \n");
	    	}
		 pcmd->result.result = (int)ret;
	}
}
static void vfs_file_seekdir(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		bk_vfs_seekdir((DIR *)pcmd->param.param1,pcmd->param.lparam);
		pcmd->result.result = (int)0;
	}
}
static void vfs_file_telldir(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		long ret = bk_vfs_telldir((DIR *)pcmd->param.param1);
	    	if(ret<0){
			LOGE("closedir failed %d\n",ret);
	    	}
		 pcmd->param.lparam = ret;
	}
}
static void vfs_file_rewinddir(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		bk_vfs_rewinddir((DIR *)pcmd->param.param1);
		pcmd->result.result = 0;
	}
}
static void vfs_file_statfs(webrtc_cmd_t *pcmd){
	if(pcmd!= NULL){
		struct statfs statfsinfo ={0};
		int ret = bk_vfs_statfs((const char *)pcmd->param.param1,(struct statfs *)pcmd->param.param2);
	    	if(ret<0){
			LOGE("fstat failed %d\n", ret);
	    	}
		pcmd->result.result =ret;
	}
}
bool put_globle_vfs_file_req_cmd(webrtc_cmd_t *pcmd){
	bk_err_t ret = BK_OK;
	bool result = false;
	queue_msg_t msg;
	msg.param = (void*)pcmd;
	//LOGW("%s %d \n", __func__, __LINE__);
	if(globle_vfs_file_mutex!=NULL){
		rtos_lock_mutex(&globle_vfs_file_mutex);
		if (globle_data_queue!=NULL){
			//LOGW("%s %d 0x%x\n", __func__, __LINE__,pcmd->mb_cmd.param1);
		       ret = rtos_push_to_queue(&globle_data_queue, &msg, BEKEN_NO_WAIT);
			if (BK_OK != ret){
				LOGE("%s failed\n", __func__);
			}else{
			    result = true;
			    if(globle_vfs_file_sem!= NULL){
				rtos_set_semaphore(&globle_vfs_file_sem);
			    }
			}
		}
		rtos_unlock_mutex(&globle_vfs_file_mutex);
	}
	return result;
}
void vfs_file_handle_globel_req(webrtc_cmd_t *pcmd){
	if(pcmd->mb_cmd.param1 == VFS_FILE_COM_OPEN){
		vfs_file_open(pcmd);
		
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_READ){
		vfs_file_read(pcmd);

	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_WRITE){
		vfs_file_write(pcmd);

	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_CLOSE){
		vfs_file_close(pcmd);

	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_UNLINK){
		vfs_file_unlink(pcmd);

	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_LSEEK){
		vfs_file_lseek(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_TELL){
		vfs_file_tell(pcmd);

	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_STAT){
		vfs_file_stat(pcmd);

	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_MKDIR){
		vfs_file_mkdir(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_FSTAT){
		vfs_file_fstat(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_RENAME){
		vfs_file_rename(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_FSYNC){
		vfs_file_fsync(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_FTRUNCATE){
		vfs_file_ftruncate(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_FCNTL){
		vfs_file_fcntl(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_RMDIR){
		vfs_file_rmdir(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_OPENDIR){
		vfs_file_opendir(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_CLOSEDIR){
		vfs_file_closedir(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_READDIR){
		vfs_file_readdir(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_SEEKDIR){
		vfs_file_seekdir(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_TELLDIR){
		vfs_file_telldir(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_REWINDDIR){
		vfs_file_rewinddir(pcmd);
	}else if(pcmd->mb_cmd.param1 == VFS_FILE_COM_STATFS){
		vfs_file_statfs(pcmd);

	}
        if(project_config && project_config->mailbox_send_media_response_msg!= NULL){
		 	project_config->mailbox_send_media_response_msg(pcmd->mb_cmd.param1,pcmd->mb_cmd.param2,pcmd->mb_cmd.param3);
        }
}
void vfs_file_handle_req_rx(webrtc_cmd_t *pcmd){
	put_globle_vfs_file_req_cmd(pcmd);
	
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
