// Copyright 2023 Beken
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_SHA256_ALT)
#include "bk_hash_alt_common.h"
#include "tls_log.h"
#include <mbedtls/sha256.h>

extern uint32_t bk_get_sym_ctx_size(void);

void __attribute__((weak)) mbedtls_sha256_init( mbedtls_sha256_context *ctx )
{
    if(ctx == NULL)
        return;
    os_memset( ctx, 0, sizeof( mbedtls_sha256_context ) );
}

void __attribute__((weak)) mbedtls_sha256_free( mbedtls_sha256_context *ctx )
{
    if(ctx == NULL)
        return;
    if(ctx != NULL && (NULL != ctx->Context_p)){
		val_SymRelease(ctx->Context_p);
        ctx->Context_p = NULL;
    }
    
    if(ctx != NULL && (NULL != ctx->DataLeft)){
        os_free(ctx->DataLeft);
        ctx->DataLeft = NULL;
    }
	os_memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void __attribute__((weak)) mbedtls_sha256_clone( mbedtls_sha256_context *dst,
                           const mbedtls_sha256_context *src )
{
    ValSymAlgo_t Algorithm = VAL_SYM_ALGO_NONE;

    if(dst == NULL || src == NULL){
		return;
	}    
    if(0 == src->is224){
        Algorithm = VAL_SYM_ALGO_HASH_SHA256;
    }else{
        Algorithm = VAL_SYM_ALGO_HASH_SHA224;
    }
    bk_val_sym_alloc(Algorithm, &(dst->Context_p));
    if(dst->DataLeft == NULL){
        dst->DataLeft = os_malloc(BK_SHA256_ALT_MAX_BLOCK_SIZE);
        os_memset(dst->DataLeft, 0, BK_SHA256_ALT_MAX_BLOCK_SIZE);
    }
    dst->LenLeft = src->LenLeft;
    dst->is224 = src->is224;
    os_memcpy(dst->digest, src->digest, BK_ALT_DIGEST_SHA256_SIZE);
    os_memcpy(dst->DataLeft, src->DataLeft, BK_SHA256_ALT_MAX_BLOCK_SIZE);
    os_memcpy(dst->Context_p, src->Context_p, bk_get_sym_ctx_size());
}

int __attribute__((weak)) mbedtls_internal_sha256_process( mbedtls_sha256_context *ctx,
                                     const unsigned char data[64] )
{
    if(ctx == NULL){
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }
	return 0;
}

int __attribute__((weak)) mbedtls_sha256_starts_ret( mbedtls_sha256_context *ctx, int is224 )
{
    ValSymAlgo_t Algorithm = VAL_SYM_ALGO_NONE;
    int ret = 0;
    
    if(ctx == NULL){
        ret = MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
        return ret;
    }

    if(is224 < 0 || is224 > 1){
        ret = MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
        return ret;
    }
    if(0 == is224){
        ctx->is224 = 0;
        Algorithm = VAL_SYM_ALGO_HASH_SHA256;
    }else{
        ctx->is224 = 1;
        Algorithm = VAL_SYM_ALGO_HASH_SHA224;
    }
    bk_val_sym_alloc(Algorithm, &(ctx->Context_p));
    if(ctx->DataLeft == NULL){
        ctx->DataLeft = os_malloc(BK_SHA256_ALT_MAX_BLOCK_SIZE);
        os_memset(ctx->DataLeft, 0, BK_SHA256_ALT_MAX_BLOCK_SIZE);
    }
    return ret;

}

/*
 * SHA-256 process buffer
 */
int __attribute__((weak)) mbedtls_sha256_update_ret( mbedtls_sha256_context *ctx,
                               const unsigned char *input,
                               size_t ilen )
{
    int ret = bk_hash_update(ctx->Context_p, ctx->DataLeft, &(ctx->LenLeft), SHA256_ALT, input, ilen);
    if(ret < 0){
        ret = MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
        mbedtls_sha256_free(ctx);
    }
    return ret;
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void __attribute__((weak)) mbedtls_sha256_update( mbedtls_sha256_context *ctx,
                            const unsigned char *input,
                            size_t ilen )
{
    mbedtls_sha256_update_ret( ctx, input, ilen );
}
#endif

/*
 * SHA-256 final digest
 */
int __attribute__((weak)) mbedtls_sha256_finish_ret( mbedtls_sha256_context *ctx,
                               unsigned char output[32] )
{
    int ret = 0;
    if(ctx == NULL || output == NULL){
        ret = MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
        goto cleanup;
    }
	ValSize_t digest_size = sizeof(ctx->digest);    
    ret = val_SymHashFinal(ctx->Context_p,
                            ctx->DataLeft,
                            ctx->LenLeft,
                            ctx->digest,
                            &digest_size); 
    if(ret != VAL_SUCCESS){
        MBEDTLS_LOGE("val_SymHashFinal return %d\r\n", ret);
        ret = MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
        goto cleanup;        
    }
    os_memcpy(output, ctx->digest, sizeof(ctx->digest));

    ctx->LenLeft = 0;
    return ret;

cleanup:
    mbedtls_sha256_free(ctx);
    return ret;
}


#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void __attribute__((weak)) mbedtls_sha256_finish( mbedtls_sha256_context *ctx,
                            unsigned char output[32] )
{
    mbedtls_sha256_finish_ret( ctx, output );
}
#endif

#endif /* MBEDTLS_SHA256_ALT */