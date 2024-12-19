#ifndef RTC_BK_H_
#define RTC_BK_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
void* rtc_bk_malloc(size_t sz);
void* rtc_bk_realloc(void *ptr, size_t sz);
void  rtc_bk_free(void* ptr);
void* rtc_bk_memcpy(void *dest, void *source, size_t n);
void *rtc_bk_memset(void *ptr, int c, size_t n);
uint32_t bk_get_milliseconds(void);


#ifdef __cplusplus
}
#endif
#endif
