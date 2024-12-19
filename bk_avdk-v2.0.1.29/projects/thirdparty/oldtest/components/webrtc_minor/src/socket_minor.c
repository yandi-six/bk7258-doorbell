#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include "socket_minor.h"
#include <errno.h>

#define TAG "minor_socket"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)
extern void delay_us(UINT32 us);
static beken_mutex_t  g_socket_mutex = NULL;
static sockets_t gsockets[MAX_SOCKETS] = {0};
static project_config_t *project_config = NULL; 

static beken_thread_t globle_sockets_pid = NULL;
static beken_semaphore_t globle_sockets_sem = NULL;
static beken_semaphore_t globle_sockets_exit_sem = NULL;
static beken_mutex_t  globle_sockets_mutex = NULL;
static beken_queue_t globle_data_queue = NULL;
static bool globle_socket_runing = false;

void socket_handle_globel_req(webrtc_cmd_t *pcmd);
void hand_globle_data_handle(void){
	 bk_err_t ret = BK_OK;
	 queue_msg_t msg;
	if(globle_sockets_mutex!=NULL){
		rtos_lock_mutex(&globle_sockets_mutex);
		while(rtos_is_queue_empty(&globle_data_queue) == false){
				ret = rtos_pop_from_queue(&globle_data_queue, &msg, 0);
				if (kNoErr == ret){       
					webrtc_cmd_t *pcmd = (webrtc_cmd_t *)msg.param;
					if(pcmd!= NULL){
						socket_handle_globel_req(pcmd);			
					}
				}
		}
		rtos_unlock_mutex(&globle_sockets_mutex);
	}

}

void socket_globle_thread(void *param){
	
	if(globle_sockets_sem==NULL || globle_data_queue ==NULL || globle_sockets_mutex ==NULL){
		goto exit_0;
	}
	bk_err_t ret = BK_OK;
	globle_socket_runing = true;
	while(globle_socket_runing){
		rtos_get_semaphore(&globle_sockets_sem, BEKEN_NEVER_TIMEOUT);
		if(globle_socket_runing){
		    hand_globle_data_handle();
		}
	}
	
	hand_globle_data_handle();
	LOGW("%s %d exit thread \n", __func__, __LINE__);
	if(globle_sockets_exit_sem!= NULL){
			int count = rtos_get_semaphore_count(&globle_sockets_exit_sem);
			if(count == 0){				
				rtos_set_semaphore(&globle_sockets_exit_sem);
			}
	}
exit_0:
	if(globle_sockets_sem){
		rtos_deinit_semaphore(&globle_sockets_sem);
		globle_sockets_sem = NULL;
	}
	if (globle_data_queue)
	{
		rtos_deinit_queue(&globle_data_queue);
		globle_data_queue = NULL;
	}
        if(globle_sockets_mutex!= NULL){
        rtos_deinit_mutex(&globle_sockets_mutex);
	globle_sockets_mutex = NULL;
        }
	globle_sockets_pid = NULL;
	rtos_delete_thread(NULL);
}



void init_sockets(project_config_t *config){
	    bk_err_t ret = BK_OK;
	    rtos_init_mutex(&g_socket_mutex);
	    rtos_init_mutex(&globle_sockets_mutex);
	    
	    project_config = config;
	    int i = 0;
	    for(i = 0;i<MAX_SOCKETS;i++){
		  gsockets[i].socket = NULL;
	    }
	    if(globle_sockets_sem == NULL){
		    ret = rtos_init_semaphore_ex(&globle_sockets_sem, 1, 0);	
		    if (ret != BK_OK){
				LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
		    }
	    }
	    if(globle_sockets_exit_sem == NULL){
		    ret = rtos_init_semaphore_ex(&globle_sockets_exit_sem, 1, 0);	
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
	    if(globle_sockets_pid == NULL){
		    ret = rtos_create_psram_thread(&globle_sockets_pid,
							5,
							"glable_socket_minor",
							(beken_thread_function_t)socket_globle_thread,
							8*1024,
							NULL);
		    if (ret != kNoErr) {
				LOGE("create socket task fail \r\n");
					
		    }
	    }

}
void uninit_sockets(){
    globle_socket_runing =false;
    if(globle_sockets_sem!= NULL){
	 rtos_set_semaphore(&globle_sockets_sem);
	if(globle_sockets_exit_sem){
		  rtos_get_semaphore(&globle_sockets_exit_sem, BEKEN_WAIT_FOREVER);
		  rtos_deinit_semaphore(&globle_sockets_exit_sem);
		  globle_sockets_exit_sem = NULL;
	}
     }
    if(g_socket_mutex!= NULL){
      rtos_deinit_mutex(&g_socket_mutex);
    }
     project_config = NULL;
}

static void destory_redv_data(recv_data_t *data){	   
	   if(data!= NULL){
		   if(data->data!= NULL){
			rtc_bk_free(data->data);
		    }
		   rtc_bk_free(data);
		   data = NULL;
	   }
}
static void destory_all_recvs_data(sockets_impl_t *psocket){
	bk_err_t ret = BK_OK;
	recv_data_t *pdata = NULL;
	queue_msg_t msg;
	if(psocket->recv_data_queue!= NULL){
		while(rtos_is_queue_empty(&psocket->recv_data_queue) == false){
				ret = rtos_pop_from_queue(&psocket->recv_data_queue, &msg, 0);
				if (kNoErr == ret)
				{       
					pdata = (recv_data_t *)msg.param;
					if(pdata!= NULL){
						destory_redv_data(pdata);			
					}
				}
		}
	}
}

static void del_all_recv_data(sockets_impl_t *psocket){
	rtos_lock_mutex(&psocket->socket_mutex);
	if(psocket->recv_data_list!= NULL){
		rtc_list_for_each(psocket->recv_data_list, (void (*)(void*))destory_redv_data);		
		rtc_list_free(psocket->recv_data_list);
		psocket->recv_data_list = NULL;
	}
	rtos_unlock_mutex(&psocket->socket_mutex);
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

void socket_handle_globel_req(webrtc_cmd_t *pcmd){
	if(pcmd!=NULL){
		//LOGE("%s %d   %d\n", __func__, __LINE__,pcmd->mb_cmd.param1);
		 if(pcmd->mb_cmd.param1 == SOCKET_COM_ERROR){
				int s= pcmd->param.nparam1;
				//LOGE("%s %d  socket =%d\n", __func__, __LINE__,s);		
				if(s>=0 && s<MAX_SOCKETS){
					sockets_impl_t *psocket = gsockets[s].socket;
					if(psocket!= NULL){
											
						psocket->last_error = pcmd->param.nparam2;
					        pcmd->result.result = 1;
						//LOGE("%s %d socket = %d error = %d\n", __func__, __LINE__,s,psocket->last_error);
						if(psocket->socket_select_sem!= NULL && psocket->select_waited ==true){
								int count = rtos_get_semaphore_count(&psocket->socket_select_sem);
								if(count == 0){
									rtos_set_semaphore(&psocket->socket_select_sem);
								}
						}

					}
				}

		  }else if(pcmd->mb_cmd.param1 == SOCKET_COM_RECV){
			//LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);	
			if(pcmd){
				if(pcmd->msgid != MESSAGE_HEAD_ID){
					LOGE("%s %d    0x%x\n", __func__, __LINE__,pcmd->msgid);
				}
				int s= pcmd->param.nparam1;
			
				if(s>=0 && s<MAX_SOCKETS){

						sockets_impl_t *psocket = gsockets[s].socket;
						if(psocket!= NULL && pcmd->recvdata.len>0 && psocket->closeed == false){
							recv_data_t *precvdata = (recv_data_t*)rtc_bk_malloc(sizeof(recv_data_t));
							if(precvdata){
								rtc_bk_memset(precvdata,0,sizeof(recv_data_t));
								precvdata->data = (void*)rtc_bk_malloc(pcmd->recvdata.len);
								if(precvdata->data!= NULL && pcmd->recvdata.data!= NULL && pcmd->recvdata.len>0){
									rtc_bk_memcpy(precvdata->data,pcmd->recvdata.data,pcmd->recvdata.len);
									precvdata->len = pcmd->recvdata.len;
									//LOGW("%s %d  %d  %s\n", __func__, __LINE__,precvdata->len,(char*)pcmd->recvdata.data);
									if(pcmd->recvdata.fromlen>0 && pcmd->recvdata.fromlen<sizeof(precvdata->fromaddr)){
										rtc_bk_memcpy(precvdata->fromaddr,pcmd->recvdata.fromaddr,pcmd->recvdata.fromlen);
										precvdata->fromlen = pcmd->recvdata.fromlen;
									}
									pcmd->result.result = pcmd->recvdata.len;
									//LOGW("%s %d    %d   %d\n", __func__, __LINE__,s,pcmd->recvdata.len);
#if 0
									queue_msg_t msg;
									msg.param = (void*)precvdata;
									int ret = rtos_push_to_queue(&psocket->recv_data_queue, &msg, BEKEN_NO_WAIT);
									if (BK_OK != ret)
									{
										LOGW("%s %d \n", __func__, __LINE__);
									}
#else
									rtos_lock_mutex(&psocket->socket_mutex);
									psocket->recv_data_list = rtc_list_append(psocket->recv_data_list, (void*)precvdata);
									rtos_unlock_mutex(&psocket->socket_mutex);
#endif

								}else{
									rtc_bk_free(precvdata);
								}
							}						
							if(psocket->socket_select_sem!= NULL && psocket->select_waited ==true){
								int count = rtos_get_semaphore_count(&psocket->socket_select_sem);
								if(count == 0){
									rtos_set_semaphore(&psocket->socket_select_sem);
								}
							}
						}
				}else{
					LOGE("%s %d \n", __func__, __LINE__);
				}
			}else{
				LOGE("%s %d \n", __func__, __LINE__);
			}	
		}
	}
       if(project_config && project_config->mailbox_send_media_response_msg!= NULL){
		 	project_config->mailbox_send_media_response_msg(pcmd->mb_cmd.param1,pcmd->mb_cmd.param2,pcmd->mb_cmd.param3);
       }
}
bool put_globle_sockets_req_cmd(webrtc_cmd_t *pcmd){
	bk_err_t ret = BK_OK;
	bool result = false;
	queue_msg_t msg;
	msg.param = (void*)pcmd;
	//LOGW("%s %d \n", __func__, __LINE__);
	if(globle_sockets_mutex!=NULL){
		rtos_lock_mutex(&globle_sockets_mutex);
		if (globle_data_queue!=NULL){
			//LOGW("%s %d 0x%x\n", __func__, __LINE__,pcmd->mb_cmd.param1);
		       ret = rtos_push_to_queue(&globle_data_queue, &msg, BEKEN_NO_WAIT);
			if (BK_OK != ret){
				LOGE("%s failed\n", __func__);
			}else{
			    result = true;
			    if(globle_sockets_sem!= NULL){
				int count = rtos_get_semaphore_count(&globle_sockets_sem);
				if(count == 0){
					rtos_set_semaphore(&globle_sockets_sem);
				}
			    }
			}
		}
		rtos_unlock_mutex(&globle_sockets_mutex);
	}
	return result;
}
void socket_handle_req_rx(webrtc_cmd_t *pcmd){
	 //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
	put_globle_sockets_req_cmd(pcmd);
	  
}
void socket_handle_resp_rx(webrtc_cmd_t *pcmd){
        //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
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
int socket(int domain, int type, int protocol){
	//LOGW("%s %d\n", __func__, __LINE__);
	int result = -1;
	sockets_impl_t *psocket = (sockets_impl_t *)rtc_bk_malloc(sizeof(sockets_impl_t));
	if(psocket){
		rtc_bk_memset(psocket,0,sizeof(sockets_impl_t));
		psocket->sock = -1;
		psocket->closeed = false;
		psocket->recv_data_list = NULL;
		psocket->last_error = 0;
		rtos_init_queue(&psocket->recv_data_queue,"recv_data_queue",sizeof(queue_msg_t),128);
		rtos_init_semaphore_ex(&psocket->socket_select_sem, 1, 0);
		rtos_init_mutex(&psocket->socket_mutex);	
		if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd!= NULL){
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
				pcmd->param.param1 = domain;
				pcmd->param.param2 = type;
				pcmd->param.param3 = protocol;
				pcmd->result.result = -1;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_CREATE,(u32)pcmd,(u32)psocket);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
				}

				int s = pcmd->result.result;
				

				//LOGE("%s %d create socket  %d\n", __func__, __LINE__,s);
				if(s>=0 && s<MAX_SOCKETS){				   
				   gsockets[s].socket = psocket;
				   psocket->sock = s;
				   result = s;
				}else{
				   rtos_deinit_semaphore(&psocket->socket_select_sem);
				   rtos_deinit_mutex(&psocket->socket_mutex);
				   rtc_bk_free(psocket);
				   psocket = NULL;
				}
				destory_mailbox_send_cmd_data(pcmd);
			}
			
			
		}else{
			LOGE("%s %d socket\n", __func__, __LINE__);
		}
        }
	return result;
}
int close(int s){
	//LOGW("%s %d  socket  =  %d\n", __func__, __LINE__,s);
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
		  int result;
		  psocket->closeed = true;
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
				pcmd->param.param1 = s;
				//LOGW("%s %d  socket  =  %d\n", __func__, __LINE__,s);
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_CLOSE,(u32)pcmd,(u32)psocket);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
						rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					destory_all_recvs_data(psocket);
					del_all_recv_data(psocket);
					if(psocket->socket_select_sem){
						rtos_deinit_semaphore(&psocket->socket_select_sem);
					}
					if(psocket->socket_mutex){
						rtos_deinit_mutex(&psocket->socket_mutex);
					}
					if (psocket->recv_data_queue!= NULL){
						rtos_deinit_queue(&psocket->recv_data_queue);
						psocket->recv_data_queue = NULL;
					}
					rtc_bk_free(psocket);
					gsockets[s].socket = NULL;
					result = 1;
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				return result;
			}
			
			
		  }

		}
	}
	return -1;
}
int accept(int s, struct sockaddr *addr, socklen_t *addrlen){
	LOGD("%s %d  socket = %d\n", __func__, __LINE__,s);
	int result = 0;
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){

		  if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			//LOGE("%s %d --------------------==== sock = %d    %d   %p   %d\n", __func__, __LINE__,s,psocket->sock,name,namelen);
			//LOGE("%s %d socket .....\n", __func__, __LINE__);
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				bk_err_t ret = BK_OK;
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
						LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
						LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}	
				pcmd->param.param1 = s;
				pcmd->param.param2 = (u32)addr;
				pcmd->param.param3 = (u32)addrlen;
				pcmd->result.result = -1;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_ACCEPT,(u32)pcmd,(u32)psocket);
				if(res == BK_OK){
					LOGW("%s %d\n", __func__, __LINE__);
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
					  	rtos_unlock_mutex(&pcmd->mutex);
					  	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
					result = pcmd->result.result;
					if(result>=0){
						LOGD("%s %d  socket = %d addrlen =  %d\n", __func__, __LINE__,result,pcmd->result.resultlen);
						if(pcmd->result.resultlen>0 && pcmd->result.resultlen<= sizeof(struct sockaddr)){
							rtc_bk_memcpy((void *)addr,pcmd->result.szresult,pcmd->result.resultlen);
							*addrlen=(socklen_t)pcmd->result.resultlen;
						}
						LOGD("%s %d sa_family .....%d   socket  %d\n", __func__, __LINE__,addr->sa_family,result);

						sockets_impl_t *psocket = (sockets_impl_t *)rtc_bk_malloc(sizeof(sockets_impl_t));
						if(psocket){
							rtc_bk_memset(psocket,0,sizeof(sockets_impl_t));
							psocket->sock = result;
							psocket->closeed = false;
							psocket->recv_data_list = NULL;
							psocket->last_error = 0;
							rtos_init_queue(&psocket->recv_data_queue,"recv_data_queue",sizeof(queue_msg_t),128);
							rtos_init_semaphore_ex(&psocket->socket_select_sem, 1, 0);
							rtos_init_mutex(&psocket->socket_mutex);
							gsockets[result].socket = psocket;
							
						}

					}
				}else{
					result = -1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				return result;
			}		
			
		  }

		}
	}
	return 0;
}
int bind(int s, const struct sockaddr *name, socklen_t namelen){
	//LOGW("%s %d\n", __func__, __LINE__);
	int result = 0;
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){

		  if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			//LOGE("%s %d --------------------==== sock = %d    %d   %p   %d\n", __func__, __LINE__,s,psocket->sock,name,namelen);
			//LOGE("%s %d socket .....\n", __func__, __LINE__);
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				bk_err_t ret = BK_OK;
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
						LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
						LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}	
				pcmd->param.param1 = s;
				pcmd->param.param2 = (u32)name;
				pcmd->param.param3 = namelen;
				pcmd->result.result = -1;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_BIND,(u32)pcmd,(u32)psocket);
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
				return result;
			}		
			
		  }

		}
	}
	return 0;
}
int shutdown(int s, int how){
	//LOGW("%s %d\n", __func__, __LINE__);
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
			 int result = 0;
			 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
					webrtc_cmd_t webrtc_cmd = {0};
					webrtc_cmd_t *pcmd = &webrtc_cmd;
					if(pcmd!= NULL){
						bk_err_t ret = BK_OK;
						rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
						ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
						if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
						}
						ret = rtos_init_mutex(&pcmd->mutex);
						if (ret != BK_OK){
								LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
						}
						pcmd->param.param1 = s;
						pcmd->param.param2 = how;
						int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_SHUTDOWN,(u32)pcmd,(u32)psocket);
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
						return result;				
					}
			
			
			  }
	  }
	}
	return 0;
}
int getpeername (int s, struct sockaddr *name, socklen_t *namelen){
	//LOGW("%s %d\n", __func__, __LINE__);
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
			 int result = 0;
			 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
					webrtc_cmd_t webrtc_cmd = {0};
					webrtc_cmd_t *pcmd = &webrtc_cmd;
					if(pcmd!= NULL){
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
						pcmd->param.param1 = s;
						int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_GETPEERNAME,(u32)pcmd,(u32)psocket);
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
							int len = pcmd->result.resultlen;
							rtc_bk_memcpy((void*)name,pcmd->result.szresult,len);
							*namelen = len;
						}else{
							result = -1;
						}
						destory_mailbox_send_cmd_data(pcmd);
						return result;				
					}
			
			
			  }
	  }
	}
	return 0;
}
int getsockname (int s, struct sockaddr *name, socklen_t *namelen){
	//LOGW("%s %d\n", __func__, __LINE__);
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
			 int result = 0;
			 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
					webrtc_cmd_t webrtc_cmd = {0};
					webrtc_cmd_t *pcmd = &webrtc_cmd;
					if(pcmd!= NULL){
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
						pcmd->param.param1 = s;
						int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_GETSOCKNAME,(u32)pcmd,(u32)psocket);
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
							int len = pcmd->result.resultlen;
							rtc_bk_memcpy((void*)name,pcmd->result.szresult,len);
							*namelen = len;
							//LOGE("%s %d  sa_family = %d\n", __func__, __LINE__,name->sa_family);
						}else{
							result = -1;
						}
						destory_mailbox_send_cmd_data(pcmd);
						return result;				
					}
			
			
			  }
	  }
	}
	return 0;
}
int getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen){
	//LOGW("%s %d\n", __func__, __LINE__);
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
			 int result = 0;
			 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
					webrtc_cmd_t webrtc_cmd = {0};
					webrtc_cmd_t *pcmd = &webrtc_cmd;
					if(pcmd!= NULL){
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
						pcmd->param.param1 = s;
						pcmd->param.param2 = level;
						pcmd->param.param3 = optname;
						pcmd->param.param4 = *optlen;
						int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_GETSOCKOPT,(u32)pcmd,(u32)psocket);
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
							int len = pcmd->result.resultlen;
							rtc_bk_memcpy(optval,pcmd->result.szresult,len);
							*optlen = len;
						 }else{
							result = -1;
						 }

						destory_mailbox_send_cmd_data(pcmd);
						return result;				
					}
			
			
			  }
	  }
	}
	return 0;
}
int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen){
	//LOGW("%s %d\n", __func__, __LINE__);
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
			 int result = 0;
			 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
					webrtc_cmd_t webrtc_cmd = {0};
					webrtc_cmd_t *pcmd = &webrtc_cmd;
					if(pcmd!= NULL){
						//LOGW("%s %d rtc_bk_malloc = %d \n", __func__, __LINE__,sizeof(webrtc_cmd_t));
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
						pcmd->param.param1 = s;
						pcmd->param.param2 = level;
						pcmd->param.param3 = optname;
						pcmd->param.param4 = optlen;
						rtc_bk_memcpy((void*)pcmd->param.szparam,(void*)optval,optlen);
						int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_SETSOCKOPT,(u32)pcmd,(u32)psocket);
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
						return result;				
					}
			
			
			  }
	  }
	}
	return 0;
}


int connect(int s, const struct sockaddr *name, socklen_t namelen){
	//LOGW("%s %d\n", __func__, __LINE__);
   	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL ){
		      int result;
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
				pcmd->param.param1 = s;
				rtc_bk_memcpy(pcmd->senddata.toaddr,(void *)name,namelen);
				pcmd->senddata.tolen = namelen;
		
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_CONNECT,(u32)pcmd,(u32)psocket);
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
					result = 1;
				}
				destory_mailbox_send_cmd_data(pcmd);
				return result;
			}
			
			
		  }
			

		}
   	}
	return -1;
}
int listen(int s, int backlog){
	//LOGW("%s %d\n", __func__, __LINE__);
   	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL ){
		      int result;
 		      if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
					destory_mailbox_send_cmd_data(pcmd);
				}else{
					ret = rtos_init_mutex(&pcmd->mutex);
					if (ret != BK_OK){
						LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
						destory_mailbox_send_cmd_data(pcmd);
					}else{
						pcmd->param.param1 = s;
						pcmd->param.param2 = backlog;
		
						int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_LISTEN,(u32)pcmd,(u32)psocket);
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
							result = 1;
						}
						destory_mailbox_send_cmd_data(pcmd);
						return result;
					}
				}
			}
			
			
		  }
			

		}
   	}
	return -1;
}
ssize_t recv(int s, void *mem, size_t len, int flags){
#if 0
	bk_err_t ret = BK_OK;
	int result = 0;
	queue_msg_t msg;
   	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL && psocket->recv_data_queue!= NULL){
			if(rtos_is_queue_empty(&psocket->recv_data_queue) == false){
				ret = rtos_pop_from_queue(&psocket->recv_data_queue, &msg, 0);
				if (kNoErr == ret){
					recv_data_t *pdata = (recv_data_t *)msg.param;
					if(pdata!= NULL && pdata->data!= NULL && pdata->len<=len){
						rtc_bk_memcpy(mem,pdata->data,pdata->len);
						result =  pdata->len;
						destory_redv_data(pdata);
						
					}
				}
			}
		}
   	}
	return result;
#else
	
        bool copyall = false;
	size_t copylen = 0;
	int result = 0;
 	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL && psocket->recv_data_list!= NULL){
			rtos_lock_mutex(&psocket->socket_mutex);
			recv_data_t *pdata = (recv_data_t *)rtc_list_nth_data(psocket->recv_data_list,0);
			if(pdata!= NULL && pdata->data!= NULL){
				if(len<pdata->len){
					copyall = false;
					copylen = len;
				}else{
					copyall = true;
					copylen = pdata->len;
				}
				//LOGW("%s %d     %d\n", __func__, __LINE__,copylen);
				rtc_bk_memcpy(mem,pdata->data,copylen);
				if(copyall == true){
					psocket->recv_data_list = rtc_list_remove(psocket->recv_data_list,pdata);
				        destory_redv_data(pdata);
				}else{
				   
					void *tmp = pdata->data+copylen;
					size_t left = pdata->len-copylen;
					rtc_bk_memcpy(pdata->data,tmp,left);
					pdata->len = left;
				}
				result = copylen;
			}
			rtos_unlock_mutex(&psocket->socket_mutex);

		}
   	}

	return result;
#endif

}
ssize_t read(int s, void *mem, size_t len){
	//LOGD("%s %d\n", __func__, __LINE__);
        bool copyall = false;
	size_t copylen = 0;
	int result = 0;
 	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL && psocket->recv_data_list!= NULL){
			rtos_lock_mutex(&psocket->socket_mutex);
			recv_data_t *pdata = (recv_data_t *)rtc_list_nth_data(psocket->recv_data_list,0);
			if(pdata!= NULL && pdata->data!= NULL){
				if(len<pdata->len){
					copyall = false;
					copylen = len;
				}else{
					copyall = true;
					copylen = pdata->len;
				}
				
				rtc_bk_memcpy(mem,pdata->data,copylen);
				if(copyall == true){
					
					psocket->recv_data_list = rtc_list_remove(psocket->recv_data_list,pdata);
				        destory_redv_data(pdata);
				}else{
				   
					void *tmp = pdata->data+copylen;
					size_t left = pdata->len-copylen;
					rtc_bk_memcpy(pdata->data,tmp,left);
					pdata->len = left;
				}
				result = copylen;
			}
			rtos_unlock_mutex(&psocket->socket_mutex);

		}
   	}

	return result;
}
ssize_t recvfrom(int s, void *mem, size_t len, int flags,
        struct sockaddr *from, socklen_t *fromlen){
	//LOGW("%s  %d \n", __func__, __LINE__);
#if 0
	bk_err_t ret = BK_OK;
	int result = 0;
	queue_msg_t msg;
   	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL && psocket->recv_data_queue!= NULL){
			if(rtos_is_queue_empty(&psocket->recv_data_queue) == false){
				ret = rtos_pop_from_queue(&psocket->recv_data_queue, &msg, 0);
				if (kNoErr == ret){
					recv_data_t *pdata = (recv_data_t *)msg.param;
					if(pdata!= NULL && pdata->data!= NULL && pdata->len<=len){
						rtc_bk_memcpy(mem,pdata->data,pdata->len);
						rtc_bk_memcpy((void*)from,pdata->fromaddr,pdata->fromlen);
						*fromlen = pdata->fromlen;
						result =  pdata->len;
						destory_redv_data(pdata);
						
					}
				}
			}
		}
   	}
	return result;
#else
        bool copyall = false;
	size_t copylen = 0;
	int result = 0;
 	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL ){
			if(psocket->recv_data_list!= NULL){
				rtos_lock_mutex(&psocket->socket_mutex);
				recv_data_t *pdata = (recv_data_t *)rtc_list_nth_data(psocket->recv_data_list,0);
				if(pdata!= NULL && pdata->data!= NULL){
					if(len<pdata->len){
						copyall = false;
						copylen = len;
					}else{
						copyall = true;
						copylen = pdata->len;
					}
				
					rtc_bk_memcpy(mem,pdata->data,copylen);
					if(copyall == true){
						psocket->recv_data_list = rtc_list_remove(psocket->recv_data_list,pdata);
						destory_redv_data(pdata);
					}else{
					   
						void *tmp = pdata->data+copylen;
						size_t left = pdata->len-copylen;
						rtc_bk_memcpy(pdata->data,tmp,left);
						pdata->len = left;
					}
					socklen_t addfromlen = *fromlen;
					if(pdata->fromlen>0 && addfromlen>=pdata->fromlen){
						 
						rtc_bk_memcpy((void*)from,pdata->fromaddr,addfromlen);
						*fromlen = addfromlen;
					}
					result =  copylen;
				}
				rtos_unlock_mutex(&psocket->socket_mutex);
			}

		}else{
			result = -1;
		}
   	}

	return result;
#endif
}
ssize_t recvmsg(int s, struct msghdr *message, int flags){
	return 0;
}
ssize_t send(int s, const void *dataptr, size_t size, int flags){
	//LOGW("%s %d\n", __func__, __LINE__);
	int result = -1;
     	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){

		  if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				//LOGW("sendto  %d",s);
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
				pcmd->result.result = 0;
				pcmd->responseed = 0;
				pcmd->txstate = 0;
				pcmd->param.param1 = s;
				pcmd->senddata.data = rtc_bk_malloc(size);
				if(pcmd->senddata.data!= NULL){
					rtc_bk_memcpy(pcmd->senddata.data,(void *)dataptr,size);
					pcmd->senddata.len = size;
					pcmd->senddata.size = size;
					pcmd->senddata.flags = flags;
					int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_SEND,(u32)pcmd,(u32)psocket);
					if(res == BK_OK){
						rtos_lock_mutex(&pcmd->mutex);
						if(pcmd->responseed == 0){
							pcmd->isWaited = 1;
						   	rtos_unlock_mutex(&pcmd->mutex);
						   	rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
						}else{
							rtos_unlock_mutex(&pcmd->mutex);
						}
						result =  pcmd->result.result;
						if(result<0){
						    psocket->last_error = result;
						}
					}else{
					  	result = 0;
					}
				}
				destory_mailbox_send_cmd_data(pcmd);
			}
		  }

		}
	}
	return result;
}
ssize_t sendmsg(int s, const struct msghdr *message, int flags){
	return 0;
}
ssize_t sendto(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen){
#if 0
     //LOGW("%s %d\n", __func__, __LINE__);
      int result = -1;
     if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
		  if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				//LOGW("sendto  %d",s);	
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				pcmd->result.result = 0;
				pcmd->responseed = 0;
				pcmd->txstate = 0;
				pcmd->param.param1 = s;				
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->senddata.data = rtc_bk_malloc(size);
				if(pcmd->senddata.data!=NULL){
					rtc_bk_memcpy(pcmd->senddata.data,(void *)dataptr,size);
					pcmd->senddata.len = size;
					pcmd->senddata.size = size;
					pcmd->senddata.flags = flags;
					rtc_bk_memcpy(pcmd->senddata.toaddr,(void *)to,tolen);
					pcmd->senddata.tolen = tolen;
					int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_SEND,(u32)pcmd,(u32)psocket);
					if(res == BK_OK){
						rtos_lock_mutex(&pcmd->mutex);
						if(pcmd->responseed == 0){
							 pcmd->isWaited = 1;
							 rtos_unlock_mutex(&pcmd->mutex);
						 	 rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
						}else{
							rtos_unlock_mutex(&pcmd->mutex);
						}								
						result =  pcmd->result.result;						
					}else{
						result =  0;
					}
				}else{
					result =  0;
				}
			      destory_mailbox_send_cmd_data(pcmd);
			}
			
			
		  }

		}
	}
	return result;
#endif

int result = -1;
     if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
		  if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				//LOGW("sendto  %d",s);	
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				pcmd->result.result = 0;
				pcmd->responseed = 0;
				pcmd->txstate = 0;
				pcmd->param.param1 = s;				
				bk_err_t ret = BK_OK;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->senddata.data = rtc_bk_malloc(size);
				if(pcmd->senddata.data!=NULL){
					rtc_bk_memcpy(pcmd->senddata.data,(void *)dataptr,size);
					pcmd->senddata.len = size;
					pcmd->senddata.size = size;
					pcmd->senddata.flags = flags;
					rtc_bk_memcpy(pcmd->senddata.toaddr,(void *)to,tolen);
					pcmd->senddata.tolen = tolen;
					int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_SEND,(u32)pcmd,(u32)psocket);
					if(res == BK_OK){
						rtos_lock_mutex(&pcmd->mutex);
						if(pcmd->responseed == 0){
							 pcmd->isWaited = 1;
							 rtos_unlock_mutex(&pcmd->mutex);
						 	 rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
						}else{
							rtos_unlock_mutex(&pcmd->mutex);
						}								
						result =  pcmd->result.result;						
					}else{
						result =  0;
					}
				}else{
					result =  0;
				}
			      destory_mailbox_send_cmd_data(pcmd);
			}
			
			
		  }

		}
	}
	return result;


}
ssize_t write(int s, const void *dataptr, size_t size){
	//LOGW("%s %d\n", __func__, __LINE__);
	int result = -1;
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
		  if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){
				//LOGW("sendto  %d",s);
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
				pcmd->result.result = 0;
				pcmd->responseed = 0;
				pcmd->txstate = 0;
				pcmd->param.param1 = s;
				pcmd->senddata.data = rtc_bk_malloc(size);
				rtc_bk_memcpy(pcmd->senddata.data,(void *)dataptr,size);
				pcmd->senddata.len = size;
				pcmd->senddata.len = size;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_SEND,(u32)pcmd,(u32)psocket);
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
				  	result = 0;
				}

				destory_mailbox_send_cmd_data(pcmd);
				
			}
			
			
		  }

		}
	}
	return result;

}
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                struct timeval *timeout){
	
	int s = maxfdp1-1;	
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL  ){
			if(psocket->last_error<0){
				LOGE("%s %d socket = %d error = %d\n", __func__, __LINE__,s,psocket->last_error);
				return psocket->last_error;
			}
			bool is_empty = true;
			rtos_lock_mutex(&psocket->socket_mutex);
				if(psocket->recv_data_list ==NULL){
					is_empty = true;
				}else{
					is_empty = false;
				}
			rtos_unlock_mutex(&psocket->socket_mutex);
			if(is_empty && writeset == NULL){
				int timeoutms = timeout->tv_sec*1000+timeout->tv_usec/1000;
				psocket->select_waited = true;
				rtos_get_semaphore(&psocket->socket_select_sem, timeoutms);
				psocket->select_waited = false;
				if(psocket->last_error<0){
				   return psocket->last_error;
				}
				if(psocket->recv_data_list ==NULL){
				   return 0;
				}else{
				   return 1;
				}
			}else if(is_empty && writeset != NULL){
				int timeoutms = 1;
				FD_SET(s, writeset);
				if(readset!= NULL){
				   FD_CLR(s, readset);
				}
				psocket->select_waited = true;
				rtos_get_semaphore(&psocket->socket_select_sem, timeoutms);
				psocket->select_waited = false;
				if(psocket->last_error<0){
				   return psocket->last_error;
				}
				return 1;
			}else{
			   if(writeset!= NULL){
				 FD_CLR(s, writeset);
			    }
			   if(readset!= NULL){
				FD_SET(s, readset);
			    }
			   return 1;
			}

		}else{
		   //LOGE("%s %d \n", __func__, __LINE__);
		   return -1;
		}
	}else{
	   //LOGE("%s %d \n", __func__, __LINE__);
	   return -1;
	}
	return 0;
}

#if LWIP_SOCKET_POLL
int poll(struct pollfd *fds, nfds_t nfds, int timeout){
	//LOGW("%s %d\n", __func__, __LINE__);
}
#endif
int ioctl(int s, long cmd, void *argp){

	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
			 int result = 0;
			 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
					webrtc_cmd_t webrtc_cmd = {0};
					webrtc_cmd_t *pcmd = &webrtc_cmd;
					if(pcmd!= NULL){
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
						pcmd->param.param1 = s;
						pcmd->param.lparam = cmd;
						pcmd->param.param2 = (u32)argp;

						int res =  project_config->mailbox_send_media_req_msg(SOCKET_COM_IOCTRL,(u32)pcmd,(u32)psocket);
						if(res == BK_OK){
						   rtos_lock_mutex(&pcmd->mutex);
						   if(pcmd->responseed == 0){
							 pcmd->isWaited = 1;
							 rtos_unlock_mutex(&pcmd->mutex);
						   	 rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
						   }else{
						  	 rtos_unlock_mutex(&pcmd->mutex);
						    }
						}
						result = pcmd->result.result;
						destory_mailbox_send_cmd_data(pcmd);
						return result;				
					}
			
			
			  }
	  }
	}
	return 1;
}
#if 1
int socket_fcntl(int s, int cmd, int val){
	if(s>=0 && s<MAX_SOCKETS){
		sockets_impl_t *psocket = gsockets[s].socket;
		if(psocket!= NULL){
			 int result = 0;
			 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
					webrtc_cmd_t webrtc_cmd = {0};
					webrtc_cmd_t *pcmd = &webrtc_cmd;
					if(pcmd!= NULL){
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
						pcmd->param.param1 = s;
						pcmd->param.param2 = cmd;
						pcmd->param.param3 = val;

						int res =  project_config->mailbox_send_media_req_msg(SOCKET_COM_FCNTL,(u32)pcmd,(u32)psocket);
						if(res == BK_OK){
						   rtos_lock_mutex(&pcmd->mutex);
						   if(pcmd->responseed == 0){
							 pcmd->isWaited = 1;
							 rtos_unlock_mutex(&pcmd->mutex);
						   	 rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
						   }else{
						  	 rtos_unlock_mutex(&pcmd->mutex);
						    }
						}
						result = pcmd->result.result;
						destory_mailbox_send_cmd_data(pcmd);
						return result;				
					}
			
			
			  }
	  }
	}
	return 0;
}
#endif
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size){
	LOGW("%s %d\n", __func__, __LINE__);
	return NULL;
}
int inet_pton(int af, const char *src, void *dst){
	LOGW("%s %d\n", __func__, __LINE__);
	return 0;
}


struct socket_ip4_addr {
  u32_t addr;
};
#include <ctype.h>
#define socket_isdigit(c)           isdigit((unsigned char)(c))
#define socket_isxdigit(c)          isxdigit((unsigned char)(c))
#define socket_islower(c)           islower((unsigned char)(c))
#define socket_isspace(c)           isspace((unsigned char)(c))
#define socket_isupper(c)           isupper((unsigned char)(c))
#define socket_tolower(c)           tolower((unsigned char)(c))
#define socket_toupper(c)           toupper((unsigned char)(c))
#define socket_ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
#define socket_ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)
#define SOCKET_IPADDR_NONE         ((u32_t)0xffffffffUL)
int socket_ip4addr_aton(const char *cp, struct socket_ip4_addr *addr)
{
	  u32_t val;
	  u8_t base;
	  char c;
	  u32_t parts[4];
	  u32_t *pp = parts;

	  c = *cp;
	  for (;;) {
	    /*
	     * Collect number up to ``.''.
	     * Values are specified as for C:
	     * 0x=hex, 0=octal, 1-9=decimal.
	     */
	    if (!socket_isdigit(c)) {
	      return 0;
	    }
	    val = 0;
	    base = 10;
	    if (c == '0') {
	      c = *++cp;
	      if (c == 'x' || c == 'X') {
		base = 16;
		c = *++cp;
	      } else {
		base = 8;
	      }
	    }
	    for (;;) {
	      if (socket_isdigit(c)) {
		val = (val * base) + (u32_t)(c - '0');
		c = *++cp;
	      } else if (base == 16 && socket_isxdigit(c)) {
		val = (val << 4) | (u32_t)(c + 10 - (socket_islower(c) ? 'a' : 'A'));
		c = *++cp;
	      } else {
		break;
	      }
	    }
	    if (c == '.') {
	      /*
	       * Internet format:
	       *  a.b.c.d
	       *  a.b.c   (with c treated as 16 bits)
	       *  a.b (with b treated as 24 bits)
	       */
	      if (pp >= parts + 3) {
		return 0;
	      }
	      *pp++ = val;
	      c = *++cp;
	    } else {
	      break;
	    }
	  }
	  /*
	   * Check for trailing characters.
	   */
	  if (c != '\0' && !socket_isspace(c)) {
	    return 0;
	  }
	  /*
	   * Concoct the address according to
	   * the number of parts specified.
	   */
	  switch (pp - parts + 1) {

	    case 0:
	      return 0;       /* initial nondigit */

	    case 1:             /* a -- 32 bits */
	      break;

	    case 2:             /* a.b -- 8.24 bits */
	      if (val > 0xffffffUL) {
		return 0;
	      }
	      if (parts[0] > 0xff) {
		return 0;
	      }
	      val |= parts[0] << 24;
	      break;

	    case 3:             /* a.b.c -- 8.8.16 bits */
	      if (val > 0xffff) {
		return 0;
	      }
	      if ((parts[0] > 0xff) || (parts[1] > 0xff)) {
		return 0;
	      }
	      val |= (parts[0] << 24) | (parts[1] << 16);
	      break;

	    case 4:             /* a.b.c.d -- 8.8.8.8 bits */
	      if (val > 0xff) {
		return 0;
	      }
	      if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff)) {
		return 0;
	      }
	      val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
	      break;
	    default:
		LOGE("%s %d  unhandled\n", __func__, __LINE__);

	      break;
	  }
	  if (addr) {
	    socket_ip4_addr_set_u32(addr, htonl(val));
	  }
	  return 1;
}
char *socket_ip4addr_ntoa_r(const struct socket_ip4_addr *addr, char *buf, int buflen)
{
	  u32_t s_addr;
	  char inv[3];
	  char *rp;
	  u8_t *ap;
	  u8_t rem;
	  u8_t n;
	  u8_t i;
	  int len = 0;

	  s_addr = socket_ip4_addr_get_u32(addr);

	  rp = buf;
	  ap = (u8_t *)&s_addr;
	  for (n = 0; n < 4; n++) {
	    i = 0;
	    do {
	      rem = *ap % (u8_t)10;
	      *ap /= (u8_t)10;
	      inv[i++] = (char)('0' + rem);
	    } while (*ap);
	    while (i--) {
	      if (len++ >= buflen) {
		return NULL;
	      }
	      *rp++ = inv[i];
	    }
	    if (len++ >= buflen) {
	      return NULL;
	    }
	    *rp++ = '.';
	    ap++;
	  }
	  *--rp = 0;
	  return buf;
}
u32_t socket_ipaddr_addr(const char *cp)
{
  struct socket_ip4_addr val;

  if (socket_ip4addr_aton(cp, &val)) {
    return socket_ip4_addr_get_u32(&val);
  }
  return (SOCKET_IPADDR_NONE);
}

u32_t inet_addr(const char *cp){
	//LOGW("%s %d\n", __func__, __LINE__);
	return socket_ipaddr_addr(cp);

}
int inet_aton(const char *string, struct in_addr *addr){
     //LOGW("%s %d\n", __func__, __LINE__);

     return socket_ip4addr_aton(string,(struct socket_ip4_addr *)addr);
}
char * inet_ntoa(struct in_addr in){
     //LOGW("%s %d\n", __func__, __LINE__);
     static char szaddr[64] = {0};
     memset(szaddr,0,sizeof(szaddr));
     socket_ip4addr_ntoa_r((const struct socket_ip4_addr *)&in,szaddr,sizeof(szaddr));

     return szaddr;
}
int getaddrinfo(const char *nodename,
       const char *servname,
       const struct addrinfo *hints,
       struct addrinfo **res){
	//LOGW("%s %d\n", __func__, __LINE__);
	 int result = 0;
	 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL && nodename!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd!= NULL){
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
				snprintf((char*)pcmd->param.szparam,127,"%s",nodename);
				if(servname!= NULL){
				  snprintf((char*)pcmd->result.szresult,127,"%s",servname);
				  pcmd->result.resultlen = strlen(servname);
				}else{
				  pcmd->result.resultlen = 0;
				}
				pcmd->param.param1 = (u32)hints;
				pcmd->param.param2 = (u32)res;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_GETADDRINFO,(u32)pcmd,(u32)0);
				if(res == BK_OK){
					rtos_lock_mutex(&pcmd->mutex);
					if(pcmd->responseed == 0){
						pcmd->isWaited = 1;
						rtos_unlock_mutex(&pcmd->mutex);
				   		rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
					}else{
						rtos_unlock_mutex(&pcmd->mutex);
					}
				}
				result = (u32_t)pcmd->result.result;
				destory_mailbox_send_cmd_data(pcmd);
				return result;
				
				
			}
			
			
	  }

       return 0;

}
void freeaddrinfo(struct addrinfo *ai){
	 //LOGW("%s %d\n", __func__, __LINE__);
	 int result = 0;
	 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd!= NULL){
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
				pcmd->param.param1 = (u32)ai;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_FREEADDRINFO,(u32)pcmd,(u32)0);
				if(res == BK_OK){
				   rtos_lock_mutex(&pcmd->mutex);
				   if(pcmd->responseed == 0){
					pcmd->isWaited = 1;
					rtos_unlock_mutex(&pcmd->mutex);
				        rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
				   }else{
				   	rtos_unlock_mutex(&pcmd->mutex);
				   }
				}
				result = (u32_t)pcmd->result.result;
				destory_mailbox_send_cmd_data(pcmd);
				
				
			}
			
			
	  }

}
struct hostent *gethostbyname(const char *name){
	 //LOGW("%s %d\n", __func__, __LINE__);
	 int result = 0;
	 struct hostent *hptr = NULL;
	 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd!= NULL){
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
				pcmd->param.param2 = 0;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_GETHOSTBYNAME,(u32)pcmd,(u32)0);
				if(res == BK_OK){
				   rtos_lock_mutex(&pcmd->mutex);
				   if(pcmd->responseed == 0){
					pcmd->isWaited = 1;
					rtos_unlock_mutex(&pcmd->mutex);
				        rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
				   }else{
				   	rtos_unlock_mutex(&pcmd->mutex);
				   }
				}
				result = pcmd->result.result;
				if(result == 1){
				    hptr = (struct hostent *)pcmd->param.param2;
				}else{
					//LOGW("%s %d\n", __func__, __LINE__);
				}
				destory_mailbox_send_cmd_data(pcmd);
				
				
			}
			
			
	  }


	return hptr;
}
int getlocaladdress(char *addr,int len){
	 //LOGW("%s %d\n", __func__, __LINE__);
	 int result = 0;
	 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd!= NULL){
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
				pcmd->result.result =-1;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_GETLOCALADDR,(u32)pcmd,(u32)0);
				if(res == BK_OK){
				   rtos_lock_mutex(&pcmd->mutex);
				   if(pcmd->responseed == 0){
					pcmd->isWaited = 1;
					rtos_unlock_mutex(&pcmd->mutex);
				        rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
				   }else{
				   	rtos_unlock_mutex(&pcmd->mutex);
				   }
				}
				snprintf(addr,len,"%s",pcmd->result.szresult);
				result = (u32_t)pcmd->result.result;
				destory_mailbox_send_cmd_data(pcmd);
				
				
			}
			
			
	  }
	return result;

}
int get_netif_ip4_config(void *config){
	 int result = 0;
	 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd!= NULL){
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
				pcmd->param.param1 = (u32)config;
				pcmd->result.result =-1;
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_GET_NET_CONFIG,(u32)pcmd,(u32)0);
				if(res == BK_OK){
				   rtos_lock_mutex(&pcmd->mutex);
				   if(pcmd->responseed == 0){
					pcmd->isWaited = 1;
					rtos_unlock_mutex(&pcmd->mutex);
				        rtos_get_semaphore(&pcmd->sem, BEKEN_NEVER_TIMEOUT);
				   }else{
				   	rtos_unlock_mutex(&pcmd->mutex);
				   }
				}
				
				result = (u32_t)pcmd->result.result;
				destory_mailbox_send_cmd_data(pcmd);
				
				
			}
			
			
	  }
	return result;
}
