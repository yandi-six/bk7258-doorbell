
#include <driver/int.h>
#include <os/mem.h>
#include <components/log.h>
#include <driver/gpio.h>
#include <driver/gpio_types.h>

#include <driver/dma.h>
#include <driver/i2c.h>
#include <driver/jpeg_enc.h>
#include <driver/jpeg_enc_types.h>

#include <driver/dvp_camera.h>
#include <driver/dvp_camera_types.h>
#include <driver/video_common_driver.h>
#include "bk_psram_mem_slab.h"
#include "rtc_bk.h"
#include "bk_dvp.h"
#include "bk_frame_buffer.h"

#define TAG "dvp-camera"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)


int video_frame_buffer_fb_init(fb_type_t type)
{
	// LOGW("%s %d \n", __func__, __LINE__);
	return BK_OK;
}
int video_frame_buffer_fb_deinit(fb_type_t type)
{
	// LOGW("%s %d \n", __func__, __LINE__);
	return BK_OK;
}
void video_frame_buffer_fb_clear(fb_type_t type)
{
	// LOGW("%s %d \n", __func__, __LINE__);
}
frame_buffer_t *video_frame_buffer_fb_malloc(fb_type_t type, uint32_t size)
{
	// LOGW("%s %d \n", __func__, __LINE__);
	frame_buffer_t *frame = (frame_buffer_t *)rtc_bk_malloc(sizeof(frame_buffer_t));
	if (frame != NULL)
	{
		frame->size = size;
		frame->base_addr = (uint8_t *)bk_psram_frame_buffer_malloc(PSRAM_HEAP_ENCODE, size);
		if (frame->base_addr == NULL)
		{
			rtc_bk_free(frame);
			frame = NULL;
			return NULL;
		}
		else
		{
			frame->frame = frame->base_addr;
			frame->size = size;
			frame->length = 0;
			frame->width = 0;
			frame->height = 0;
			frame->h264_type = 0;
			frame->fmt = 0;
			frame->mix = 0;
			frame->err_state = false;
			frame->cb = NULL;
			return frame;
		}
	}
	return NULL;
}

void video_frame_buffer_fb_push(frame_buffer_t *frame)
{
	// LOGW("%s %d \n", __func__, __LINE__);
}
void video_frame_buffer_fb_direct_free(frame_buffer_t *frame)
{
	// LOGW("%s %d \n", __func__, __LINE__);
	if (frame != NULL)
	{
		if (frame->base_addr != NULL)
		{
			bk_psram_frame_buffer_free(frame->base_addr);
		}
		rtc_bk_free(frame);
	}
}

bk_err_t bk_dvp_camera_open(media_camera_device_t *device)
{
	dvp_camera_config_t config = {0};
#if 1
	config.fb_init = frame_buffer_fb_init;
	config.fb_deinit = frame_buffer_fb_deinit;
	config.fb_clear = frame_buffer_fb_clear;
	config.fb_malloc = frame_buffer_fb_malloc;
	config.fb_complete = frame_buffer_fb_push;
	config.fb_free = frame_buffer_fb_direct_free;
#else
	config.fb_init = video_frame_buffer_fb_init;
	config.fb_deinit = video_frame_buffer_fb_deinit;
	config.fb_clear = video_frame_buffer_fb_clear;
	config.fb_malloc = video_frame_buffer_fb_malloc;
	config.fb_complete = video_frame_buffer_fb_push;
	config.fb_free = video_frame_buffer_fb_direct_free;
#endif
	config.device = device;

	return bk_dvp_camera_driver_init(&config);
}

bk_err_t bk_dvp_camera_close(void)
{
	bk_dvp_camera_driver_deinit();
	return 0;
}
bk_err_t bk_dvp_camera_set_compress(compress_ratio_t *config)
{
	return bk_video_compression_ratio_config(config);
}

char *bk_dvp_camera_getinfo(void)
{
	char *name;
	uint16_t id = 0;

	if (bk_dvp_camera_get_current_sensor() != NULL)
	{

		name = bk_dvp_camera_get_current_sensor()->name;
		return name;
	}
	else
	{
		bk_printf("senser is null\n");
		return NULL; // 或者返回一个默认字符串
	}

	// id = bk_dvp_camera_get_current_sensor() ->id;
}

