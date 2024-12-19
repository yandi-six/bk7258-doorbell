// Copyright 2020-2021 Beken
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

#include <driver/int.h>
#include <os/mem.h>
#include <driver/gpio.h>
#include <driver/gpio_types.h>
#include <driver/jpeg_enc.h>
#include <driver/jpeg_enc_types.h>
#include <driver/dvp_camera.h>
#include <driver/dvp_camera_types.h>

#include <driver/i2c.h>

#include "dvp_sensor_devices.h"

#define GC0311_WRITE_ADDRESS (0x66)
#define GC0311_READ_ADDRESS (0x67)
#define GC0311_CHIP_ID (0xBB)
#define FLIP_INIT_GC0311 0x14

#define TAG "gc0311"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)

#define SENSOR_I2C_READ(reg, value) \
	do {\
		dvp_camera_i2c_read_uint8((GC0311_WRITE_ADDRESS >> 1), reg, value);\
	}while (0)

#define SENSOR_I2C_WRITE(reg, value) \
	do {\
		dvp_camera_i2c_write_uint8((GC0311_WRITE_ADDRESS >> 1), reg, value);\
	}while (0)


bool gc0311_read_flag = false;

// gc0311_DEV
#if 1
const uint8_t sensor_gc0311_init_talbe[][2] =
	{
		{0xfe, 0xf0},
		{0xfe, 0xf0},
		{0xfe, 0xf0},
		{0xfc, 0x16}, // clock enable
		{0xfc, 0x16}, // clock enable
		{0x42, 0x00},
		{0x4f, 0x01},
		{0x03, 0x02},
		{0x04, 0x88},

		{0x26, 0x00},
		{0x29, 0x40}, //[7]black_compress_en, [6:0]global_offset
		{0x2a, 0x30},
		{0x2b, 0x30},
		{0x2c, 0x30},
		{0x2d, 0x30},

		{0x77, 0x70},
		{0x78, 0x40},
		{0x79, 0x50},

		{0x73, 0x85},
		{0x74, 0x80},
		{0x75, 0x80},
		{0x76, 0x98},

		////////////  BLK /////////////////////
		////////////////////////////////////////////////
		{0x26, 0xf7}, //
		{0x28, 0x7f}, // BLK value limit
		{0x32, 0x00},
		{0x33, 0x20}, // offset_ratio_G1
		{0x34, 0x20}, // offset_ratio_G2
		{0x35, 0x20}, // offset ratio_R
		{0x36, 0x20}, // offset ratio_B
		{0x3b, 0x08},
		{0x3c, 0x08},
		{0x3d, 0x08},
		{0x3e, 0x08},

		/////////////CISCTL/////////////////////////////
		{0xfe, 0x00}, // Page select
		//////window setting/////
		{0x0d, 0x01}, //{8}cis_win_height
		{0x0e, 0xe8}, //{7:0}cis_win_height
		{0x0f, 0x02}, //{9:8}cis_win_width
		{0x10, 0x88}, //{7:0}cis_win_width
		{0x09, 0x00}, // row_start
		{0x0a, 0x00},
		{0x0b, 0x00}, // col_start
		{0x0c, 0x04},
		/////////////////banding////////////////
		{0x05, 0x02},
		{0x06, 0x2c}, // Hb
		{0x07, 0x00},
		{0x08, 0xb8}, // VB
		{0xfe, 0x01}, // VB
		{0x29, 0x00},
		{0x2a, 0x60}, // step
		{0x2b, 0x02},
		{0x2c, 0xa0}, // 15fps
		{0x2d, 0x03},
		{0x2e, 0x00}, // 11fps
		{0x2f, 0x03},
		{0x30, 0xc0}, // 10fps
		{0x31, 0x05},
		{0x32, 0x40}, // 8fps
		{0x33, 0x20},
		{0xfe, 0x00},

		{0x17, FLIP_INIT_GC0311}, // [1]updown, [0]mirror
		{0x19, 0x04},
		{0x1f, 0x08},
		{0x20, 0x01},
		{0x21, 0x48},
		{0x1b, 0x48}, // for decrease FPN
		{0x22, 0xba},
		{0x23, 0x06}, // 07 20140113 lanking
		{0x24, 0x16}, // PAD driver

		// global gain for range
		{0x70, 0x50}, // 58

		////////////////////////////////////////////////
		////////////block enable/////////////
		////////////////////////////////////////////////
		{0x40, 0xdf},
		{0x41, 0x2e},
		{0x42, 0xf9},
		{0x44, 0xa2}, // yuv
		{0x46, 0x03}, // sync_mode
		{0x4d, 0x01},
		{0x4f, 0x01},
		{0x7e, 0x08},
		{0x7f, 0xc3},

		// DN & EEINTP
		{0x80, 0xe7},
		{0x82, 0xa0},
		{0x83, 0x02},
		{0x84, 0x02},
		{0x89, 0x22},
		{0x90, 0x00},
		{0x92, 0x08},
		{0x94, 0x08},
		{0x95, 0x24},

		/////////////////////ASDE////////////////
		{0x9a, 0x20}, // Y offset Limit
		{0x9c, 0x86},
		{0xa4, 0x50},
		{0xa7, 0xf0}, // 40 travis 20160127
		{0xaa, 0x60},
		{0xa2, 0x45},

		////////////////Y gamma ////////////////
		//////////////////////////////////////////
		{0xfe, 0x00},
		{0x63, 0x00},
		{0x64, 0x04},
		{0x65, 0x09},
		{0x66, 0x17},
		{0x67, 0x2b},
		{0x68, 0x3d},
		{0x69, 0x4f},
		{0x6a, 0x5f},
		{0x6b, 0x82},
		{0x6c, 0xa2},
		{0x6d, 0xc5},
		{0x6e, 0xe5},
		{0x6f, 0xff},
		{0xfe, 0x00},

		////////////////RGB gamma //////////////
		///////////////////////////////////////

		{0xfe, 0x00},
		{0xbf, 0x0b},
		{0xc0, 0x17},
		{0xc1, 0x2a},
		{0xc2, 0x41},
		{0xc3, 0x54},
		{0xc4, 0x66},
		{0xc5, 0x74},
		{0xc6, 0x8c},
		{0xc7, 0xa3},
		{0xc8, 0xb5},
		{0xc9, 0xc4},
		{0xca, 0xd0},
		{0xcb, 0xdb},
		{0xcc, 0xe5},
		{0xcd, 0xf0},
		{0xce, 0xf7},
		{0xcf, 0xff},

		/// YUV maxtrix////
		{0xb0, 0x13},
		{0xb1, 0x1f},
		{0xb2, 0x0e},
		{0xb3, 0xF5},
		{0xb4, 0xE5},
		{0xb5, 0x26},
		{0xb6, 0x20},
		{0xb7, 0xde},
		{0xb8, 0x02},
		/////////////YCP//////////////
		////////////////////////////
		{0xd1, 0x28},
		{0xd2, 0x28},
		{0xdd, 0x00},
		{0xed, 0x00},

		{0xde, 0x34}, // autogray_mode
		{0xe4, 0x00},
		{0xe5, 0x40},

		// luma_div
		{0xfe, 0x01},
		{0x18, 0x21},

		///////////MEANSURE WINDOW////////
		{0x08, 0xa4},
		{0x09, 0xf0},

		//////////////////AEC///////////////////////
		{0xfe, 0x01},
		{0x10, 0x08}, // 08//18//[7]low_light_mode,[6]measure_point
		{0x11, 0x11}, //[6:4]very N
		{0x12, 0x00}, // 90 before Gamma,should set bit[7] as 0
		{0x13, 0x48}, // Y target
		{0x15, 0xfe}, // fc //high range
		{0x16, 0xd8}, // AEC ignore
		{0x17, 0x98}, // AEC ignore en
		{0x1a, 0x61}, // SLOW margin
		{0x18, 0x21}, // LUMA
		{0x21, 0xc0}, // auto_post_gain
		{0x22, 0x60}, // auto pre gain

		//////////////////AWB////////////////////

		{0x06, 0x10},
		{0x08, 0xa0},

		{0x50, 0xfe},
		{0x51, 0x05},
		{0x52, 0x28},
		{0x53, 0x05},
		{0x54, 0x10},
		{0x55, 0x20},
		{0x56, 0x16}, // AWB C max
		{0x57, 0x10},
		{0x58, 0xf2},
		{0x59, 0x10},
		{0x5a, 0x10},
		{0x5b, 0xf0},
		{0x5e, 0xe8},
		{0x5f, 0x20},
		{0x60, 0x20},
		{0x61, 0xe0},

		{0x62, 0x03},
		{0x63, 0x30},
		{0x64, 0xc0},
		{0x65, 0xd0},
		{0x66, 0x20},
		{0x67, 0x00}, // 80 //AWB_outdoor_mode

		{0x6d, 0x90},
		{0x6e, 0x40},
		{0x6f, 0x08},
		{0x70, 0x10},
		{0x71, 0x62},
		{0x72, 0x24},
		{0x73, 0x71},
		{0x74, 0x10},

		{0x75, 0x40},
		{0x76, 0x40},
		{0x77, 0xc9},
		{0x78, 0x9c},

		{0x79, 0x18},
		{0x7a, 0x40},
		{0x7b, 0xb0},
		{0x7c, 0xf5},

		{0x81, 0x80},
		{0x82, 0x42},
		{0x83, 0x98},

		{0x8a, 0xf8},
		{0x8b, 0xf4},
		{0x8c, 0x0a},
		{0x8d, 0x00},
		{0x8e, 0x00},
		{0x8f, 0x00},
		{0x90, 0x12},
		{0xfe, 0x00},

		//////////////// SPI reciver////////////////
		{0xad, 0x00},
		//////////////////LSC///////////////////////
		{0xfe, 0x01},
		{0xc0, 0x1e},
		{0xc1, 0x18},
		{0xc2, 0x17},
		{0xc6, 0x1e},
		{0xc7, 0x18},
		{0xc8, 0x17},
		{0xba, 0x30},
		{0xbb, 0x25},
		{0xbc, 0x20},
		{0xb4, 0x30},
		{0xb5, 0x25},
		{0xb6, 0x20},
		{0xc3, 0x00},
		{0xc4, 0x00},
		{0xc5, 0x00},
		{0xc9, 0x00},
		{0xca, 0x00},
		{0xcb, 0x00},
		{0xbd, 0x00},
		{0xbe, 0x00},
		{0xbf, 0x00},
		{0xb7, 0x00},
		{0xb8, 0x00},
		{0xb9, 0x00},
		{0xa8, 0x0f},
		{0xa9, 0x05},
		{0xaa, 0x00},
		{0xab, 0x0f},
		{0xac, 0x05},
		{0xad, 0x00},
		{0xae, 0x0f},
		{0xaf, 0x05},
		{0xb0, 0x00},
		{0xb1, 0x0f},
		{0xb2, 0x05},
		{0xb3, 0x00},
		{0xa4, 0x00},
		{0xa5, 0x00},
		{0xa6, 0x00},
		{0xa7, 0x00},
		{0xa1, 0x3c},
		{0xa2, 0x50},
		{0xfe, 0x00},

		{0x50, 0x01}, // crop
		{0xfe, 0x01},
		{0x9c, 0x00},
		{0x9e, 0xc0},
		{0x9f, 0x40},
		{0xfe, 0x00},
		{0x42, 0xfb},
		{0xf1, 0x07}, // output enable
		{0xf2, 0x01}, // output enable DVP or SPI

};
#endif



const uint8_t sensor_gc0311_QVGA_320_240_talbe[][2] =
	{

};

const uint8_t sensor_gc0311_VGA_640_480_talbe[][2] =
	{


};


bool gc0311_detect(void)
{
	uint8_t data = 0;
    //return false;
	SENSOR_I2C_READ(0xf0, &data);

	LOGW("%s, id: 0x%02X\n", __func__, data);

	if (data == GC0311_CHIP_ID)
	{
		LOGW("%s success\n", __func__);
		return true;
	}

	return false;
}

void gc0311_read_register(uint8_t addr, uint8_t data)

{
	if (gc0311_read_flag)
	{
		if (addr == 0x4e)
		{
			return;
		}

		uint8_t value = 0;
		rtos_delay_milliseconds(2);
		SENSOR_I2C_READ(addr, &value);
		if (value != data)
		{
			LOGI("0x%02x, 0x%02x-0x%02x\r\n", addr, data, value);
		}
	}
}

int gc0311_init(void)

{
	uint32_t size = sizeof(sensor_gc0311_init_talbe) / 2, i;

	LOGI("%s\n", __func__);

	for (i = 0; i < size; i++)
	{
		SENSOR_I2C_WRITE(sensor_gc0311_init_talbe[i][0], sensor_gc0311_init_talbe[i][1]);

		gc0311_read_register(sensor_gc0311_init_talbe[i][0], sensor_gc0311_init_talbe[i][1]);
	}

	return 0;
}

int gc0311_set_ppi(media_ppi_t ppi)
{
	uint32_t size, i;
	int ret = -1;

	LOGI("%s\n", __func__);

	switch (ppi)
	{
	case PPI_320X240:
	{
		size = sizeof(sensor_gc0311_QVGA_320_240_talbe) / 2;

		for (i = 0; i < size; i++)
		{
			SENSOR_I2C_WRITE(sensor_gc0311_QVGA_320_240_talbe[i][0],
							 sensor_gc0311_QVGA_320_240_talbe[i][1]);

			gc0311_read_register(sensor_gc0311_QVGA_320_240_talbe[i][0],
								 sensor_gc0311_QVGA_320_240_talbe[i][1]);
		}

		ret = 0;
	}
	break;

	case PPI_640X480:
	{

		size = sizeof(sensor_gc0311_VGA_640_480_talbe) / 2;
		for (i = 0; i < size; i++)
		{
			SENSOR_I2C_WRITE(sensor_gc0311_VGA_640_480_talbe[i][0],
							 sensor_gc0311_VGA_640_480_talbe[i][1]);

			gc0311_read_register(sensor_gc0311_VGA_640_480_talbe[i][0],
								 sensor_gc0311_VGA_640_480_talbe[i][1]);
		}

		ret = 0;
	}
	break;

	case PPI_800X600:
	case PPI_1280X720:
	default:
		break;
	}

	return ret;
}

int gc0311_set_fps(frame_fps_t fps)
{
	return 0;
}

int gc0311_reset(void)
{
	SENSOR_I2C_WRITE(0xFE, 0x80);
	return 0;
}

int gc0311_dump(media_ppi_t ppi)
{
	uint32_t size, i;
	int ret = -1;
	uint8_t value = 0;

	LOGI("%s\n", __func__);

	size = sizeof(sensor_gc0311_init_talbe) / 2;

	for (i = 0; i < size; i++)
	{
		SENSOR_I2C_READ(sensor_gc0311_init_talbe[i][0], &value);
		LOGI("[0x%02x, 0x%02x]\r\n", sensor_gc0311_init_talbe[i][0], value);
	}

	switch (ppi)
	{
	case PPI_320X240:
	{
		size = sizeof(sensor_gc0311_QVGA_320_240_talbe) / 2;

		for (i = 0; i < size; i++)
		{
			SENSOR_I2C_READ(sensor_gc0311_QVGA_320_240_talbe[i][0], (uint8_t *)&value);
			LOGI("[%02x, %02x]\r\n", sensor_gc0311_QVGA_320_240_talbe[i][0], value);
		}

		ret = 0;
	}
	break;

	case PPI_640X480:
	{
		size = sizeof(sensor_gc0311_VGA_640_480_talbe) / 2;

		for (i = 0; i < size; i++)
		{
			SENSOR_I2C_READ(sensor_gc0311_VGA_640_480_talbe[i][0], &value);
			LOGI("[%02x, %02x]\r\n", sensor_gc0311_VGA_640_480_talbe[i][0], value);
		}

		ret = 0;
	}
	break;

	case PPI_800X600:
	case PPI_1280X720:
	default:
		break;
	}

	ret = kNoErr;

	return ret;

}

void gc0311_read_enable(bool enable)
{
	gc0311_read_flag = enable;
}


const dvp_sensor_config_t dvp_sensor_gc0311 =
{
	.name = "gc0311",
	.clk = MCLK_24M,
	.fmt = PIXEL_FMT_YUYV,
	.vsync = SYNC_HIGH_LEVEL,
	.hsync = SYNC_HIGH_LEVEL,
	/* default config */
	.def_ppi = PPI_640X480,
	.def_fps = FPS25,
	/* capability config */
	.fps_cap = FPS5 | FPS10 | FPS20 | FPS25 | FPS30,
	.ppi_cap = PPI_CAP_320X240 | PPI_CAP_320X480 | PPI_CAP_480X272 | PPI_CAP_640X480 | PPI_CAP_480X320,
	.id = ID_GC0311,
	.address = (GC0311_WRITE_ADDRESS >> 1),
	.init = gc0311_init,
	.detect = gc0311_detect,
	.set_ppi = gc0311_set_ppi,
	.set_fps = gc0311_set_fps,
	.power_down = gc0311_reset,
	.dump_register = gc0311_dump,
	.read_register = gc0311_read_enable,
};

