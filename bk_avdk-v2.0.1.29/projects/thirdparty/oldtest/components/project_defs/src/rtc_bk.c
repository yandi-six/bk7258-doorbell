
#include "rtc_bk.h"
#if CONFIG_AON_RTC
#include <driver/aon_rtc.h>
#endif
uint32_t bk_get_milliseconds(void)
{
	uint32_t time = 0;
#if CONFIG_ARCH_RISCV
	extern u64 riscv_get_mtimer(void);

	time = (riscv_get_mtimer() / 26000) & 0xFFFFFFFF;
#elif CONFIG_ARCH_CM33

#if CONFIG_AON_RTC
	time = (bk_aon_rtc_get_us() / 1000) & 0xFFFFFFFF;
#else
         time = (uint32_t)rtos_get_time();
#endif

#endif

	return time;
}
unsigned int rtc_bk_calc_align(unsigned int n, unsigned align){
     //return (n + (align - 1)) & (~(align - 1));
     return ( (n / align) +1) * align;
}
void* rtc_bk_malloc(size_t sz){	
#if CONFIG_PSRAM_AS_SYS_MEMORY
	//int alloc_size = (((int)sz)/4+1)*4;
	//if(alloc_size == 556){
	  // os_printf("rtc_bk_malloc psram_malloc mem %d\n",(int)sz);
	//}
	void * b = psram_zalloc(sz);//psram_malloc  psram_zalloc
#else
	void * b = os_malloc(sz);
#endif
	if(b == NULL){
		os_printf("Failed  webrtc_malloc  mem %d\n",(int)sz);
	}else{
		rtc_bk_memset(b,0,sz);
	}
	return b;

}
void* rtc_bk_memcpy(void *dest, void *source, size_t n)
{
	return os_memcpy(dest,source,n);
}
void* rtc_bk_realloc(void *ptr, size_t sz){
#if CONFIG_PSRAM_AS_SYS_MEMORY
	 return bk_psram_realloc(ptr,sz);
#else
	 return os_realloc(ptr,sz); 
#endif

}

void rtc_bk_free(void* ptr){

#if CONFIG_PSRAM_AS_SYS_MEMORY
        os_free(ptr);
	ptr = NULL;
#else
        os_free(ptr);
	ptr = NULL;
#endif
	
}
void *rtc_bk_memset(void *ptr, int c, size_t n){
	return os_memset(ptr,c,n);
}
