
#ifndef _SOCKETS_H
#define _SOCKETS_H
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <sys/time.h>
#include <driver/mailbox_channel.h>
#include "project_defs.h"
#include "rtc_bk.h"
#include "rtc_list.h"
#ifdef __cplusplus
extern "C" {
#endif
#define MAX_SOCKETS 1024
typedef struct
{
	int sock;
	int domain; 
	int type; 
	int protocol;
	int runing;
	int last_error;
	bool isconnected;
	bool islisten;
	RTCList *send_data_list;
	beken_thread_t socket_pid;
	beken_thread_t handout_pid;
	beken_thread_t handin_pid;
	beken_thread_t accept_pid;
	beken_semaphore_t socket_in_sem;
	beken_semaphore_t socket_out_sem;
	beken_semaphore_t socket_accept_sem;
	beken_semaphore_t handlein_socket_exit_sem;
	beken_semaphore_t handleout_socket_exit_sem;
	beken_semaphore_t accept_socket_exit_sem;
	beken_mutex_t  in_mutex;
	beken_mutex_t  out_mutex;
	beken_queue_t recv_data_queue;
	beken_queue_t out_data_queue;
	webrtc_cmd_t *accpet_cmd;
}sockets_impl_t;
typedef struct
{
	sockets_impl_t* socket;
} sockets_t;

void init_sockets(project_config_t *config);
void uninit_sockets();
void socket_handle_req_rx(webrtc_cmd_t *pcmd);
void socket_handle_resp_rx(webrtc_cmd_t *pcmd);
#ifdef __cplusplus
}
#endif



#endif /* _SOCKETS_H */
