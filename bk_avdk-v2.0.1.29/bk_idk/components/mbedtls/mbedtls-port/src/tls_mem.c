#include "tls_config.h"

#ifdef MBEDTLS_PLATFORM_MEMORY
#include <os/mem.h>

void *tls_mbedtls_mem_calloc(size_t n, size_t size)
{
	unsigned int len = n * size;
	if(len == 0){
		return 0;
	}
#if CONFIG_PSRAM_AS_SYS_MEMORY
	void * p = psram_malloc( len );
	if(p!= NULL){
	   os_memset(p,0,len);
	}
        return p;//psram_mallocs
#else
	void * p = os_zalloc( len );
	if(p!= NULL){
	   os_memset(p,0,len);
	}
	return p;
#endif
}

void tls_mbedtls_mem_free(void *ptr)
{
#if CONFIG_PSRAM_AS_SYS_MEMORY
     psram_free(ptr);//psram_free
#else
     os_free(ptr);
#endif
}

#endif /* !MBEDTLS_PLATFORM_MEMORY */
