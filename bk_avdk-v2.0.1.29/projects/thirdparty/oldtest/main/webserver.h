#ifndef webserver__h
#define webserver__h
#include "mongoose.h"
int webrtc_webserver_start(char *rootpath,char *certpath,char *certkeypath,int https_port,int http_port);
int webrtc_webserver_stop();


#endif
