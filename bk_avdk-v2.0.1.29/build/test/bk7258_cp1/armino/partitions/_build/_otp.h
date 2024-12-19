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



#include <stdint.h>

typedef enum{
    OTP_READ_WRITE = 0,
    OTP_READ_ONLY = 0x3,
    OTP_NO_ACCESS = 0xF,
} otp_privilege_t;

typedef enum{
    OTP_SECURITY = 0,
    OTP_NON_SECURITY,
} otp_security_t;

typedef struct
{
    uint32_t  name;
    uint32_t  allocated_size;
    uint32_t  offset;
    otp_privilege_t privilege;
    otp_security_t  security;
} otp_item_t;

typedef enum{
    OTP_MEMORY_CHECK_MARK,
    OTP_AES_KEY,
    OTP_MODEL_ID,
    OTP_MODEL_KEY,
    OTP_ARM_DEVICE_ID,
    OTP_DEVICE_ROOT_KEY,
    OTP_BL1_BOOT_PUBLIC_KEY_HASH,
    OTP_BL2_BOOT_PUBLIC_KEY_HASH,
    OTP_ARM_LCS,
    OTP_LOCK_CONTROL,
    OTP_BL1_SECURITY_COUNTER,
    OTP_BL2_SECURITY_COUNTER,
    OTP_HUK,
    OTP_IAK,
    OTP_IAK_LEN,
    OTP_IAK_TYPE,
    OTP_IAK_ID,
    OTP_BOOT_SEED,
    OTP_LCS,
    OTP_IMPLEMENTATION_ID,
    OTP_HW_VERSION,
    OTP_VERIFICATION_SERVICE_URL,
    OTP_ENTROPY_SEED,
    OTP_SECURE_DEBUG_PK,
    OTP_MAC_ADDRESS,
    OTP_VDDDIG_BANDGAP,
    OTP_DIA,
    OTP_GADC_CALIBRATION,
    OTP_SDMADC_CALIBRATION,
    OTP_DEVICE_ID,
    OTP_MEMORY_CHECK_VDDDIG,
    OTP_GADC_TEMPERATURE,
} otp1_id_t;

typedef enum{
    OTP_RFCALI,
} otp2_id_t;

extern const otp_item_t otp_map[][32];

uint32_t otp_map_row(void);

uint32_t otp_map_col(void);
