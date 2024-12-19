#ifndef __TIME_INTF_H__
#define __TIME_INTF_H__

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t mday;
    uint8_t month;
    uint16_t year;
    uint8_t wday;
} time_user_datetime_t;

int time_rtc_ntp_sync_init(int timezone_offset,char *timezone_ntp);
void time_rtc_ntp_sync_stop();
int time_timestr_get(char *buf,unsigned char len);
int time_datetime_get(time_user_datetime_t *pdt);
int time_datestr_get(char *buf,int len);
int time_timestr_get_ex(char *buf,int len);
int time_rtc_ntp_sync_update(int timezone_offset,char *timezone_ntp);
#endif
