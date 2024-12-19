
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <components/shell_task.h>
#include <components/event.h>
#include <components/log.h>
#include <driver/pwr_clk.h>
#include "mongoose.h"
#include "webserver.h"
#define TAG "www"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)

static struct mg_mgr *_mgr;

static  char s_http_addr[128] = {0};// = "http://0.0.0.0:8000";    // HTTP port
static  char s_https_addr[128]= {0};// = "https://0.0.0.0:8443";  // HTTPS port
static  char s_root_dir[128]= {0};// = "./www";
static  char s_certpath[128]= {0};// = "./www";
static  char s_certkeypath[128]= {0};// = "./www";
static bool  www_runing = false;
static beken_thread_t  web_pid = NULL;
void webrtc_www_ev_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data);

static void webserver_pthread_routine(void *tool_in){

	   //LOGW("webserver_pthread_routine start");
           
           www_runing = true;
	   struct mg_connection *c;
	   struct mg_mgr mgr;                            // Event manager
	   mg_log_set("3");                              // Set to 3 to enable debug
	   mg_mgr_init(&mgr);                            // Initialise event manager
	   _mgr = &mgr;
	   c = mg_http_listen(&mgr, s_http_addr, webrtc_www_ev_handler, NULL);  // Create HTTP listener
	   if(c == NULL){
		LOGW("webserver failed start %s  \n",s_http_addr);
           }
	   c = mg_http_listen(&mgr, s_https_addr, webrtc_www_ev_handler, (void *) 1);  // HTTPS listener
	   if(c == NULL){
		LOGW("webserver failed start %s  \n",s_https_addr);
           }

           while(www_runing){
               mg_mgr_poll(&mgr, 1000);
           }
           _mgr = NULL;
 
           mg_mgr_free(&mgr);
	   web_pid = NULL;
           rtos_delete_thread(NULL);


}

void webrtc_www_ev_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {  
   if (ev == MG_EV_ACCEPT && fn_data != NULL) {	    
	    LOGD("ev_handler ::MG_EV_ACCEPT https  %ld certpath = %s  certkeypath = %s\n",c->id,s_certpath,s_certkeypath);
	    struct mg_tls_opts opts = {
		.ca = NULL,       
		//.cert = "/etc/turn_server_cert.pem",     // Certificate PEM file
	       // .certkey = "/etc/turn_server_pkey.pem",  // This pem contains both cert and key
		//.cert = "/opt/cert.pem",     // Certificate PEM file
		//.certkey = "/opt/priv.key",  // This pem contains both cert and key
		.cert = s_certpath,     // Certificate PEM file
		.certkey = s_certkeypath,  // This pem contains both cert and key
	    };
	    mg_tls_init(c, &opts);
  } else if (ev == MG_EV_ACCEPT && fn_data == NULL) {
	LOGD("ev_handler ::MG_EV_ACCEPT http %ld----------------------------------------------\n",c->id);
   
  } else if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    
    if (mg_http_match_uri(hm, "/api/*")){

      
    } else if (mg_http_match_uri(hm, "/wswebclient/*")) {	
	char *pTemp;
	char szTemp[128]={0};
	char szTemp1[128]={0};
	pTemp = szTemp;
	snprintf(szTemp,sizeof(szTemp),"%.*s",(int)hm->uri.len,hm->uri.ptr);
	snprintf(szTemp1,sizeof(szTemp1),"%s","/wswebclient/");
	pTemp+=strlen(szTemp1);


	//snprintf(c->clientid,sizeof(c->clientid),"%s",pTemp);
	
	//LOGD("ev_handler ::MG_EV_HTTP_MSG wswebclient   %.*s  %s    %lu",(int)hm->uri.len,hm->uri.ptr,c->clientid,c->id);
        mg_ws_upgrade(c, hm, NULL);
    } else if (mg_http_match_uri(hm, "/upload")) {
/*
	      struct mg_http_part part;
	      size_t ofs = 0;
	      FILE *fp = NULL;
	      char fileName[128] = {0};
	      
	      while ((ofs = mg_http_next_multipart(hm->body, ofs, &part)) > 0) {
		//LOGD(("Chunk name: [%.*s] filename: [%.*s] length: %lu bytes",
		//         (int) part.name.len, part.name.ptr, (int) part.filename.len,
		//         part.filename.ptr, (unsigned long) part.body.len));
		memcpy(fileName, part.name.ptr, part.name.len);
		if(strstr(fileName, "file") != NULL) {
		  memset(fileName, 0, sizeof(fileName));
		  memcpy(fileName, part.filename.ptr, part.filename.len);
		  if((fp = fopen(fileName, "w+")) == NULL) {
		    printf("create file(%s) fail\n", fileName);
		  }
		  fwrite(part.body.ptr, part.body.len, 1, fp);
		}
	      }
	      if(fp != NULL) {
		fclose(fp);
	      }
*/
	      mg_http_reply(c, 200, "", "Thank you!");

    } else {
      LOGW("mg_recv  uri  %.*s\n",(int)hm->uri.len,hm->uri.ptr);
      struct mg_http_serve_opts opts = {.root_dir = s_root_dir};
       mg_http_serve_dir(c, ev_data, &opts);
    }
  }else if (ev == MG_EV_WS_MSG) {
   
    // Got websocket frame. Received data is wm->data. Echo it back!
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    //LOGW("%s:  websocket recv %.*s",__FILE__,(int )wm->data.len,wm->data.ptr);

   
    mg_iobuf_delete(&c->recv, c->recv.len);

  }else if (ev == MG_EV_CLOSE) {
	LOGW("ev_handler ::MG_EV_CLOSE %lu\n",c->id);

  }
  (void) fn_data;
}
int webrtc_webserver_start(char *rootpath,char *certpath,char *certkeypath,int https_port,int http_port)
{
      if(web_pid == NULL){
      if(rootpath == NULL){
	  return -1;
      }
      if(certpath == NULL){
	  return -1;
      }
      if(certkeypath == NULL){
	  return -1;
      }
      //LOGW("webrtc_streamer_webserver_start root path %s  https port = %d  http port = %d",rootpath,https_port,http_port);
      snprintf(s_root_dir,128,"%s",rootpath);
      snprintf(s_http_addr,128,"http://0.0.0.0:%d",http_port); 
      snprintf(s_https_addr,128,"https://0.0.0.0:%d",https_port);
      snprintf(s_certpath,128,"%s",certpath);
      snprintf(s_certkeypath,128,"%s",certkeypath);

      int ret;
      ret = rtos_create_psram_thread(&web_pid,
		                         5,
		                         "WWW",
		                         (beken_thread_function_t)webserver_pthread_routine,
		                         1024 * 16,
		                         (beken_thread_arg_t)NULL);
	if (ret != kNoErr){
		LOGE("create webserver_pthread_routine Thread failed ");
	}else{
		

	}
        LOGD("webrtc_streamer_webserver_start  end");
      }
      return 0;
}
int webrtc_webserver_stop()
{
    www_runing = false;

     return 0;
}
