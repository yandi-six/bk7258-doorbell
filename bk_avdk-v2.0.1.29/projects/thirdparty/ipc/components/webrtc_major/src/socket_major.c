#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <components/log.h>
#include "components/netif.h"
#include <lwip/sockets.h>
#include <lwip/dns.h>
#include <lwip/tcpip.h>
#include <lwip/netif.h>
#include "lwip/netdb.h"
#include "socket_major.h"


#define TAG "major_socket"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)
extern void delay_us(UINT32 us);
static sockets_t gsockets[MAX_SOCKETS] = {0};
static beken_mutex_t  g_sockets_mutex = NULL;
static project_config_t *project_config = NULL;
static beken_thread_t globle_sockets_pid = NULL;
static beken_semaphore_t globle_sockets_sem = NULL;
static beken_semaphore_t globle_sockets_exit_sem = NULL;
static beken_mutex_t  globle_sockets_mutex = NULL;
static beken_queue_t globle_data_queue = NULL;
static bool globle_socket_runing = false;
void create_socket_thread(sockets_impl_t *psocket);
int socket_recv_data(sockets_impl_t *psocket,void *dataptr,int size,struct sockaddr *from, socklen_t fromlen);
int socket_send_recv_data(sockets_impl_t *psocket,void *dataptr,int size,struct sockaddr *from, socklen_t fromlen);
void handle_socket_req(webrtc_cmd_t *pcmd);
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
	
	if(globle_sockets_sem==NULL || globle_data_queue ==NULL){
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
	    rtos_init_mutex(&g_sockets_mutex);
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
							"glable_socket_major",
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
     if(g_sockets_mutex!= NULL){
        rtos_deinit_mutex(&g_sockets_mutex);
	g_sockets_mutex = NULL;
    }

    
    project_config = NULL;
}
static void destory_recv_data(recv_data_t *pdata){	   
	   if(pdata!= NULL){
		   if(pdata->data!= NULL){
			rtc_bk_free(pdata->data);
			pdata->data = NULL;
		    }
		   rtc_bk_free(pdata);
		   pdata = NULL;
	   }
}
static void destory_mailbox_cmd_data(webrtc_cmd_t *cmd)
{	   
	   if(cmd!= NULL){
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
static void destory_all_recvs_data(sockets_impl_t *psocket){
	bk_err_t ret = BK_OK;
	recv_data_t *pdata = NULL;
	queue_msg_t msg;
	
	while(rtos_is_queue_empty(&psocket->recv_data_queue) == false){
			ret = rtos_pop_from_queue(&psocket->recv_data_queue, &msg, 0);
			if (kNoErr == ret)
			{       
					pdata = (recv_data_t *)msg.param;
					if(pdata!= NULL){
						destory_recv_data(pdata);			
					}
			}
	}
	
}
int socket_tcp_send(int socketfd ,char *buf,int size){
	int m = 0;
	int sended = 0;
	char *sendbuf = (char *)buf;
	int sendsize = size;
        while(sendsize>0){
		sended = send(socketfd, (char *)sendbuf, sendsize, 0);
		if(sended<0){
			LOGW("webrtc streamer  socketfd %d send  error %d\n",socketfd,sended);
			return sended;
		}
		m+=sended;
		if(m== size){
			break;
		}else{
			 sendbuf+=sended;
			 sendsize-=sended;

		}

	}
	return m;
}

int send_socket_error(sockets_impl_t *psocket){
	 bk_err_t ret = BK_OK;
	 int result = 0;
	 //LOGE("%s %d \n", __func__, __LINE__);
	 if(project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL){
			//LOGE("%s %d \n", __func__, __LINE__);
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd!= NULL){
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				pcmd->result.result = 0;
				pcmd->responseed = 0;
				pcmd->txstate = 0;
				
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
					LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
					LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}
				pcmd->param.nparam1 = psocket->sock;
				pcmd->param.nparam2 = psocket->last_error;
				//LOGE("%s %d  socket =%d\n", __func__, __LINE__,psocket->sock);
				int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_ERROR,(u32)pcmd,(u32)psocket);
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
				destory_mailbox_cmd_data(pcmd);
				return result;
				
				
			}
			
			
	  }

	return 0;

}
int socket_send_recv_data(sockets_impl_t *psocket,void *dataptr,int size,struct sockaddr *from, socklen_t fromlen){
	//LOGW("%s %d\n", __func__, __LINE__);
	int result = 0;
	bk_err_t ret = BK_OK;
        if(psocket!= NULL && project_config!= NULL && project_config->mailbox_send_media_req_msg!= NULL && size>0 && dataptr!= NULL){
			webrtc_cmd_t webrtc_cmd = {0};
			webrtc_cmd_t *pcmd = &webrtc_cmd;
			if(pcmd){			
				rtc_bk_memset(pcmd,0,sizeof(webrtc_cmd_t));
				pcmd->result.result = 0;
				pcmd->responseed = 0;
				pcmd->txstate = 0;
				ret = rtos_init_semaphore_ex(&pcmd->sem, 1, 0);
				if (ret != BK_OK){
						LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
				}
				ret = rtos_init_mutex(&pcmd->mutex);
				if (ret != BK_OK){
						LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
				}	
				pcmd->param.nparam1 = psocket->sock;
				pcmd->recvdata.data = rtc_bk_malloc(size);
				if(pcmd->recvdata.data!= NULL){
					rtc_bk_memcpy(pcmd->recvdata.data,dataptr,size);
					pcmd->recvdata.len = size;
					pcmd->recvdata.size = size;
					if(fromlen>0 && fromlen<sizeof(pcmd->recvdata.fromaddr)){
						 rtc_bk_memcpy(pcmd->recvdata.fromaddr,(void *)from,fromlen);
						 pcmd->recvdata.fromlen = fromlen;
					}else{
						 pcmd->recvdata.fromlen = 0;
					}

					int res = project_config->mailbox_send_media_req_msg(SOCKET_COM_RECV,(u32)pcmd,(u32)psocket);
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
					
				
				}
				destory_mailbox_cmd_data(pcmd);
			}		
			
		  }


	return result;
}

void socket_recv_thread(void *param){
        sockets_impl_t *psocket = (sockets_impl_t *)param;
   	fd_set readfds;
	fd_set writefds;
   	int ret,maxfd=-1;
        struct timeval timeout;
        struct sockaddr remaddr;
        socklen_t addrlen = sizeof (remaddr);
	bool tosend = false;
        int recvbufsize = 1500;
	if(psocket->type == SOCK_STREAM){
		recvbufsize = 16*1024;
	}
	int sock = psocket->sock;

	//int size = 128*1024;
        //setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
	psocket->runing = true;
	uint8_t *recvbuf = (uint8_t *)rtc_bk_malloc(recvbufsize);
	while(psocket->runing){
		tosend = false;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds); 
		
		FD_SET(sock, &readfds);
		if(tosend){
		   FD_SET(sock,&writefds);
		}
		maxfd = sock; 
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		ret = select(maxfd+1,&readfds,tosend?&writefds:NULL,NULL,&timeout);
		if(0 > ret){
			LOGE("%s %d select error = %d\n", __func__, __LINE__,ret);
			psocket->last_error = ret;
			send_socket_error(psocket);
			break;
		}else if (ret == 0){
		}else {
		    if(FD_ISSET(sock,&readfds)){
			if(psocket->type == SOCK_DGRAM){
			      if(psocket->isconnected ==false){
			          ret = recvfrom(sock, recvbuf, recvbufsize, 0, &remaddr, &addrlen);
			      }else{
				 ret = read(sock, recvbuf, recvbufsize);
			      }
			}else{
			      if(psocket->islisten){
				 ret = 0;
			      }else{
			      	ret = recv(sock, recvbuf, recvbufsize, 0);
			      }
			      //LOGW("%s %d recv %d \n", __func__, __LINE__,ret);
			}
			if (ret < 0){
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
		
				}else{
					psocket->last_error = ret;
					send_socket_error(psocket);
					LOGE("%s %d  sock = %d error %d \n", __func__, __LINE__,sock,ret);
					break;
				}
			}else if(ret>0){
				//LOGW("%s %d recv %d \n", __func__, __LINE__,ret);
				socket_recv_data(psocket,recvbuf,ret,&remaddr,addrlen);
			}			
		    }
		   if (FD_ISSET(sock, &writefds)) {

		        
		    }
		}


	}
	psocket->runing = false;
	LOGI("%s %d socket = %d exit thread \n", __func__, __LINE__,sock);
	if(psocket->accept_pid){

		if(psocket->socket_accept_sem!= NULL){
			int count = rtos_get_semaphore_count(&psocket->socket_accept_sem);
			if(count == 0){
				rtos_set_semaphore(&psocket->socket_accept_sem);
			}
		}
		rtos_get_semaphore(&psocket->accept_socket_exit_sem, BEKEN_NEVER_TIMEOUT);
		

	}
	 
	if(psocket->socket_in_sem!= NULL){
		rtos_set_semaphore(&psocket->socket_in_sem);
	}  
	rtos_get_semaphore(&psocket->handlein_socket_exit_sem, BEKEN_NEVER_TIMEOUT);

	if(psocket->socket_out_sem!= NULL){
		rtos_set_semaphore(&psocket->socket_out_sem);
	}  
	rtos_get_semaphore(&psocket->handleout_socket_exit_sem, BEKEN_NEVER_TIMEOUT);

	if(recvbuf!= NULL){
		rtc_bk_free(recvbuf);
		recvbuf = NULL;
	}
	destory_all_recvs_data(psocket);
	if(psocket->in_mutex){
		rtos_deinit_mutex(&psocket->in_mutex);
	}
	if(psocket->out_mutex){
		rtos_deinit_mutex(&psocket->out_mutex);
	}
	if(psocket->socket_in_sem){
        	rtos_deinit_semaphore(&psocket->socket_in_sem);
	}
	if(psocket->socket_out_sem){
        	rtos_deinit_semaphore(&psocket->socket_out_sem);
	}
	if(psocket->socket_accept_sem){
        	rtos_deinit_semaphore(&psocket->socket_accept_sem);
	}
	if(psocket->handlein_socket_exit_sem){
		rtos_deinit_semaphore(&psocket->handlein_socket_exit_sem);
	}
	if(psocket->handleout_socket_exit_sem){
		rtos_deinit_semaphore(&psocket->handleout_socket_exit_sem);
	}
	if(psocket->accept_socket_exit_sem){
		rtos_deinit_semaphore(&psocket->accept_socket_exit_sem);
	}

	if (psocket->recv_data_queue)
	{
		rtos_deinit_queue(&psocket->recv_data_queue);
		psocket->recv_data_queue = NULL;
	}
	if (psocket->out_data_queue)
	{
		rtos_deinit_queue(&psocket->out_data_queue);
		psocket->out_data_queue = NULL;
	}
	rtos_lock_mutex(&g_sockets_mutex);
	gsockets[sock].socket = NULL;
        rtos_unlock_mutex(&g_sockets_mutex);
	close(sock);
	rtc_bk_free(psocket);		
	LOGW("%s %d socket = %d exit thread \n", __func__, __LINE__,sock);
	rtos_delete_thread(NULL);

}
void handin_data_handle(sockets_impl_t *psocket){
	bk_err_t ret = BK_OK;
	queue_msg_t msg;
	if(psocket->in_mutex){
		rtos_lock_mutex(&psocket->in_mutex);
		while(rtos_is_queue_empty(&psocket->recv_data_queue) == false){
				ret = rtos_pop_from_queue(&psocket->recv_data_queue, &msg, 0);
				if (kNoErr == ret){       
					recv_data_t *pdata = (recv_data_t *)msg.param;
					if(pdata!= NULL){
						//LOGW("%s %d -- %d \n", __func__, __LINE__,pdata->len);
						socket_send_recv_data(psocket,pdata->data,pdata->len,(struct sockaddr *)pdata->fromaddr,pdata->fromlen);
						destory_recv_data(pdata);
						//LOGW("%s %d  \n", __func__, __LINE__);			
					}
				}
		}
		rtos_unlock_mutex(&psocket->in_mutex);
	}
}

void socket_handin_thread(void *param){
	sockets_impl_t *psocket = (sockets_impl_t *)param;
	if(psocket ==NULL || psocket->socket_in_sem==NULL || psocket->recv_data_queue ==NULL){
		goto exit_0;
	}
	bk_err_t ret = BK_OK;
	while(psocket->runing){
		rtos_get_semaphore(&psocket->socket_in_sem, 10);
		handin_data_handle(psocket);
	}
	//LOGW("%s %d exit thread \n", __func__, __LINE__);
	if(psocket->handlein_socket_exit_sem!= NULL){
		rtos_set_semaphore(&psocket->handlein_socket_exit_sem);

	}
exit_0:
	psocket->handin_pid = NULL;
	rtos_delete_thread(NULL);
}
void handout_data_handle(sockets_impl_t *psocket){
	bk_err_t ret = BK_OK;
	queue_msg_t msg;
	if(psocket->out_mutex){
		rtos_lock_mutex(&psocket->out_mutex);
		while(rtos_is_queue_empty(&psocket->out_data_queue) == false){
				ret = rtos_pop_from_queue(&psocket->out_data_queue, &msg, 0);
				if (kNoErr == ret){       
					webrtc_cmd_t *pcmd = (webrtc_cmd_t *)msg.param;
					if(pcmd!= NULL){
						handle_socket_req(pcmd);			
					}
				}
		}
		rtos_unlock_mutex(&psocket->out_mutex);
	}
}

void socket_handout_thread(void *param){
	sockets_impl_t *psocket = (sockets_impl_t *)param;
	if(psocket ==NULL || psocket->socket_out_sem==NULL || psocket->out_data_queue ==NULL){
		goto exit_0;
	}
	bk_err_t ret = BK_OK;
	while(psocket->runing){
		rtos_get_semaphore(&psocket->socket_out_sem, 10);
		handout_data_handle(psocket);
	}
	//LOGW("%s %d exit thread \n", __func__, __LINE__);
	handout_data_handle(psocket);
	if(psocket->handleout_socket_exit_sem!= NULL){
		rtos_set_semaphore(&psocket->handleout_socket_exit_sem);

	}
exit_0:
	psocket->handout_pid = NULL;
	rtos_delete_thread(NULL);
}
int socket_recv_data(sockets_impl_t *psocket,void *dataptr,int size,struct sockaddr *from, socklen_t fromlen){
	bk_err_t ret = BK_OK;
	if(psocket->in_mutex){
		rtos_lock_mutex(&psocket->in_mutex);
		if(psocket!= NULL && size>0 && fromlen>0 && from!= NULL){
		 	recv_data_t *precv_data = (recv_data_t*)rtc_bk_malloc(sizeof(recv_data_t));
			if(precv_data!=NULL){
				rtc_bk_memset(precv_data,0,sizeof(recv_data_t));
				precv_data->data = (void*)rtc_bk_malloc(size+1);
				if(precv_data->data!= NULL){
						rtc_bk_memcpy(precv_data->data,dataptr,size);
						precv_data->len = size;
						if(fromlen<=sizeof(precv_data->fromaddr) && fromlen>0){
							rtc_bk_memcpy(precv_data->fromaddr,(void*)from,fromlen);
							precv_data->fromlen = fromlen;
						}else{
							precv_data->fromlen = 0;
						}
						queue_msg_t msg;
						msg.param = (void*)precv_data;
						//LOGW("%s %d -- %d \n", __func__, __LINE__,size);
						if (psocket->recv_data_queue!=NULL){
							//LOGW("%s %d -- %d \n", __func__, __LINE__,size);
							ret = rtos_push_to_queue(&psocket->recv_data_queue, &msg, BEKEN_NO_WAIT);
							if (BK_OK != ret)
							{
								LOGE("%s failed\n", __func__);
							}
						}
						if(psocket->socket_in_sem!= NULL){
							int count = rtos_get_semaphore_count(&psocket->socket_in_sem);
							if(count == 0){
								rtos_set_semaphore(&psocket->socket_in_sem);
							}

						}
				
				

				}else{
					rtc_bk_free(precv_data);
					precv_data = NULL;
				}
			}
		}
		rtos_unlock_mutex(&psocket->in_mutex);
	}
	return 0;

}

void create_socket_thread(sockets_impl_t *psocket){
                bk_err_t ret = BK_OK;
		ret = rtos_create_psram_thread(&psocket->socket_pid,
						5,
						"recv_socket",
						(beken_thread_function_t)socket_recv_thread,
					        8*1024,
						psocket);
		if (ret != kNoErr) {
			LOGE("create socket task fail \r\n");
					
		}

}
void create_handle_socket_thread(sockets_impl_t *psocket){
		bk_err_t ret = BK_OK;
		ret = rtos_create_psram_thread(&psocket->handout_pid,
						5,
						"handout_socket",
						(beken_thread_function_t)socket_handout_thread,
					        8*1024,
						psocket);
		if (ret != kNoErr) {
			LOGE("create socket task fail \r\n");
					
		}
		ret = rtos_create_psram_thread(&psocket->handin_pid,
						5,
						"handin_socket",
						(beken_thread_function_t)socket_handin_thread,
					        8*1024,
						psocket);
		if (ret != kNoErr) {
			LOGE("create socket task fail \r\n");
					
		}
}
int create_accept_socket(int sock){
        
	int result = 0;
	if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket!= NULL){
					LOGE("%s %d socket -------  error ---------%d\n", __func__, __LINE__,sock);
					close(sock);
					sockets_impl_t *ptemp = gsockets[sock].socket;
					ptemp->runing = false;
					close(ptemp->sock);
					gsockets[sock].socket = NULL;
					

				}else{
					LOGD("%s %d socket %d\n", __func__, __LINE__,sock);
					if(gsockets[sock].socket == NULL){
						sockets_impl_t *psocket = (sockets_impl_t *)rtc_bk_malloc(sizeof(sockets_impl_t));
						if(psocket){
							rtc_bk_memset(psocket,0,sizeof(sockets_impl_t));
							bk_err_t ret = BK_OK;
							ret = rtos_init_mutex(&psocket->in_mutex);
							if (ret != BK_OK){
								LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
							}
							ret = rtos_init_mutex(&psocket->out_mutex);
							if (ret != BK_OK){
								LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->socket_in_sem, 1, 0);
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->socket_out_sem, 1, 0);
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->handlein_socket_exit_sem, 1, 0);	
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->handleout_socket_exit_sem, 1, 0);	
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->accept_socket_exit_sem, 1, 0);	
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}

							ret = rtos_init_semaphore_ex(&psocket->socket_accept_sem, 1, 0);	
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}			
					
							ret = rtos_init_queue(&psocket->recv_data_queue,"recv_data_queue",sizeof(queue_msg_t),128);
							if (ret != BK_OK){
								LOGE("create mailbox_msg_queue fail\n");
							}
							ret = rtos_init_queue(&psocket->out_data_queue,"out_data_queue",sizeof(queue_msg_t),128);
							if (ret != BK_OK){
								LOGE("create mailbox_msg_queue fail\n");
							}
							psocket->type = SOCK_STREAM;
							psocket->sock = sock;
							psocket->runing = true;
							psocket->accept_pid=NULL;
							psocket->socket_pid = NULL;
							psocket->handout_pid = NULL;
							psocket->send_data_list = NULL;
							gsockets[sock].socket = psocket;
							create_handle_socket_thread(psocket);
							if(psocket->type == SOCK_STREAM){
							if(psocket->socket_pid == NULL){
								create_socket_thread(psocket);
						        }	
							}
							result = 1;
						}else{
							LOGE("%s %d close socket %d\n", __func__, __LINE__,sock);
							close(sock);
						}
					}else{
						LOGE("%s %d close socket %d\n", __func__, __LINE__,sock);
						close(sock);
					}
				}
		}else{
				LOGE("%s %d close socket %d\n", __func__, __LINE__,sock);
				close(sock);
		}
	
	return result;
}
void socket_accept_thread(void *param){
	webrtc_cmd_t *pcmd = (webrtc_cmd_t *)param;
	if(pcmd ==NULL){
		goto exit_0;
	}
	int sock;
	int accept_sock;
	sockets_impl_t *psocket = NULL;
	struct sockaddr addr;
	socklen_t len = sizeof(struct sockaddr);
	 sock = pcmd->param.param1;
	if(sock>=0 && sock<MAX_SOCKETS){
			if(gsockets[sock].socket != NULL){
				psocket = gsockets[sock].socket;

			}else{
				goto exit_0;
			}
	}else{
		goto exit_0;
	}
	while(psocket->runing){
		
		accept_sock = accept(sock, (struct sockaddr*)&addr, &len);
		//LOGW("%s %d  sa_family = %d len = %d     %d\n", __func__, __LINE__,addr.sa_family,len,sizeof(struct sockaddr));
		if(accept_sock>=0){
			LOGD("%s %d  accept  socket =%d\n", __func__, __LINE__,accept_sock);
			webrtc_cmd_t *acceptcmd = psocket->accpet_cmd;
			if(acceptcmd!=NULL){
				create_accept_socket(accept_sock);
				acceptcmd->result.result = accept_sock;
				LOGD("%s %d *********  %p    %d\n", __func__, __LINE__,acceptcmd,len);
				rtc_bk_memcpy(acceptcmd->result.szresult,(void*)&addr,sizeof(struct sockaddr));
				acceptcmd->result.resultlen = sizeof(struct sockaddr);
			
				if(project_config && project_config->mailbox_send_media_response_msg!= NULL){
					 	project_config->mailbox_send_media_response_msg(acceptcmd->mb_cmd.param1,acceptcmd->mb_cmd.param2,acceptcmd->mb_cmd.param3);
				}
				LOGD("%s %d <<<<<\n", __func__, __LINE__);
			}
			psocket->accpet_cmd = NULL;
			if(psocket->socket_accept_sem){
		            rtos_get_semaphore(&psocket->socket_accept_sem, BEKEN_NEVER_TIMEOUT);
			}
			LOGD("%s %d >>>>>>>\n", __func__, __LINE__);

	 	}else{
			webrtc_cmd_t *acceptcmd = psocket->accpet_cmd;
			if(acceptcmd!= NULL){
			acceptcmd->result.result = accept_sock;
			if(project_config && project_config->mailbox_send_media_response_msg!= NULL){
				 	project_config->mailbox_send_media_response_msg(acceptcmd->mb_cmd.param1,acceptcmd->mb_cmd.param2,acceptcmd->mb_cmd.param3);
			}
			}
			break;

		}
	}

	if(psocket->accept_socket_exit_sem!= NULL){
		rtos_set_semaphore(&psocket->accept_socket_exit_sem);

	}
exit_0:
	LOGD("%s %d socket %d exit thread\n", __func__, __LINE__,sock);
	psocket->accept_pid = NULL;
	rtos_delete_thread(NULL);
}
void create_accept_socket_thread(sockets_impl_t *psocket,webrtc_cmd_t *pcmd){
		bk_err_t ret = BK_OK;
		ret = rtos_create_psram_thread(&psocket->accept_pid,
						5,
						"accept_socket",
						(beken_thread_function_t)socket_accept_thread,
					        8*1024,
						pcmd);
		if (ret != kNoErr) {
			LOGE("create socket task fail \r\n");
					
		}
}
bool handle_socket_accept(webrtc_cmd_t *pcmd){
	rtos_lock_mutex(&g_sockets_mutex);
	bool result = false;
	if(pcmd!= NULL){
			pcmd->result.result = -1;
			int sock = pcmd->param.param1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket->runing){
						if(psocket->accept_pid == NULL){
						    LOGD("%s %d ======= %d  %p\n", __func__, __LINE__,sock,pcmd);
						    psocket->accpet_cmd = pcmd;
						    result = true;
						    create_accept_socket_thread(psocket,pcmd);
						    

						}else{
						    LOGD("%s %d ------- %d  %p\n", __func__, __LINE__,sock,pcmd);
						    psocket->accpet_cmd = pcmd;	
						    result = true;				    
						    if(psocket->socket_accept_sem!= NULL){
							int count = rtos_get_semaphore_count(&psocket->socket_accept_sem);
							if(count == 0){
								rtos_set_semaphore(&psocket->socket_accept_sem);
							}else{
								 LOGD("%s %d ******* %d  %d\n", __func__, __LINE__,sock,count);
							}
						    }
						
						}
					}
				}
			}
	}
	rtos_unlock_mutex(&g_sockets_mutex);	
	return result;
}
int create_socket(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	rtos_lock_mutex(&g_sockets_mutex);
	
	int sock = -1;
	int domain = 0;
	int type = 0;
	int protocol = 0;
	if(pcmd!= NULL){
			domain = pcmd->param.param1;
			type = pcmd->param.param2;
			protocol = pcmd->param.param3;	
			sock = socket(domain,type,protocol);
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket!= NULL){
					LOGE("%s %d socket -------  error ---------%d\n", __func__, __LINE__,sock);
					close(sock);
					sockets_impl_t *ptemp = gsockets[sock].socket;
					ptemp->runing = false;
					close(ptemp->sock);
					gsockets[sock].socket = NULL;
					pcmd->result.result = -1;

				}else{
					LOGD("%s %d socket %d\n", __func__, __LINE__,sock);
					if(gsockets[sock].socket == NULL){
						sockets_impl_t *psocket = (sockets_impl_t *)rtc_bk_malloc(sizeof(sockets_impl_t));
						if(psocket){
							rtc_bk_memset(psocket,0,sizeof(sockets_impl_t));
							bk_err_t ret = BK_OK;
							ret = rtos_init_mutex(&psocket->in_mutex);
							if (ret != BK_OK){
								LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
							}
							ret = rtos_init_mutex(&psocket->out_mutex);
							if (ret != BK_OK){
								LOGE("%s %d  create mutex fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->socket_in_sem, 1, 0);
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->socket_out_sem, 1, 0);
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->handlein_socket_exit_sem, 1, 0);	
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							ret = rtos_init_semaphore_ex(&psocket->handleout_socket_exit_sem, 1, 0);	
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							
							ret = rtos_init_semaphore_ex(&psocket->accept_socket_exit_sem, 1, 0);	
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}		
							ret = rtos_init_semaphore_ex(&psocket->socket_accept_sem, 1, 0);	
							if (ret != BK_OK){
								LOGE("%s %d  create semaphore fail\n", __func__, __LINE__);
							}
							ret = rtos_init_queue(&psocket->recv_data_queue,"recv_data_queue",sizeof(queue_msg_t),128);
							if (ret != BK_OK){
								LOGE("create mailbox_msg_queue fail\n");
							}
							ret = rtos_init_queue(&psocket->out_data_queue,"out_data_queue",sizeof(queue_msg_t),128);
							if (ret != BK_OK){
								LOGE("create mailbox_msg_queue fail\n");
							}
							psocket->domain = domain;
							psocket->type = type;
							psocket->protocol = protocol;
							psocket->sock = sock;
							psocket->accept_pid=NULL;
							psocket->accpet_cmd=NULL;
							psocket->runing = true;
							psocket->socket_pid = NULL;
							psocket->handout_pid = NULL;
							psocket->send_data_list = NULL;
							gsockets[sock].socket = psocket;
							pcmd->result.result = sock;
							create_handle_socket_thread(psocket);
							if(psocket->type == SOCK_STREAM){
							if(psocket->socket_pid == NULL){
								create_socket_thread(psocket);
						        }	
							}
						}else{
							LOGE("%s %d close socket %d\n", __func__, __LINE__,sock);
							close(sock);
						}
					}else{
						LOGE("%s %d close socket %d\n", __func__, __LINE__,sock);
						close(sock);
					}
				}
			}else{
				LOGE("%s %d close socket %d\n", __func__, __LINE__,sock);
				close(sock);
			}
	}else{
	}
	rtos_unlock_mutex(&g_sockets_mutex);
	return sock;
}
int create_socket_response(webrtc_cmd_t *pcmd){

	return create_socket(pcmd);

}
int close_socket_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	rtos_lock_mutex(&g_sockets_mutex);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			LOGI("%s %d sock = %d \n", __func__, __LINE__,sock);
			if(sock>=0 && sock<MAX_SOCKETS){
				close(sock);
				if(gsockets[sock].socket != NULL){
					gsockets[sock].socket->runing = false;
					pcmd->result.result = 1;
				}else{
					pcmd->result.result = -1;
				}
			}else{
				pcmd->result.result = -1;
			}
	}else{
			pcmd->result.result = -1;
	}
	rtos_unlock_mutex(&g_sockets_mutex);
	//LOGE("%s %d \n", __func__, __LINE__);
	return -1;
}
int bind_socket_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	int res = 0;
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;	
				
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					   sockets_impl_t *psocket = gsockets[sock].socket;					 
					   const struct sockaddr *localaddr =  (const struct sockaddr *)pcmd->param.param2;
					   socklen_t len = (socklen_t)pcmd->param.param3;
					   struct sockaddr_in *inadd = (struct sockaddr_in *)localaddr;
					   //LOGE("%s %d sock = %d  port = %d\n", __func__, __LINE__,sock,htons(inadd->sin_port));
					   res = bind(psocket->sock, localaddr, len);
					   if (res< 0) {
							pcmd->result.result = res;
					   }else{
						  if(psocket->socket_pid == NULL){
								create_socket_thread(psocket);
						  }
						  pcmd->result.result = res;

					    }
					
			
				}else{
					pcmd->result.result = -1;

				}
			}else{
				pcmd->result.result = -1;

			}

	}else{
			pcmd->result.result = -1;
			
	}

	return -1;
}
int send_socket_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			//LOGW("send_socket_response  %d\n",sock);
			pcmd->result.result = -1;
			rtos_lock_mutex(&g_sockets_mutex);
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					   
					   sockets_impl_t *psocket = gsockets[sock].socket;
					   if(psocket!= NULL && psocket->last_error>=0){
						rtos_unlock_mutex(&g_sockets_mutex);

						//LOGE("%s %d  send %d\n", __func__, __LINE__,pcmd->senddata.len);
						if(psocket->type == SOCK_DGRAM){
							if(psocket->isconnected == false){
								int n = sendto(sock, pcmd->senddata.data, pcmd->senddata.len, pcmd->senddata.flags, (struct sockaddr *)pcmd->senddata.toaddr, pcmd->senddata.tolen);
								pcmd->result.result = n;
								if (n < 0) {
								     //psocket->last_error = n;
								     //LOGW("sendto() failed error = %d",n);
								     						    
								}
							}else{
								int n = write(sock, pcmd->senddata.data, pcmd->senddata.len);
								pcmd->result.result = n;
								if (n < 0) {
								     //psocket->last_error = n;
								     //LOGW("sendto() failed error = %d",n);
								     						    
								}

							}
						}else if(psocket->type == SOCK_STREAM){					     
							int n = socket_tcp_send(sock, pcmd->senddata.data,pcmd->senddata.len);
							pcmd->result.result = n;
							if (n < 0) {
							     //LOGW("send() failed error = %d",n);
							     //psocket->last_error = n;
							}
						     
						}


					  }else{
						rtos_unlock_mutex(&g_sockets_mutex);
						//LOGE("%s %d error %d \n", __func__, __LINE__,psocket->last_error);
					  }
				}else{
					rtos_unlock_mutex(&g_sockets_mutex);
					LOGE("%s %d error \n", __func__, __LINE__);
				}
			}else{
				rtos_unlock_mutex(&g_sockets_mutex);
			}
	}
		

	return 0;
	
}
int send_inet_addr_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
		    const char *addr = (const char *)pcmd->param.szparam;
		    u32_t uaddr = inet_addr(addr);
		    pcmd->result.result = uaddr;
	}else{
	}
	return 0;
}
int socket_recv_response(webrtc_cmd_t *pcmd){
	return 0;
}
int socket_ioctrol_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						long cmd = pcmd->param.lparam;
						void *argp = (void *)pcmd->param.param2;
						int result = ioctl(sock,cmd,argp);
						pcmd->result.result = result;
					}else{

					}
				}else{

				}
			}else{

			}
	}else{

	}

	return 0;
}
int socket_fcntl_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						int cmd = (int)pcmd->param.param2;
						int val = (int)pcmd->param.param3;
						int result = fcntl(sock,cmd,val);
						pcmd->result.result = result;
					}else{

					}
				}else{

				}
			}else{

			}
	}else{

	}

	return 0;
}
int socket_getpeername_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						struct sockaddr sa;
						socklen_t len = sizeof(struct sockaddr);
						int result = getpeername(sock,(struct sockaddr *)&sa,&len);
						pcmd->result.result = result;
						rtc_bk_memcpy((void*)pcmd->result.szresult,(void*)&sa,len);
						pcmd->result.resultlen = len;
					}else{

					}
				}else{

				}
			}else{

			}
	}else{

	}

	return 0;
}
int socket_getsockname_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						struct sockaddr sa;
						socklen_t len = sizeof(struct sockaddr);
						int result = getsockname(sock,(struct sockaddr *)&sa,&len);
						pcmd->result.result = result;
						rtc_bk_memcpy((void*)pcmd->result.szresult,(void*)&sa,len);
						pcmd->result.resultlen = len;
						//LOGE("%s %d sa_family = %d\n", __func__, __LINE__,sa.sa_family);
					}else{

					}
				}else{

				}
			}else{

			}
	}else{

	}

	return 0;
}
int socket_getsockopt_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						int level = pcmd->param.param2;
                                                int optname = pcmd->param.param3;
						socklen_t optlen = pcmd->param.param4;
						int result = getsockopt(sock,level,optname,(void *)pcmd->result.szresult,&optlen);
						pcmd->result.result = result;
						pcmd->result.resultlen = optlen;

					}else{

					}
				}else{

				}
			}else{

			}
	}else{

	}

	return 0;
}
int socket_setsockopt_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						int level = pcmd->param.param2;
                                                int optname = pcmd->param.param3;
						socklen_t optlen = pcmd->param.param4;
						int result = setsockopt(sock,level,optname,pcmd->param.szparam,optlen);
						pcmd->result.result = result;
					}else{
					}
				}else{
				}
			}else{
			}
	}else{
	}
	return 0;
}
int socket_shutdown_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
		if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						int how = pcmd->param.param2;
						int result = shutdown(sock,how);
						pcmd->result.result = result;
					}else{
					}
				}else{
				}
			}else{
			}
		}else{
		}

	return 0;
}
int socket_getaddrinfo_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			struct addrinfo *hints = (struct addrinfo *)pcmd->param.param1;
			struct addrinfo **res = (struct addrinfo **)pcmd->param.param2;
			const char *nodename = (const char *)pcmd->param.szparam;
			const char *servname = NULL;
			if(pcmd->result.resultlen>0){
				servname = (const char *)pcmd->result.szresult;
			}
			pcmd->result.result = -1;
			int result = getaddrinfo(nodename,servname,hints,res);
			pcmd->result.result = result;
			
	}else{
	}
	return 0;
}
int socket_freeaddrinfo_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			struct addrinfo *ai = (struct addrinfo *)pcmd->param.param1;
			if(ai!= NULL){
			    freeaddrinfo(ai);
			}
			pcmd->result.result = 1;
			
	}else{
	}
	return 0;

}
int socket_gethostbyname_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			pcmd->result.result = 0;
			char *host = (char *)pcmd->param.param1;
			if(host!= NULL){
				struct hostent *hptr = gethostbyname(host);
				if(hptr!= NULL){
					pcmd->result.result = 1;
					pcmd->param.param2 = (u32)hptr;
				}else{
					//LOGE("%s %d \n", __func__, __LINE__);
					pcmd->param.param2 = 0;
				}
			}
			
			
	}else{
	}
	return 0;

}
int socket_connect_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						const struct sockaddr *name = (const struct sockaddr *)pcmd->senddata.toaddr;
						socklen_t namelen = pcmd->senddata.tolen;
						int result = connect(sock,name,namelen);
						psocket->isconnected = true;
						pcmd->result.result = result;
						if(psocket->socket_pid == NULL){
							create_socket_thread(psocket);
						}
					}else{

					}
				}else{
				}
			}else{

			}
	}else{

	}
	return 0;
}
int socket_listen_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			//LOGE("%s %d sock = %d\n", __func__, __LINE__,sock);
			pcmd->result.result = -1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL){
						psocket->islisten = true;
						int backlog = pcmd->param.param2;
						int result = listen(sock,backlog);
						pcmd->result.result = result;
					}else{

					}
				}else{

				}
			}else{

			}
	}else{

	}
	return 0;
}
int socket_getlocaladdr_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			netif_ip4_config_t ip4_config;
			extern uint32_t uap_ip_is_start(void);

			os_memset(&ip4_config, 0x0, sizeof(netif_ip4_config_t));
			bk_netif_get_ip4_config(NETIF_IF_AP, &ip4_config);
			if (uap_ip_is_start()){
				bk_netif_get_ip4_config(NETIF_IF_AP, &ip4_config);
			}else{
				bk_netif_get_ip4_config(NETIF_IF_STA, &ip4_config);
			}
			snprintf((char*)pcmd->result.szresult,sizeof(pcmd->result.szresult),"%s",ip4_config.ip);
	
			pcmd->result.resultlen = strlen(((char*)pcmd->result.szresult));
			pcmd->result.result = 1;
			
	}else{
	}
	return 0;
}
int socket_netif_ip4_config_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
			netif_ip4_config_t *ip4_config = (netif_ip4_config_t *)pcmd->param.param1;
			extern uint32_t uap_ip_is_start(void);

			os_memset(ip4_config, 0x0, sizeof(netif_ip4_config_t));
			bk_netif_get_ip4_config(NETIF_IF_AP, ip4_config);
			if (uap_ip_is_start()){
				bk_netif_get_ip4_config(NETIF_IF_AP, ip4_config);
			}else{
				bk_netif_get_ip4_config(NETIF_IF_STA, ip4_config);
			}
			pcmd->result.result = 1;
			
	}else{
	}
	return 0;
}
int socket_inet_ntoa_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
		struct in_addr *in = (struct in_addr *)pcmd->param.szparam;
		char * addr = inet_ntoa(*in);
		snprintf((char*)pcmd->result.szresult,sizeof(pcmd->result.szresult),"%s",addr);
		pcmd->result.resultlen = strlen(((char*)pcmd->result.szresult));
		pcmd->result.result = 1;
			
	}else{
	}
	return 0;
}
int socket_inet_aton_response(webrtc_cmd_t *pcmd){
	//LOGE("%s %d \n", __func__, __LINE__);
	if(pcmd!= NULL){
		struct in_addr * addr_in = (struct in_addr  *)pcmd->result.szresult;
		char * addr = (char *)pcmd->param.szparam;
		int result = inet_aton(addr,addr_in);
		pcmd->result.result = result;
			
	}else{
	}

	return 0;
}

void socket_handle_globel_req(webrtc_cmd_t *pcmd){
	 //LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
	if(pcmd->mb_cmd.param1 == SOCKET_COM_CLOSE){
		close_socket_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_CREATE){
		create_socket_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_INETADDR){
		send_inet_addr_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETADDRINFO){
		socket_getaddrinfo_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_FREEADDRINFO){
		socket_freeaddrinfo_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETHOSTBYNAME){
		socket_gethostbyname_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETLOCALADDR){
		socket_getlocaladdr_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GET_NET_CONFIG){
		socket_netif_ip4_config_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_INET_NTOA){
		socket_inet_ntoa_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_INET_ATON){
		socket_inet_aton_response(pcmd);
	}
        if(project_config && project_config->mailbox_send_media_response_msg!= NULL){
		 	project_config->mailbox_send_media_response_msg(pcmd->mb_cmd.param1,pcmd->mb_cmd.param2,pcmd->mb_cmd.param3);
        }

}
void handle_socket_req(webrtc_cmd_t *pcmd){
	// LOGW("%s %d cmd = 0x%x   %d\n", __func__, __LINE__,pcmd->mb_cmd.param1,pcmd->param.param1);
	bool response = true;
	if(pcmd->mb_cmd.param1 == SOCKET_COM_BIND){
		bind_socket_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_SEND){
		send_socket_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_IOCTRL){
		socket_ioctrol_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_FCNTL){
		socket_fcntl_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETPEERNAME){
		socket_getpeername_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETSOCKNAME){
		socket_getsockname_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETSOCKOPT){
		socket_getsockopt_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_SETSOCKOPT){
		socket_setsockopt_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_SHUTDOWN){
		socket_shutdown_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_CONNECT){
		socket_connect_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_LISTEN){
		socket_listen_response(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_ACCEPT){
		response = false;
		handle_socket_accept(pcmd);
        }
        if(response == true && project_config && project_config->mailbox_send_media_response_msg!= NULL){
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
bool put_socket_req_cmd(webrtc_cmd_t *pcmd){		
	bk_err_t ret = BK_OK;
	bool result = false;
	if(pcmd!= NULL){
			int sock = pcmd->param.param1;
			if(sock>=0 && sock<MAX_SOCKETS){
				if(gsockets[sock].socket != NULL){
					sockets_impl_t *psocket = gsockets[sock].socket;
					if(psocket!= NULL && psocket->out_mutex!= NULL){
							rtos_lock_mutex(&psocket->out_mutex);
							queue_msg_t msg;
							msg.param = (void*)pcmd;
							//LOGW("%s %d  \n", __func__, __LINE__);
							if (psocket->out_data_queue!=NULL){
								//LOGW("%s %d  0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
							       ret = rtos_push_to_queue(&psocket->out_data_queue, &msg, BEKEN_NO_WAIT);
								if (BK_OK != ret)
								{
									LOGE("%s failed\n", __func__);
								}else{
									result = true;
								       if(psocket->socket_out_sem!= NULL){
									int count = rtos_get_semaphore_count(&psocket->socket_out_sem);
									if(count == 0){
										rtos_set_semaphore(&psocket->socket_out_sem);
									}
								       }
								}
							}
							rtos_unlock_mutex(&psocket->out_mutex);
					}else{

					}
				}else{
				}
			}else{

			}
	}else{

	}
	return result;
}
void socket_handle_req_rx(webrtc_cmd_t *pcmd){
	bool result = false;
	// LOGW("%s %d cmd = 0x%x \n", __func__, __LINE__,pcmd->mb_cmd.param1);
	if(pcmd->mb_cmd.param1 == SOCKET_COM_CREATE){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_CLOSE){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_BIND){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_SEND){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_CONNECT){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_LISTEN){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_IOCTRL){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_FCNTL){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETPEERNAME){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETSOCKNAME){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETSOCKOPT){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_SETSOCKOPT){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_SHUTDOWN){
		result = put_socket_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_INETADDR){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETADDRINFO){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_FREEADDRINFO){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETHOSTBYNAME){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GETLOCALADDR){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_GET_NET_CONFIG){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_INET_NTOA){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_INET_ATON){
		result = put_globle_sockets_req_cmd(pcmd);
	}else if(pcmd->mb_cmd.param1 == SOCKET_COM_ACCEPT){
		result = put_socket_req_cmd(pcmd);
	}

	if(result == false ){
		if(project_config && project_config->mailbox_send_media_response_msg!= NULL){
			 	project_config->mailbox_send_media_response_msg(pcmd->mb_cmd.param1,pcmd->mb_cmd.param2,pcmd->mb_cmd.param3);
		}
	}
	


}
void socket_handle_resp_rx(webrtc_cmd_t *pcmd){
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
