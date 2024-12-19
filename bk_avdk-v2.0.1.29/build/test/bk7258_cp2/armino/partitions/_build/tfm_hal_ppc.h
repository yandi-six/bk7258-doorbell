// Copyright 2022-2024 Beken
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

//This is a generated file, don't modify it!

#pragma once


static inline void tfm_hal_ppc_init(void)
{
    bk_prro_set_secure(PRRO_DEV_AON, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_AON_WDT, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_APB_WDT, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_AUD, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_BTDM, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_BTDM_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_CAN, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_CKMN, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_DISP, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_DISP_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_DMA0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_DMA1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_DMAD, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_DMAD_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_EFS, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_ENET, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_ENET_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_FLASH, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIOHIG, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_H264, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_HSU_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_I2C0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_I2C1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_I2S0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_I2S1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_I2S2, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_IRDA, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_JPDEC_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_JPENC_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_JPGD, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_JPGE, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_LA, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_LA_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_LIN, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_MAC, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_MAC_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_MBOX_AHB, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_MCHECK, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_MOD, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_PPRO_MPC, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_PSRAM, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_PWM0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_PWM1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_QSPI0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_QSPI1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_REG, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_ROTT, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_ROTT_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_RTC, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SADC, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SBC, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SCAL0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SCAL0_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SCAL1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SCAL1_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SCLD, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SCR, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SDIO, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SDMADC, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SPI0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SPI1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_SYS, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_TIMER0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_TIMER1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_TRNG, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_UART0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_UART1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_UART2, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_USB, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_USB_M, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_XVR, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_YUV, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO0, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO1, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO2, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO3, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO4, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO5, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO6, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO7, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO8, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO9, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO10, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO11, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO12, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO13, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO14, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO15, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO16, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO17, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO18, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO19, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO20, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO21, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO22, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO23, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO24, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO25, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO26, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO27, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO28, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO29, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO30, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO31, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO32, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO33, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO34, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO35, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO36, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO37, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO38, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO39, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO40, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO41, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO42, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO43, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO44, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO45, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO46, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO47, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO48, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO49, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO50, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO51, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO52, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO53, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO54, PRRO_SECURE);
    bk_prro_set_secure(PRRO_DEV_GPIO55, PRRO_SECURE);
}