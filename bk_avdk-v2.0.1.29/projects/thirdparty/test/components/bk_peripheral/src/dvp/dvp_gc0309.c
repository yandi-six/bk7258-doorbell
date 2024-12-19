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

#define GC0309_WRITE_ADDRESS (0x42)
#define GC0309_READ_ADDRESS (0x43)
#define GC0309_CHIP_ID (0xA0)
#define FLIP_INIT_GC0309 0x10

#define TAG "gc0309"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)

#define SENSOR_I2C_READ(reg, value)                                         \
    do                                                                      \
    {                                                                       \
        dvp_camera_i2c_read_uint8((GC0309_WRITE_ADDRESS >> 1), reg, value); \
    } while (0)

#define SENSOR_I2C_WRITE(reg, value)                                         \
    do                                                                       \
    {                                                                        \
        dvp_camera_i2c_write_uint8((GC0309_WRITE_ADDRESS >> 1), reg, value); \
    } while (0)

bool gc0309_read_flag = false;

// gc0309_DEV
#if 1
const uint8_t sensor_gc0309_init_talbe[][2] =
    {
#if 1

        {0xfe, 0x80},
        {0xfe, 0x00}, // set page0

        {0x1a, 0x16},
        {0xd2, 0x10}, // close AEC
        {0x22, 0x55}, // close AWB

        {0x5a, 0x56},
        {0x5b, 0x40},
        {0x5c, 0x4a},

        {0x22, 0x57},

#if 0   // fix 14.3
	{0x01,0xfa},
	{0x02,0xd4},
	{0x0f,0x01},

	{0x03,0x01},
	{0x04,0x90},

	{0xe2,0x00},
	{0xe3,0x64},

	{0xe4,0x02},
	{0xe5,0xbc},
	{0xe6,0x02},
	{0xe7,0xbc},
	{0xe8,0x02},
	{0xe9,0xbc},
	{0xea,0x09},
	{0xeb,0xc4},
#elif 0 // mclk =13MHZ fps =8.3
        {0x01, 0x2c},
        {0x02, 0x88},
        {0x0f, 0x02},

        {0x03, 0x00},
        {0x04, 0x9c},

        {0xe2, 0x00},
        {0xe3, 0x34},

        {0xe4, 0x02},
        {0xe5, 0x70},
        {0xe6, 0x02},
        {0xe7, 0x70},
        {0xe8, 0x02},
        {0xe9, 0x70},
        {0xea, 0x04},
        {0xeb, 0x10},
#else // mclk =26MHZ fps =16.6--7.2
        {0x01, 0x2c},
        {0x02, 0xac},
        {0x0f, 0x02},

        {0x03, 0x01},
        {0x04, 0x38},
        // 294 318 420 840
        {0xe2, 0x00},
        {0xe3, 0x84},
        {0xe4, 0x02},
        {0xe5, 0x94},
        {0xe6, 0x03},
        {0xe7, 0x18},
        {0xe8, 0x04},
        {0xe9, 0x20},
        {0xea, 0x08},
        {0xeb, 0x40},
#endif
        {0x05, 0x00},
        {0x06, 0x00},
        {0x07, 0x00},
        {0x08, 0x00},
        {0x09, 0x01},
        {0x0a, 0xe8},
        {0x0b, 0x02},
        {0x0c, 0x88},
        {0x0d, 0x02},
        {0x0e, 0x02},
        {0x10, 0x22},
        {0x11, 0x0d},
        {0x12, 0x2a},
        {0x13, 0x00},
        {0x15, 0x0a},
        {0x16, 0x05},
        {0x17, 0x01},

        {0x1b, 0x03},
        {0x1c, 0xc1},
        {0x1d, 0x08},
        {0x1e, 0x20},
        {0x1f, 0x16},

        {0x20, 0xff},
        {0x21, 0xf8},
        {0x24, 0xa2},
        {0x25, 0x0f},
        // output sync_mode
        {0x26, 0x03},
        {0x2f, 0x01},
        /////////////////////////////////////////////////////////////////////
        /////////////////////////// grab_t ////////////////////////////////
        /////////////////////////////////////////////////////////////////////
        {0x30, 0xf7},
        {0x31, 0x40},
        {0x32, 0x00},
        {0x39, 0x04},
        {0x3a, 0x20},
        {0x3b, 0x20},
        {0x3c, 0x02},
        {0x3d, 0x02},
        {0x3e, 0x02},
        {0x3f, 0x02},

        // gain
        {0x50, 0x24},

        {0x53, 0x80},
        {0x54, 0x80},
        {0x55, 0x80},
        {0x56, 0x80},

        /////////////////////////////////////////////////////////////////////
        /////////////////////////// LSC_t ////////////////////////////////
        /////////////////////////////////////////////////////////////////////
        {0x8b, 0x20},
        {0x8c, 0x20},
        {0x8d, 0x20},
        {0x8e, 0x10},
        {0x8f, 0x10},
        {0x90, 0x10},
        {0x91, 0x3c},
        {0x92, 0x50},
        {0x5d, 0x12},
        {0x5e, 0x1a},
        {0x5f, 0x24},
        /////////////////////////////////////////////////////////////////////
        /////////////////////////// DNDD_t ///////////////////////////////
        /////////////////////////////////////////////////////////////////////
        {0x60, 0x07},
        {0x61, 0x0e},
        {0x62, 0x0f}, // 0x0c
        {0x64, 0x03},
        {0x66, 0xe8},
        {0x67, 0x86},
        {0x68, 0xa2},

        /////////////////////////////////////////////////////////////////////
        /////////////////////////// asde_t ///////////////////////////////
        /////////////////////////////////////////////////////////////////////
        {0x69, 0x20},
        {0x6a, 0x0f},
        {0x6b, 0x00},
        {0x6c, 0x53},
        {0x6d, 0x83},
        {0x6e, 0xac},
        {0x6f, 0xac},
        {0x70, 0x15},
        {0x71, 0x33},
        /////////////////////////////////////////////////////////////////////
        /////////////////////////// eeintp_t///////////////////////////////
        /////////////////////////////////////////////////////////////////////
        {0x72, 0xdc},
        {0x73, 0x80},
        // for high resolution in light scene
        {0x74, 0x02},
        {0x75, 0x3f},
        {0x76, 0x02},
        {0x77, 0x54},
        {0x78, 0x88},
        {0x79, 0x81},
        {0x7a, 0x81},
        {0x7b, 0x22},
        {0x7c, 0xff},

        /////////////////////////////////////////////////////////////////////
        ///////////////////////////CC_t///////////////////////////////
        /////////////////////////////////////////////////////////////////////
        {0x93, 0x48},
        {0x94, 0x00},
        {0x95, 0x05},
        {0x96, 0xe8},
        {0x97, 0x40},
        {0x98, 0xf8},
        {0x9c, 0x00},
        {0x9d, 0x00},
        {0x9e, 0x00},

        /////////////////////////////////////////////////////////////////////
        ///////////////////////////YCP_t///////////////////////////////
        /////////////////////////////////////////////////////////////////////
        {0xb1, 0x42},
        {0xb2, 0x42},
        {0xb8, 0x20},
        {0xbe, 0x36},
        {0xbf, 0x00},
        /////////////////////////////////////////////////////////////////////
        ///////////////////////////AEC_t///////////////////////////////
        /////////////////////////////////////////////////////////////////////
        {0xd0, 0xcb},
        {0xd1, 0x10},

        {0xd3, 0x50},
        {0xd5, 0xf2},
        {0xd6, 0x16},
        {0xdb, 0x92},
        {0xdc, 0xa5},
        {0xdf, 0x23},
        {0xd9, 0x00},
        {0xda, 0x00},
        {0xe0, 0x09},

        {0xec, 0x20},
        {0xed, 0x04},
        {0xee, 0xa0},
        {0xef, 0x40},
        ///////////////////////////////////////////////////////////////////
        ///////////////////////////GAMMA//////////////////////////////////
        ///////////////////////////////////////////////////////////////////
        // for FAE to choose the gamma curve according to different LCD
        {0x9F, 0x0e}, // gamma curve lvl2
        {0xA0, 0x1c},
        {0xA1, 0x34},
        {0xA2, 0x48},
        {0xA3, 0x5a},
        {0xA4, 0x6b},
        {0xA5, 0x7b},
        {0xA6, 0x95},
        {0xA7, 0xab},
        {0xA8, 0xbf},
        {0xA9, 0xce},
        {0xAA, 0xd9},
        {0xAB, 0xe4},
        {0xAC, 0xec},
        {0xAD, 0xf7},
        {0xAE, 0xfd},
        {0xAF, 0xff},

#if 0
	{0x9F,0x10},	// gamma curve lvl3
	{0xA0,0x20},
	{0xA1,0x38},
	{0xA2,0x4e},
	{0xA3,0x63},
	{0xA4,0x76},
	{0xA5,0x87},
	{0xA6,0xa2},
	{0xA7,0xb8},
	{0xA8,0xca},
	{0xA9,0xd8},
	{0xAA,0xe3},
	{0xAB,0xeb},
	{0xAC,0xf0},
	{0xAD,0xf8},
	{0xAE,0xfd},
	{0xAF,0xff},

	{0x9F,0x14},	// gamma curve lvl4
	{0xA0,0x28},
	{0xA1,0x44},
	{0xA2,0x5d},
	{0xA3,0x72},
	{0xA4,0x86},
	{0xA5,0x95},
	{0xA6,0xb1},
	{0xA7,0xc6},
	{0xA8,0xd5},
	{0xA9,0xe1},
	{0xAA,0xea},
	{0xAB,0xf1},
	{0xAC,0xf5},
	{0xAD,0xfb},
	{0xAE,0xfe},
	{0xAF,0xff},
#endif

        // Y_gamma
        {0xc0, 0x00},
        {0xc1, 0x0B},
        {0xc2, 0x15},
        {0xc3, 0x27},
        {0xc4, 0x39},
        {0xc5, 0x49},
        {0xc6, 0x5A},
        {0xc7, 0x6A},
        {0xc8, 0x89},
        {0xc9, 0xA8},
        {0xca, 0xC6},
        {0xcb, 0xE3},
        {0xcc, 0xFF},

        /////////////////////////////////////////////////////////////////
        /////////////////////////// ABS_t ///////////////////////////////
        /////////////////////////////////////////////////////////////////
        {0xf0, 0x02},
        {0xf1, 0x01},
        {0xf2, 0x00},
        {0xf3, 0x30},

        /////////////////////////////////////////////////////////////////
        /////////////////////////// Measure Window ///////////////////////
        /////////////////////////////////////////////////////////////////
        {0xf7, 0x04},
        {0xf8, 0x02},
        {0xf9, 0x9f},
        {0xfa, 0x78},

        //---------------------------------------------------------------
        {0xfe, 0x01},

        /////////////////////////////////////////////////////////////////
        ///////////////////////////AWB_p/////////////////////////////////
        /////////////////////////////////////////////////////////////////
        {0x00, 0xf5},
        //	{0x01,0x0a},
        {0x02, 0x1a},
        {0x0a, 0xa0},
        {0x0b, 0x60},
        {0x0c, 0x08},
        {0x0e, 0x4c},
        {0x0f, 0x39},
        {0x11, 0x3f},
        {0x12, 0x72},
        {0x13, 0x39},
        {0x14, 0x42},
        {0x15, 0x43},
        {0x16, 0xc2},
        {0x17, 0xa8},
        {0x18, 0x18},
        {0x19, 0x40},
        {0x1a, 0xd0},
        {0x1b, 0xf5},

        {0x70, 0x40},
        {0x71, 0x58},
        {0x72, 0x30},
        {0x73, 0x48},
        {0x74, 0x20},
        {0x75, 0x60},

        {0xfe, 0x00},

        {0xd2, 0x90}, // Open AEC
        {0x23, 0x00},
        {0x2d, 0x0a},
        {0x20, 0xff},
        {0x73, 0x00},
        {0x77, 0x33},
        {0xb3, 0x40},
        {0xb4, 0x80},
        {0xb5, 0x00},
        {0xba, 0x00},
        {0xbb, 0x00},

        {0x8b, 0x22},
        {0x71, 0x43},
        {0x31, 0x60},
        {0x1c, 0x49},
        {0x1d, 0x98},
        {0x10, 0x26},
        {0x1a, 0x26},

        {0x14, 0x10}, // Mirror UpsideDown

        {0xff, 0xff},

#endif
        // {0xfe, 0x00},
        // {0xb1, 0x00},
        // {0xb2, 0x00},
};
#endif

const uint8_t sensor_gc0309_QVGA_320_240_talbe[][2] =
    {
		{0xfe, 0x01},
		{0x54, 0x22},

		{0xFE, 0x00},
		{0x46, 0x00},
		{0x47, 0x00},
		{0x48, 0x00},
		{0x49, 0x00},
		{0x4a, 0xf0},
		{0x4b, 0x01},
		{0x4c, 0x40},
};

const uint8_t sensor_gc0309_VGA_640_480_talbe[][2] =
    {
		{0xfe, 0x01},
		{0x54, 0x11},

		{0xFE, 0x00},
		{0x46, 0x00},
		{0x47, 0x00},
		{0x48, 0x00},
		{0x49, 0x01},
		{0x4a, 0xe0},
		{0x4b, 0x02},
		{0x4c, 0x80},
};

bool gc0309_detect(void)
{
    uint8_t data = 0;
    // return false;
    SENSOR_I2C_READ(0x00, &data);

    LOGW("%s, id: 0x%02X\n", __func__, data);

    if (data == GC0309_CHIP_ID)
    {
        LOGW("%s success\n", __func__);
        return true;
    }

    return false;
}

void gc0309_read_register(uint8_t addr, uint8_t data)

{
    if (gc0309_read_flag)
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

int gc0309_init(void)

{
    uint32_t size = sizeof(sensor_gc0309_init_talbe) / 2, i;

    LOGI("%s\n", __func__);

    for (i = 0; i < size; i++)
    {
        SENSOR_I2C_WRITE(sensor_gc0309_init_talbe[i][0], sensor_gc0309_init_talbe[i][1]);

        gc0309_read_register(sensor_gc0309_init_talbe[i][0], sensor_gc0309_init_talbe[i][1]);
    }

    return 0;
}

int gc0309_set_ppi(media_ppi_t ppi)
{
    uint32_t size, i;
    int ret = -1;

    LOGI("%s\n", __func__);

    switch (ppi)
    {
    case PPI_320X240:
    {
        size = sizeof(sensor_gc0309_QVGA_320_240_talbe) / 2;

        for (i = 0; i < size; i++)
        {
            SENSOR_I2C_WRITE(sensor_gc0309_QVGA_320_240_talbe[i][0],
                             sensor_gc0309_QVGA_320_240_talbe[i][1]);

            gc0309_read_register(sensor_gc0309_QVGA_320_240_talbe[i][0],
                                 sensor_gc0309_QVGA_320_240_talbe[i][1]);
        }

        ret = 0;
    }
    break;

    case PPI_640X480:
    {

        size = sizeof(sensor_gc0309_VGA_640_480_talbe) / 2;
        for (i = 0; i < size; i++)
        {
            SENSOR_I2C_WRITE(sensor_gc0309_VGA_640_480_talbe[i][0],
                             sensor_gc0309_VGA_640_480_talbe[i][1]);

            gc0309_read_register(sensor_gc0309_VGA_640_480_talbe[i][0],
                                 sensor_gc0309_VGA_640_480_talbe[i][1]);
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

int gc0309_set_fps(frame_fps_t fps)
{
    return 0;
}

int gc0309_reset(void)
{
    SENSOR_I2C_WRITE(0xFE, 0x80);
    return 0;
}

int gc0309_dump(media_ppi_t ppi)
{
    uint32_t size, i;
    int ret = -1;
    uint8_t value = 0;

    LOGI("%s\n", __func__);

    size = sizeof(sensor_gc0309_init_talbe) / 2;

    for (i = 0; i < size; i++)
    {
        SENSOR_I2C_READ(sensor_gc0309_init_talbe[i][0], &value);
        LOGI("[0x%02x, 0x%02x]\r\n", sensor_gc0309_init_talbe[i][0], value);
    }

    switch (ppi)
    {
    case PPI_320X240:
    {
        size = sizeof(sensor_gc0309_QVGA_320_240_talbe) / 2;

        for (i = 0; i < size; i++)
        {
            SENSOR_I2C_READ(sensor_gc0309_QVGA_320_240_talbe[i][0], (uint8_t *)&value);
            LOGI("[%02x, %02x]\r\n", sensor_gc0309_QVGA_320_240_talbe[i][0], value);
        }

        ret = 0;
    }
    break;

    case PPI_640X480:
    {
        size = sizeof(sensor_gc0309_VGA_640_480_talbe) / 2;

        for (i = 0; i < size; i++)
        {
            SENSOR_I2C_READ(sensor_gc0309_VGA_640_480_talbe[i][0], &value);
            LOGI("[%02x, %02x]\r\n", sensor_gc0309_VGA_640_480_talbe[i][0], value);
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

void gc0309_read_enable(bool enable)
{
    gc0309_read_flag = enable;
}

const dvp_sensor_config_t dvp_sensor_gc0309 =
    {
        .name = "gc0309",
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
        .id = ID_GC0309,
        .address = (GC0309_WRITE_ADDRESS >> 1),
        .init = gc0309_init,
        .detect = gc0309_detect,
        .set_ppi = gc0309_set_ppi,
        .set_fps = gc0309_set_fps,
        .power_down = gc0309_reset,
        .dump_register = gc0309_dump,
        .read_register = gc0309_read_enable,
};
