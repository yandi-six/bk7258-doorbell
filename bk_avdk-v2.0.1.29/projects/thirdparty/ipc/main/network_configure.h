
#ifndef NETWORK_CONFIGURE_H
#define NETWORK_CONFIGURE_H

#ifdef __cplusplus
extern "C"
{
#endif
typedef void(*network_configure_callback)(char *ssid,char *password,char *key,char *name);
typedef void(*network_configure_ble_disconnect_callback)(int state);
void network_configure_start(char *serialNumber,int type,network_configure_callback callback,network_configure_ble_disconnect_callback disconnect_callback);
void network_configure_restart(void);
void network_configure_stop_advertising(void);
#ifdef __cplusplus
}
#endif

#endif

