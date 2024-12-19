#ifndef _DVP_CAMERA_H_
#define _DVP_CAMERA_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <driver/int.h>
#include <os/mem.h>
#include <driver/gpio.h>
#include <driver/gpio_types.h>

#include <driver/dma.h>
#include <driver/i2c.h>
#include <driver/jpeg_enc.h>
#include <driver/jpeg_enc_types.h>


#include <driver/dvp_camera.h>
#include <driver/dvp_camera_types.h>
#include <driver/video_common_driver.h>

typedef enum
{
	SENSER_UNKNOWN,
	SENSER_NULL,
	SENSER_GC0328C,
	SENSER_GC2145,

	// 添加其他传感器类型
} sensor_type_t;


bk_err_t bk_dvp_camera_open(media_camera_device_t *device);
bk_err_t bk_dvp_camera_close(void);
bk_err_t bk_dvp_camera_set_compress(compress_ratio_t *config);

char *bk_dvp_camera_getinfo(void);

// sensor_type_t get_sensor_type(void);

#ifdef __cplusplus
}
#endif
#endif
