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



#include "_otp.h"

const otp_item_t otp_map[][32] = {
{
    {OTP_MEMORY_CHECK_MARK,                   96,    0x0,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_AES_KEY,                             32,    0x60,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_MODEL_ID,                             4,    0x100,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_MODEL_KEY,                           16,    0x104,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_ARM_DEVICE_ID,                        4,    0x114,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_DEVICE_ROOT_KEY,                     16,    0x118,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_BL1_BOOT_PUBLIC_KEY_HASH,            32,    0x128,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_BL2_BOOT_PUBLIC_KEY_HASH,            32,    0x148,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_ARM_LCS,                              4,    0x168,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_LOCK_CONTROL,                         4,    0x16c,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_BL1_SECURITY_COUNTER,                 4,    0x188,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_BL2_SECURITY_COUNTER,                64,    0x200,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_HUK,                                 32,    0x240,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_IAK,                                 32,    0x260,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_IAK_LEN,                              4,    0x280,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_IAK_TYPE,                             4,    0x284,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_IAK_ID,                              32,    0x288,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_BOOT_SEED,                           32,    0x2a8,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_LCS,                                  4,    0x2c8,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_IMPLEMENTATION_ID,                   32,    0x2cc,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_HW_VERSION,                          32,    0x2ec,    OTP_READ_WRITE,    OTP_SECURITY},
    {OTP_VERIFICATION_SERVICE_URL,            32,    0x30c,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_ENTROPY_SEED,                        64,    0x34c,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_SECURE_DEBUG_PK,                      2,    0x38c,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_MAC_ADDRESS,                          8,    0x3ac,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_VDDDIG_BANDGAP,                       2,    0x3b4,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_DIA,                                  2,    0x3b6,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_GADC_CALIBRATION,                     4,    0x3b8,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_SDMADC_CALIBRATION,                   4,    0x3bc,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_DEVICE_ID,                            8,    0x3c0,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_MEMORY_CHECK_VDDDIG,                  4,    0x3c8,    OTP_READ_WRITE,    OTP_NON_SECURITY},
    {OTP_GADC_TEMPERATURE,                     2,    0x3cc,    OTP_READ_WRITE,    OTP_NON_SECURITY},
},
{
    {OTP_RFCALI,          1024,    0x0,    OTP_READ_WRITE,    OTP_NON_SECURITY},
},
};

uint32_t otp_map_row(void)
{
    return sizeof(otp_map) / sizeof(otp_map[0]);
}

uint32_t otp_map_col(void)
{
    return sizeof(otp_map[0]) / sizeof(otp_map[0][0]);
}
