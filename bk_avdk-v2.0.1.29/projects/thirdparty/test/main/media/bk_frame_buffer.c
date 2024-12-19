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
// 旧版
#include <os/os.h>
#include <components/log.h>

#include <driver/int.h>
#include <os/mem.h>

#include <driver/media_types.h>
#include <driver/psram.h>
#include <driver/video_common_driver.h>

#include "bk_frame_buffer.h"

#include "bk_psram_mem_slab.h"

#define TAG "frame_buffer"

#include "bk_list.h"

#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)

#define list_for_each_safe_edge(pos, n, head)                                      \
	for (pos = (head)->next, n = pos->next; (pos != (head)) && (pos->next != pos); \
		 pos = n, n = pos->next)

extern uint32_t platform_is_in_interrupt_context(void);

fb_info_t *fb_info = NULL;
fb_mem_list_t fb_mem_list[FB_INDEX_MAX] = {0};
uint8_t fb_count[FB_INDEX_MAX] = {2, 2, 2};

// 根据给定的像素格式，返回对应的帧缓冲区内存列表
fb_mem_list_t *frame_buffer_list_get(pixel_format_t fmt)
{
	fb_mem_list_t *ret = NULL;

	switch (fmt)
	{
	case PIXEL_FMT_JPEG:
		ret = &fb_mem_list[FB_INDEX_JPEG];
		break;
	case PIXEL_FMT_RGB565:
	case PIXEL_FMT_RGB565_LE:
	case PIXEL_FMT_YUYV:
	case PIXEL_FMT_UYVY:
	case PIXEL_FMT_YYUV:
	case PIXEL_FMT_UVYY:
	case PIXEL_FMT_VUYY:
	case PIXEL_FMT_RGB888:
	case PIXEL_FMT_BGR888:
		ret = &fb_mem_list[FB_INDEX_DISPLAY];
		break;
	case PIXEL_FMT_H264:
		ret = &fb_mem_list[FB_INDEX_H264];
		break;
	case PIXEL_FMT_UNKNOW:
	default:
		break;
	}

	return ret;
}

fb_type_t frame_buffer_type_get(pixel_format_t fmt)
{
	fb_type_t ret = FB_INDEX_MAX;

	switch (fmt)
	{
	case PIXEL_FMT_JPEG:
		ret = FB_INDEX_JPEG;
		break;
	case PIXEL_FMT_RGB565_LE:
	case PIXEL_FMT_RGB565:
	case PIXEL_FMT_YUYV:
	case PIXEL_FMT_UYVY:
	case PIXEL_FMT_YYUV:
	case PIXEL_FMT_UVYY:
	case PIXEL_FMT_VUYY:
	case PIXEL_FMT_RGB888:
	case PIXEL_FMT_BGR888:
		ret = FB_INDEX_DISPLAY;
		break;
	case PIXEL_FMT_H264:
		ret = FB_INDEX_H264;
		break;
	case PIXEL_FMT_UNKNOW:
	default:
		break;
	}

	return ret;
}

// 从指定的列表中移除一个帧缓冲区节点
bk_err_t frame_buffer_list_remove(frame_buffer_t *frame, LIST_HEADER_T *list)
{
	frame_buffer_node_t *tmp = NULL;
	frame_buffer_node_t *node = list_entry(frame, frame_buffer_node_t, frame);
	LIST_HEADER_T *pos, *n;
	bk_err_t ret = BK_FAIL;

	list_for_each_safe(pos, n, list)
	{
		tmp = list_entry(pos, frame_buffer_node_t, list);
		if (tmp != NULL && (tmp->frame.frame == node->frame.frame))
		{
			list_del(pos);
			ret = BK_OK;
			break;
		}
	}

	return ret;
}

// 分配节点
int frame_buffer_fb_init(fb_type_t type)
{
	fb_mem_list_t *mem_list = &fb_mem_list[type];

	if (mem_list->enable == true)
	{
		LOGE("%s already init\n", __func__);
		return BK_FAIL;
	}

	if (type == FB_INDEX_DISPLAY)
	{
		mem_list->mode = FB_MEM_ALONE;
		mem_list->count = fb_count[type];
		mem_list->free_request = false;
		mem_list->ready_request = false;

		if (rtos_init_semaphore_ex(&mem_list->free_sem, mem_list->count, 0) != BK_OK)
		{
			LOGE("%s free_sem init failed\n", __func__);
		}

		if (rtos_init_semaphore_ex(&mem_list->ready_sem, mem_list->count, 0) != BK_OK)
		{
			LOGE("%s ready_sem init failed\n", __func__);
		}
	}
	else if (type == FB_INDEX_JPEG || type == FB_INDEX_H264)
	{
		mem_list->mode = FB_MEM_SHARED;
		mem_list->count = fb_count[type];
	}
	else
	{
		LOGE("%s unknow type: %d\n", __func__, type);
		return BK_FAIL;
	}

	for (int i = 0; i < fb_count[type]; i++)
	{
		frame_buffer_node_t *node = (frame_buffer_node_t *)os_malloc(sizeof(frame_buffer_node_t));

		if (node == NULL)
		{
			LOGE("%s os_malloc node failed\n", __func__);
			return BK_FAIL;
		}

		os_memset(node, 0, sizeof(frame_buffer_node_t));

		list_add_tail(&node->list, &mem_list->free);
	}

	mem_list->enable = true;

	return BK_OK;
}

//
int frame_buffer_fb_deinit(fb_type_t type)
{
	int ret = BK_OK;
	fb_mem_list_t *mem_list = NULL;
	frame_buffer_node_t *tmp = NULL;
	LIST_HEADER_T *pos, *n;
	uint32_t isr_context = platform_is_in_interrupt_context();

	GLOBAL_INT_DECLARATION() = 0;

	mem_list = &fb_mem_list[type];

	if (mem_list->enable == false)
	{
		LOGE("%s already deinit\n", __func__);
		return ret;
	}

	if (!isr_context)
	{
		rtos_lock_mutex(&mem_list->lock);
		GLOBAL_INT_DISABLE();
	}

	if (type == FB_INDEX_DISPLAY)
	{
		LOGI("display mem deinit\n");

		if (!isr_context)
		{
			GLOBAL_INT_RESTORE();
		}

		if (rtos_deinit_semaphore(&mem_list->free_sem) != BK_OK)
		{
			LOGE("%s free_sem init failed\n", __func__);
		}

		if (rtos_deinit_semaphore(&mem_list->ready_sem) != BK_OK)
		{
			LOGE("%s ready_sem init failed\n", __func__);
		}

		if (!isr_context)
		{
			GLOBAL_INT_DISABLE();
		}
	}
	else if (type == FB_INDEX_JPEG)
	{
		LOGI("jpeg mem deinit\n");
	}
	else if (type == FB_INDEX_H264)
	{
		LOGI("h264 mem deinit\n");
	}
	else
	{
		LOGE("%s unknow type: %d\n", __func__, type);
		ret = BK_FAIL;
		goto out;
	}

	if (!list_empty(&mem_list->free))
	{
		list_for_each_safe(pos, n, &mem_list->free)
		{
			tmp = list_entry(pos, frame_buffer_node_t, list);
			LOGD("free list: %p\n", tmp);
			if (tmp != NULL)
			{
				if (tmp->frame.base_addr)
				{
					bk_psram_frame_buffer_free(tmp->frame.base_addr);
					tmp->frame.base_addr = tmp->frame.frame = NULL;
				}
				list_del(pos);
				os_free(tmp);
				tmp = NULL;
			}
		}

		INIT_LIST_HEAD(&mem_list->free);
	}

	if (!list_empty(&mem_list->ready))
	{
		list_for_each_safe(pos, n, &mem_list->ready)
		{
			LOGD("ready list: %p\n", tmp);
			tmp = list_entry(pos, frame_buffer_node_t, list);
			if (tmp != NULL)
			{
				if (tmp->frame.base_addr)
				{
					bk_psram_frame_buffer_free(tmp->frame.base_addr);
					tmp->frame.base_addr = tmp->frame.frame = NULL;
				}
				list_del(pos);
				os_free(tmp);
				tmp = NULL;
			}
		}

		INIT_LIST_HEAD(&mem_list->ready);
	}

	mem_list->enable = false;

out:

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&mem_list->lock);
	}

	return ret;
}

// 清除指定类型的帧缓冲区的特定状态
void frame_buffer_fb_clear(fb_type_t type)
{
	fb_mem_list_t *mem_list = NULL;
	frame_buffer_node_t *tmp = NULL;
	LIST_HEADER_T *pos, *n;
	uint32_t isr_context = platform_is_in_interrupt_context();

	GLOBAL_INT_DECLARATION() = 0;

	mem_list = &fb_mem_list[type];

	if (mem_list->enable == false)
	{
		LOGE("%s already deinit\n", __func__);
		return;
	}

	if (!isr_context)
	{
		rtos_lock_mutex(&mem_list->lock);
		GLOBAL_INT_DISABLE();
	}

	if (!list_empty(&mem_list->ready))
	{
		list_for_each_safe(pos, n, &mem_list->ready)
		{
			tmp = list_entry(pos, frame_buffer_node_t, list);
			if (tmp != NULL)
			{
				tmp->frame.err_state = true;
			}
		}
	}

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&mem_list->lock);
	}
}

void frame_buffer_fb_free(frame_buffer_t *frame, frame_module_t index)
{
	if (frame == NULL || index >= MODULE_MAX)
	{
		LOGE("%s %d, frame is null\r\n", __func__, index);
		return;
	}

	fb_type_t type = frame_buffer_type_get(frame->fmt);
	fb_mem_list_t *mem_list = &fb_mem_list[type];
	frame_buffer_node_t *node = list_entry(frame, frame_buffer_node_t, frame);
	uint32_t isr_context = platform_is_in_interrupt_context();
	GLOBAL_INT_DECLARATION();

	if (mem_list == NULL)
	{
		LOGE("%s invalid mem_list: %p, %d\n", __func__, mem_list, index);
		if (fb_info->modules[index].enable)
		{
			if (fb_info->modules[index].sem)
			{
				rtos_set_semaphore(&fb_info->modules[index].sem);
			}
		}

		return;
	}

	if (!isr_context)
	{
		rtos_lock_mutex(&mem_list->lock);
		GLOBAL_INT_DISABLE();
	}

	if (mem_list->enable == false)
	{
		if (!isr_context)
		{
			GLOBAL_INT_RESTORE();
		}

		if (fb_info->modules[index].enable)
		{
			LOGE("%s fb_mem_list disable: %p, %d\n", __func__, mem_list, index);
			if (fb_info->modules[index].sem)
			{
				rtos_set_semaphore(&fb_info->modules[index].sem);
			}
		}

		if (!isr_context)
		{
			GLOBAL_INT_DISABLE();
		}
		goto out;
	}

	if (node->free_mask & INDEX_MASK(index))
	{
		LOGE("%s refree: %d\n", __func__, index);
		goto out;
	}
	else
	{
		node->free_mask |= INDEX_MASK(index);
	}

	if (node->read_mask)
	{
		if ((node->read_mask == node->free_mask) && ((node->free_mask & fb_info->register_mask[type]) == fb_info->register_mask[type]))
		{
			if (BK_OK != frame_buffer_list_remove(frame, &mem_list->ready))
			{
				LOGE("%s remove failed\n", __func__);
			}

			node->free_mask = 0;
			node->read_mask = 0;

			// fix frame_buffer address, not free
			/*if (node->frame.base_addr)
			{
				bk_psram_frame_buffer_free(node->frame.base_addr);
				node->frame.base_addr = node->frame.frame = NULL;
			}*/
			list_add_tail(&node->list, &mem_list->free);
		}

		if (!isr_context)
		{
			GLOBAL_INT_RESTORE();
		}

		if (fb_info->modules[index].enable)
		{
			rtos_lock_mutex(&fb_info->modules[index].lock);
			if (fb_info->modules[index].sem)
			{
				if (rtos_set_semaphore(&fb_info->modules[index].sem) != BK_OK)
					LOGE("%s semaphore set faile\n", __func__);
			}
			rtos_unlock_mutex(&fb_info->modules[index].lock);
		}

		if (!isr_context)
		{
			GLOBAL_INT_DISABLE();
		}
	}
	else
	{
		/* safte check */
		if (BK_OK != frame_buffer_list_remove(frame, &mem_list->ready))
		{
			LOGD("%s remove failed\n", __func__);
		}

		if (node->free_mask != 0)
		{
			node->free_mask = 0;
			node->read_mask = 0;
			list_add_tail(&node->list, &mem_list->free);
		}
	}

out:

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&mem_list->lock);
	}
}

// 此函数用于分配指定类型的帧缓冲区内存。
// 它首先尝试从空闲列表中获取一个节点
// 如果没有可用节点，则尝试从就绪列表中回收一个节点
// 如果仍然找不到合适的节点，则分配新的PSRAM内存
frame_buffer_t *frame_buffer_fb_malloc(fb_type_t type, uint32_t size)
{
	// bk_printf("222222222222222222222-------------------------\n");
	frame_buffer_node_t *node = NULL, *tmp = NULL;
	fb_mem_list_t *mem_list = &fb_mem_list[type];
	LIST_HEADER_T *pos, *n;
	uint8_t i_frame = 0;
	uint32_t isr_context = platform_is_in_interrupt_context();

	if (mem_list == NULL)
	{
		LOGE("%s invalid mem_list: %p\n", __func__, mem_list);
		return NULL;
	}

	if (mem_list->lock == NULL)
	{
		LOGE("[%s]:mem_list->lock is NULL\r\n", __func__);
		return NULL;
	}

	GLOBAL_INT_DECLARATION() = 0;

	if (!isr_context)
	{
		rtos_lock_mutex(&mem_list->lock);
		GLOBAL_INT_DISABLE();
	}

malloc:

	i_frame = 0;
	pos = NULL, n = NULL;
	list_for_each_safe(pos, n, &mem_list->free)
	{
		tmp = list_entry(pos, frame_buffer_node_t, list);
		if (tmp != NULL)
		{
			node = tmp;
			list_del(pos);
			break;
		}
	}
#if 0 
	if (node == NULL && type == FB_INDEX_H264)
	{
		list_for_each_safe(pos, n, &mem_list->ready)
		{
			tmp = list_entry(pos, frame_buffer_node_t, list);

			if (tmp != NULL
			    && (tmp->read_mask == tmp->free_mask))
			{
				LOGD("%s, %x, %d\r\n", __func__, tmp->frame.h264_type ,__LINE__);
				if (tmp->frame.h264_type & (0x1 << H264_NAL_I_FRAME))
				{
					i_frame++;
					if (i_frame < 2)
					{
						LOGD("%s, %d, %d\r\n", __func__, tmp->frame.sequence ,__LINE__);
						list_del(pos);
						frame_buffer_fb_direct_free_without_lock(&tmp->frame);
					}
					else
					{
						LOGD("%s, %x, %d, i_frame:%d\r\n", __func__, tmp->frame.h264_type ,__LINE__, i_frame);
						goto malloc;
					}
				}
				else
				{
					if (i_frame > 0)
					{
						i_frame++;
						LOGD("%s, %d, %d\r\n", __func__, tmp->frame.sequence ,__LINE__);
						list_del(pos);
						frame_buffer_fb_direct_free_without_lock(&tmp->frame);
					}
				}

				if (i_frame == H264_GOP_FRAME_CNT)//
				{
					LOGD("%s, %x, %d, i_frame:%d\r\n", __func__, tmp->frame.h264_type ,__LINE__, i_frame);
					goto malloc;
				}
			}
		}
	}
#endif
	// find a free node
	if (node)
	{
		// get frame free list, need malloc psram buffer again
		if (node->frame.base_addr == NULL)
		{
			node->frame.size = size;
#if (CONFIG_PSRAM_WRITE_THROUGH)
			node->frame.size += 32; // 1024
#endif
			// malloc psram mem to frame_buffer data
			if (type == FB_INDEX_DISPLAY)
			{
				// bk_printf("4444444444444444444444-------------------------\n");
				// need align 64K
#if CONFIG_SOC_BK7256XX
				node->frame.size += 64 * 1024;
				node->frame.base_addr = (uint8_t *)bk_psram_frame_buffer_malloc(PSRAM_HEAP_YUV, node->frame.size);
				if (node->frame.base_addr != NULL)
				{
					if ((uint32_t)node->frame.base_addr & 0xFFFF)
					{
						node->frame.frame = (uint8_t *)((((uint32_t)node->frame.base_addr >> 16) + 1) << 16);
					}
				}
#else
				bk_printf("11111111111111111111111111111111-------------------------\n");
				node->frame.base_addr = (uint8_t *)bk_psram_frame_buffer_malloc(PSRAM_HEAP_YUV, node->frame.size);
				node->frame.frame = node->frame.base_addr;
#endif
			}
			else
			{
				// bk_printf("3333333333333333333333333333-------------------------\n");
				node->frame.base_addr = (uint8_t *)bk_psram_frame_buffer_malloc(PSRAM_HEAP_ENCODE, node->frame.size);
				node->frame.frame = node->frame.base_addr;
			}

			if (node->frame.base_addr == NULL)
			{
				// add this node to free_list, and from ready node_list to find a frame_buffer(direct free)
				// list_add_tail(&node->list, &mem_list->free);
				os_free(node);
				node = NULL;
				goto malloc;
			}
#if (CONFIG_PSRAM_WRITE_THROUGH)
			else
			{
				// checkout node->frame->frame is aligned 32byte(8word)
				if ((uint32_t)node->frame.base_addr & 0x1F)
				{
					node->frame.frame = (uint8_t *)((((uint32_t)node->frame.base_addr >> 5) + 1) << 5);
				}

				LOGD("%s, %d, %p, size:%d\r\n", __func__, type, node->frame.frame, size);
			}
#endif
		}
	}
#if 0
	if (node == NULL && type != FB_INDEX_H264)
	{
		list_for_each_safe(pos, n, &mem_list->ready)
		{
			tmp = list_entry(pos, frame_buffer_node_t, list);
			if (tmp != NULL
			    && (tmp->read_mask == tmp->free_mask))
			{
				node = tmp;
				list_del(pos);
				break;
			}
		}
	}
#endif
	if (node == NULL)
	{
		list_for_each_safe(pos, n, &mem_list->ready)
		{
			tmp = list_entry(pos, frame_buffer_node_t, list);
			if (tmp != NULL && (tmp->read_mask == tmp->free_mask))
			{
				node = tmp;
				list_del(pos);
				break;
			}
		}
	}
	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&mem_list->lock);
	}

	if (node == NULL)
	{
		LOGE("%s %d i_frame:%d failed\n", __func__, type, i_frame);
		return NULL;
	}

	node->frame.size = size;
	node->frame.length = 0;
	node->frame.width = 0;
	node->frame.height = 0;
	node->frame.h264_type = 0;
	node->frame.fmt = 0;
	node->read_mask = 0;
	node->free_mask = 0;
	node->frame.mix = 0;
	node->frame.err_state = false;
	node->frame.cb = NULL;

	return &node->frame;
}

// 将一个帧缓冲区节点添加到就绪列表，并根据需要设置信号量，以便其他模块可以访问这个帧缓冲区
void frame_buffer_fb_push(frame_buffer_t *frame)
{
	fb_mem_list_t *mem_list = NULL;
	fb_type_t type = frame_buffer_type_get(frame->fmt);
	frame_buffer_node_t *tmp = NULL, *node = list_entry(frame, frame_buffer_node_t, frame);
	uint32_t isr_context = platform_is_in_interrupt_context();
	LIST_HEADER_T *pos, *n;
	uint32_t i = 0, length = 0;
	bk_err_t ret;
	GLOBAL_INT_DECLARATION() = 0;

	if (type > FB_INDEX_MAX)
	{
		LOGE("%s invalid frame type\n", __func__);
		return;
	}

	mem_list = &fb_mem_list[type];

	if (!isr_context)
	{
		rtos_lock_mutex(&mem_list->lock);
		GLOBAL_INT_DISABLE();
	}

	node->read_mask = 0;
	node->free_mask = 0;

	list_add_tail(&node->list, &mem_list->ready);

	if (type == FB_INDEX_DISPLAY)
	{
		list_for_each_safe(pos, n, &mem_list->ready)
		{
			tmp = list_entry(pos, frame_buffer_node_t, list);
			if (tmp != NULL)
			{
				length++;
			}
		}

		if (length == 1)
		{
			if (!isr_context)
			{
				GLOBAL_INT_RESTORE();
			}

			ret = rtos_set_semaphore(&mem_list->ready_sem);

			if (ret != BK_OK)
			{
				LOGD("%s semaphore set failed: %d\n", __func__, ret);
			}

			if (!isr_context)
			{
				GLOBAL_INT_DISABLE();
			}
		}
	}
	else
	{
		for (i = 0; i < MODULE_MAX; i++)
		{
			if (!isr_context)
			{
				GLOBAL_INT_RESTORE();
			}

			if (fb_info->modules[i].enable == true && fb_info->modules[i].type == type)
			{
				LOGD("cmp plugin\n");

				if (mem_list->mode == FB_MEM_SHARED && fb_info->modules[i].plugin == false)
				{
					if (fb_info->modules[i].sem)
					{
						ret = rtos_set_semaphore(&fb_info->modules[i].sem);
						if (ret != BK_OK)
						{
							LOGE("%s semaphore set failed: %d\n", __func__, ret);
						}
					}

					fb_info->modules[i].plugin = true;
				}
			}

			if (!isr_context)
			{
				GLOBAL_INT_DISABLE();
			}
		}
	}

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&mem_list->lock);
	}
}

// 列表中弹出一个帧缓冲区，供特定模块使用。它检查每个节点，找到第一个没有被当前模块读取的帧缓冲区并返回
frame_buffer_t *frame_buffer_fb_pop(frame_module_t index, fb_type_t type)
{
	fb_mem_list_t *mem_list = &fb_mem_list[type];
	LIST_HEADER_T *pos, *n;
	frame_buffer_node_t *node = NULL, *tmp = NULL;
	uint32_t isr_context = platform_is_in_interrupt_context();
	GLOBAL_INT_DECLARATION();

	if (mem_list == NULL)
	{
		LOGE("%s invalid mem_list: %p\n", __func__, mem_list);
		return NULL;
	}

	if (fb_info == NULL)
	{
		LOGE("%s fb_info was NULL\n", __func__);
		return NULL;
	}

	if (!isr_context)
	{
		rtos_lock_mutex(&mem_list->lock);
		GLOBAL_INT_DISABLE();
	}

	LOGD("type: %d, %p\n", type, mem_list);

	list_for_each_safe_edge(pos, n, &mem_list->ready)
	{
		tmp = list_entry(pos, frame_buffer_node_t, list);
		if (tmp != NULL && ((tmp->read_mask & INDEX_MASK(index)) == 0))
		{
			// LOGI("GET %u, %x, %x\n", tmp->frame.sequence, tmp->read_mask, tmp->read_mask & INDEX_MASK(index));
			node = tmp;
			break;
		}
		else if (tmp == NULL)
			LOGI("tmp == NULL\n");
	}

	if (node != NULL)
	{
		node->read_mask |= INDEX_MASK(index);
	}

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&mem_list->lock);
	}

	if (node == NULL)
	{
		LOGD("pop NULL\n");
		return NULL;
	}

	// LOGI("pop %u, %x, %x\n", node->frame.sequence, node->read_mask, tmp->read_mask);

	return &node->frame;
}

// 遍历帧缓冲区内存列表，找到第一个非空的就绪列表，返回其类型索引
fb_type_t frame_buffer_available_index(void)
{
	uint32_t isr_context = platform_is_in_interrupt_context();
	fb_type_t index = FB_INDEX_JPEG;
	GLOBAL_INT_DECLARATION();
	uint32_t i = 0;

	if (!isr_context)
	{
		rtos_lock_mutex(&fb_info->lock);
		GLOBAL_INT_DISABLE();
	}

	for (i = 0; i < FB_INDEX_MAX; i++)
	{
		if (!list_empty(&fb_mem_list[i].ready))
		{
			index = i;
			break;
		}
	}

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&fb_info->lock);
	}

	return index;
}

// 用于模块从帧缓冲区中读取数据。它尝试获取信号量，然后从就绪列表中弹出一个帧缓冲区
frame_buffer_t *frame_buffer_fb_read(frame_module_t index)
{
	frame_buffer_t *frame = NULL;
	bk_err_t ret = BK_FAIL;
	uint32_t isr_context = platform_is_in_interrupt_context();
	GLOBAL_INT_DECLARATION();
	fb_type_t type;

	if (index >= MODULE_MAX)
	{
		LOGE("%s invalid module: %d\n", __func__, index);
		goto out;
	}

	if (fb_info->modules[index].enable == false)
	{
		LOGE("%s module not register: %d\n", __func__, index);
		goto out;
	}

	do
	{
		rtos_lock_mutex(&fb_info->modules[index].lock);

		if (fb_info->modules[index].enable == true)
		{
			ret = rtos_get_semaphore(&fb_info->modules[index].sem, 500); // BEKEN_NEVER_TIMEOUT
		}

		if (ret != BK_OK)
		{
			LOGD("%s semaphore get failed: %d, index:%d, plugin:%d\n", __func__, ret, index, fb_info->modules[index].plugin);
			rtos_unlock_mutex(&fb_info->modules[index].lock);
			goto out;
		}

		if (fb_info->modules[index].enable == false)
		{
			rtos_unlock_mutex(&fb_info->modules[index].lock);
			break;
		}

		rtos_unlock_mutex(&fb_info->modules[index].lock);

		type = fb_info->modules[index].type;

		frame = frame_buffer_fb_pop(index, type);

		if (frame)
		{
			if (frame->err_state)
			{
				os_printf("frame->err_state\n");
				frame_buffer_fb_free(frame, index);
				continue;
			}
		}

		if (frame == NULL)
		{
			LOGD("%s faild, plugin: %d\n", __func__, fb_info->modules[index].plugin);

			if (!isr_context)
			{
				rtos_lock_mutex(&fb_info->lock);
				GLOBAL_INT_DISABLE();
			}

			fb_info->modules[index].plugin = false;

			if (!isr_context)
			{
				GLOBAL_INT_RESTORE();
				rtos_unlock_mutex(&fb_info->lock);
			}
		}
		else
		{
			break;
		}
	} while (fb_info && fb_info->modules[index].enable);

out:

	return frame;
}

// 注册一个模块到帧缓冲区管理系统，初始化信号量并设置模块的类型
bk_err_t frame_buffer_fb_register(frame_module_t index, fb_type_t type)
{
	bk_err_t ret = BK_FAIL;
	uint32_t isr_context = platform_is_in_interrupt_context();
	GLOBAL_INT_DECLARATION();

	if (fb_info->modules[index].enable == true)
	{
		LOGE("%s frame_module index already register\n", __func__);
		return ret;
	}

	if (index >= MODULE_MAX)
	{
		LOGE("%s invalid module: %d\n", __func__, index);
		return ret;
	}

	if (fb_info == NULL)
	{
		LOGE("%s fb_info was NULL\n", __func__);
		return ret;
	}

	if (!isr_context)
	{
		rtos_lock_mutex(&fb_info->lock);
	}

	// rtos_init_mutex(&fb_info->modules[index].lock);

	ret = rtos_init_semaphore_ex(&fb_info->modules[index].sem, 1, 0);

	if (ret != BK_OK)
	{
		LOGE("%s semaphore init failed: %d\n", __func__, ret);
		if (!isr_context)
		{
			rtos_unlock_mutex(&fb_info->lock);
		}
		return ret;
	}

	if (!isr_context)
	{
		GLOBAL_INT_DISABLE();
	}

	fb_info->modules[index].enable = true;
	fb_info->modules[index].plugin = false;
	fb_info->modules[index].type = type;
	fb_info->register_mask[type] |= INDEX_MASK(index);

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&fb_info->lock);
	}
	return ret;
}

bk_err_t frame_buffer_fb_deregister(frame_module_t index, fb_type_t type)
{
	bk_err_t ret = BK_FAIL;
	uint32_t isr_context = platform_is_in_interrupt_context();
	GLOBAL_INT_DECLARATION();

	if (fb_info->modules[index].enable == false)
	{
		LOGE("%s frame_module index already degister\n", __func__);
		return ret;
	}

	if (index >= MODULE_MAX)
	{
		LOGE("%s invalid module: %d\n", __func__, index);
		return ret;
	}

	if (fb_info == NULL)
	{
		LOGE("%s fb_info was NULL\n", __func__);
		return ret;
	}

	if (!isr_context)
	{
		rtos_lock_mutex(&fb_info->lock);
		GLOBAL_INT_DISABLE();
	}

	fb_info->modules[index].enable = false;
	fb_info->register_mask[type] &= INDEX_UNMASK(index);

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
	}

	ret = rtos_set_semaphore(&fb_info->modules[index].sem);

	rtos_lock_mutex(&fb_info->modules[index].lock);
	ret = rtos_deinit_semaphore(&fb_info->modules[index].sem);
	rtos_unlock_mutex(&fb_info->modules[index].lock);
	// rtos_deinit_mutex(&fb_info->modules[index].lock);

	if (ret != BK_OK)
	{
		LOGE("%s semaphore deinit failed: %d\n", __func__, ret);
	}

	if (!isr_context)
	{
		rtos_unlock_mutex(&fb_info->lock);
	}

	return ret;
}

// 直接释放帧缓冲区
void frame_buffer_fb_direct_free_without_lock(frame_buffer_t *frame)
{
	if (frame == NULL)
	{
		LOGE("%s, frame is null\r\n", __func__);
		return;
	}

	bk_err_t ret = BK_OK;
	LIST_HEADER_T *pos, *n;
	fb_type_t type = frame_buffer_type_get(frame->fmt);
	fb_mem_list_t *mem_list = &fb_mem_list[type];
	frame_buffer_node_t *tmp = NULL, *node = list_entry(frame, frame_buffer_node_t, frame);
	uint32_t length = 0;
	uint32_t isr_context = platform_is_in_interrupt_context();
	GLOBAL_INT_DECLARATION() = 0;

	if (mem_list == NULL || type >= FB_INDEX_MAX)
	{
		LOGE("%s invalid mem_list: %p, type:%d\n", __func__, mem_list, type);
		return;
	}

	if (!isr_context)
	{
		// rtos_lock_mutex(&mem_list->lock);
		GLOBAL_INT_DISABLE();
	}

	node->free_mask = 0;
	node->read_mask = 0;

	// fix frame_buffer address, not free
	/*if (node->frame.base_addr)
	{
		bk_psram_frame_buffer_free(node->frame.base_addr);
		node->frame.base_addr = node->frame.frame = NULL;
	}*/

	list_add_tail(&node->list, &mem_list->free);

	if (type == FB_INDEX_DISPLAY)
	{
		list_for_each_safe(pos, n, &mem_list->free)
		{
			tmp = list_entry(pos, frame_buffer_node_t, list);
			if (tmp != NULL)
			{
				length++;
			}
		}

		if (mem_list->free_request == true)
		{
			if (!isr_context)
			{
				GLOBAL_INT_RESTORE();
			}

			ret = rtos_set_semaphore(&mem_list->free_sem);

			if (ret != BK_OK)
			{
				LOGE("%s semaphore set failed: %d\n", __func__, ret);
			}

			if (!isr_context)
			{
				GLOBAL_INT_DISABLE();
			}

			mem_list->free_request = false;
		}
	}

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		// rtos_unlock_mutex(&mem_list->lock);
	}
}

// 直接释放帧缓冲区,在释放前会获取互斥锁
void frame_buffer_fb_direct_free(frame_buffer_t *frame)
{
	if (frame == NULL)
	{
		LOGE("%s, frame is null\r\n", __func__);
		return;
	}

	bk_err_t ret = BK_OK;
	LIST_HEADER_T *pos, *n;
	fb_type_t type = frame_buffer_type_get(frame->fmt);
	fb_mem_list_t *mem_list = &fb_mem_list[type];
	frame_buffer_node_t *tmp = NULL, *node = list_entry(frame, frame_buffer_node_t, frame);
	uint32_t length = 0;
	uint32_t isr_context = platform_is_in_interrupt_context();
	GLOBAL_INT_DECLARATION() = 0;

	if (mem_list == NULL || type >= FB_INDEX_MAX)
	{
		LOGE("%s invalid mem_list: %p, type:%d\n", __func__, mem_list, type);
		return;
	}

	if (!isr_context)
	{
		rtos_lock_mutex(&mem_list->lock);
		GLOBAL_INT_DISABLE();
	}

	node->free_mask = 0;
	node->read_mask = 0;

	// fix frame_buffer address, not free
	/*if (node->frame.base_addr)
	{
		bk_psram_frame_buffer_free(node->frame.base_addr);
		node->frame.base_addr = node->frame.frame = NULL;
	}*/

	list_add_tail(&node->list, &mem_list->free);

	if (type == FB_INDEX_DISPLAY)
	{
		list_for_each_safe(pos, n, &mem_list->free)
		{
			tmp = list_entry(pos, frame_buffer_node_t, list);
			if (tmp != NULL)
			{
				length++;
			}
		}

		if (mem_list->free_request == true)
		{
			if (!isr_context)
			{
				GLOBAL_INT_RESTORE();
			}

			ret = rtos_set_semaphore(&mem_list->free_sem);

			if (ret != BK_OK)
			{
				LOGE("%s semaphore set failed: %d\n", __func__, ret);
			}

			if (!isr_context)
			{
				GLOBAL_INT_DISABLE();
			}

			mem_list->free_request = false;
		}
	}

	if (!isr_context)
	{
		GLOBAL_INT_RESTORE();
		rtos_unlock_mutex(&mem_list->lock);
	}
}

// 用于等待并分配显示用的帧缓冲区内存。如果内存不足，它会等待直到有足够的内存可用
frame_buffer_t *frame_buffer_fb_display_malloc_wait(uint32_t size)
{
	frame_buffer_t *frame = NULL;
	fb_mem_list_t *mem_list = &fb_mem_list[FB_INDEX_DISPLAY];
	uint32_t isr_context = platform_is_in_interrupt_context();
	bk_err_t ret;
	GLOBAL_INT_DECLARATION();

	do
	{
		
		frame = frame_buffer_fb_malloc(FB_INDEX_DISPLAY, size);

		if (frame == NULL)
		{
			if (!isr_context)
			{
				rtos_lock_mutex(&mem_list->lock);
				GLOBAL_INT_DISABLE();
			}

			mem_list->free_request = true;

			if (!isr_context)
			{
				GLOBAL_INT_RESTORE();
				rtos_unlock_mutex(&mem_list->lock);
			}

			ret = rtos_get_semaphore(&mem_list->free_sem, BEKEN_NEVER_TIMEOUT);

			if (ret != BK_OK)
			{
				LOGD("%s semaphore get failed: %d\n", __func__, ret);
			}

			continue;
		}

	} while (0);

	return frame;
}

// 用于从显示用的帧缓冲区中弹出一个帧缓冲区
frame_buffer_t *frame_buffer_fb_display_pop(void)
{
	// LOGW("%s fframe_buffer_fb_display_pop start\n",__func__);
	frame_buffer_node_t *node = NULL, *tmp = NULL;
	fb_mem_list_t *mem_list = &fb_mem_list[FB_INDEX_DISPLAY];
	LIST_HEADER_T *pos, *n;
	GLOBAL_INT_DECLARATION() = 0;

	rtos_lock_mutex(&mem_list->lock);
	GLOBAL_INT_DISABLE();

	if (mem_list == NULL)
	{
		// LOGW("%s mem_list is  null\n", __func__);
	}


// #define list_for_each_safe(pos, n, head) 
// 	for (pos = (head)->next, n = pos->next; pos != (head); 
// 		pos = n, n = pos->next)

	list_for_each_safe(pos, n, &mem_list->ready)
	{
		// LOGW("%s list_for_each_safe is ok\n", __func__);
		tmp = list_entry(pos, frame_buffer_node_t, list);
		if (tmp != NULL)
		{
			node = tmp;
			// LOGW("%s get node is not null\n", __func__);
			list_del(pos);
			break;
		}
		else
		{
			// LOGW("%s failed  1111\n", __func__);
		}
	}

	GLOBAL_INT_RESTORE();
	rtos_unlock_mutex(&mem_list->lock);

	if (node == NULL)
	{
		// LOGW("%s failed  2222\n", __func__);
		return NULL;
	}
	// LOGW("%s node->frame is not null\n", __func__);
	return &node->frame;
}

// 用于从显示用的帧缓冲区中弹出一个帧缓冲区,在没有可用帧缓冲区时等待
frame_buffer_t *frame_buffer_fb_display_pop_wait(void)
{
	frame_buffer_t *frame = NULL;
	fb_mem_list_t *mem_list = &fb_mem_list[FB_INDEX_DISPLAY];
	bk_err_t ret;

	do
	{
		frame = frame_buffer_fb_display_pop();

		if (frame != NULL)
		{
			break;
		}

		ret = rtos_get_semaphore(&mem_list->ready_sem, 1000);

		if (ret != BK_OK)
		{
			LOGD("%s semaphore get failed: %d\n", __func__, ret);
			break;
		}

	} while (true);

	return frame;
}

// 初始化整个帧缓冲区管理系统
void frame_buffer_init(void)
{
	uint32_t i = 0;

	// 对每个帧缓冲区内存块进行初始化
	for (i = 0; i < FB_INDEX_MAX; i++)
	{
		fb_mem_list[i].free.next = &fb_mem_list[i].free;
		fb_mem_list[i].free.prev = &fb_mem_list[i].free;
		fb_mem_list[i].ready.next = &fb_mem_list[i].ready;
		fb_mem_list[i].ready.prev = &fb_mem_list[i].ready;
		fb_mem_list[i].enable = false;		   // 表示当前内存块未启用
		rtos_init_mutex(&fb_mem_list[i].lock); // 初始化内存块的互斥锁
	}

	if (fb_info == NULL)
	{
		fb_info = (fb_info_t *)os_malloc(sizeof(fb_info_t));
		os_memset((void *)fb_info, 0, sizeof(fb_info_t));
		rtos_init_mutex(&fb_info->lock);

		// 初始化帧缓冲区信息结构体中每个模块的互斥锁
		for (uint8_t index = 0; index < MODULE_MAX; index++)
		{
			rtos_init_mutex(&fb_info->modules[index].lock);
		}
	}
	// PSRAM 帧缓冲区的初始化
	bk_psram_frame_buffer_init();
}

void frame_buffer_deinit(void)
{
	if (fb_info)
	{
		for (uint8_t index = 0; index < MODULE_MAX; index++)
		{
			rtos_deinit_mutex(&fb_info->modules[index].lock);
		}

		os_free(fb_info);
		fb_info = NULL;
	}
}

frame_buffer_t *frame_buffer_display_malloc(uint32_t size)
{
	frame_buffer_t *frame = bk_psram_frame_buffer_malloc(PSRAM_HEAP_YUV, size + sizeof(frame_buffer_t) + 32);

	if (frame == NULL)
	{
		return NULL;
	}

	os_memset(frame, 0, sizeof(frame_buffer_t));
	frame->frame = (uint8_t *)((((uint32_t)(frame + 1) >> 5) + 1) << 5);

	return frame;
}

void frame_buffer_display_free(frame_buffer_t *frame)
{
	bk_psram_frame_buffer_free(frame);
}
