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

#pragma once

#include "mailbox_driver_base.h"

#if CONFIG_SYS_CPU0
//struct mailbox_direction_t:
#define MAILBAOX_CONFIG_INFO_TABLE \
{	\
	{MAILBOX0, INT_SRC_NONE, MAILBOX_CPU0, MAILBOX_CPU1},\
	{MAILBOX1, INT_SRC_MAILBOX1, MAILBOX_CPU1, MAILBOX_CPU0},\
}

#else //#if CONFIG_SYS_CPU0

//struct mailbox_direction_t:
#define MAILBAOX_CONFIG_INFO_TABLE \
{	\
	{MAILBOX0, INT_SRC_MAILBOX0, MAILBOX_CPU0, MAILBOX_CPU1},\
	{MAILBOX1, INT_SRC_NONE, MAILBOX_CPU1, MAILBOX_CPU0},\
}

#endif //#if CONFIG_SYS_CPU0

