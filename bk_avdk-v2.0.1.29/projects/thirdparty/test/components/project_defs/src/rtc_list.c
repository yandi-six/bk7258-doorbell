
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include "rtc_list.h"
#include "rtc_bk.h"
// 创建一个新的链表结点，分配内存并初始化
RTCList *rtc_list_new(void *data)
{
	RTCList *new_elem = (RTCList *)rtc_bk_malloc(sizeof(RTCList));
	if (new_elem != NULL)
	{
		new_elem->prev = NULL;
		new_elem->next = NULL;
		new_elem->data = data;
		return new_elem;
	}
	return NULL;
}

RTCList *rtc_list_append_link(RTCList *elem, RTCList *new_elem)
{
	RTCList *pelem = elem;
	if (elem == NULL)
		return new_elem;

	while (pelem->next != NULL)
		pelem = pelem->next; // rtc_list_next(pelem)

	pelem->next = new_elem;
	new_elem->prev = pelem;

	return elem;
}

RTCList *rtc_list_append(RTCList *elem, void *data)
{
	RTCList *new_elem = rtc_list_new(data);

	return rtc_list_append_link(elem, new_elem);
}

// 头插
RTCList *rtc_list_prepend(RTCList *elem, void *data)
{
	RTCList *new_elem = rtc_list_new(data);

	if (elem != NULL)
	{
		new_elem->next = elem;
		elem->prev = new_elem;
	}
	return new_elem;
}

// 连接两个链表
RTCList *rtc_list_concat(RTCList *first, RTCList *second)
{
	RTCList *it = first;
	if (it == NULL)
		return second;
	while (it->next != NULL)
		it = rtc_list_next(it); // it->next
	it->next = second;
	second->prev = it;
	return first;
}

// 释放整个链表的内存
RTCList *rtc_list_free(RTCList *list)
{
	RTCList *elem = list;
	RTCList *tmp = NULL;
	if (list == NULL)
		return NULL;
	while (elem->next != NULL)
	{
		tmp = elem;
		elem = elem->next;
		rtc_bk_free(tmp);
	}
	rtc_bk_free(elem);
	return NULL;
}

// 从链表中移除特定数据的结点
RTCList *rtc_list_remove(RTCList *first, void *data)
{
	RTCList *it = NULL;
	it = rtc_list_find(first, data);
	if (it)
		return rtc_list_remove_link(first, it);
	else
	{
		return first;
	}
}

// 获取链表的大小
int rtc_list_size(const RTCList *first)
{
	int n = 0;
	RTCList *elem = (RTCList *)first;
	while (elem != NULL)
	{
		++n;
		elem = elem->next;
	}
	return n;
}

// 对链表中的每个结点执行指定的函数
void rtc_list_for_each(const RTCList *list, void (*func)(void *))
{
	RTCList *next;
	RTCList *elem = (RTCList *)list;
	while (elem != NULL)
	{
		next = elem->next;
		func(elem->data);
		elem = next;
	}
}

void rtc_list_for_each2(const RTCList *list, void (*func)(void *, void *), void *user_data)
{
	RTCList *next = NULL;
	RTCList *elem = (RTCList *)list;
	while (elem != NULL)
	{
		next = elem->next;
		func(elem->data, user_data);
		elem = next;
	}
}

//+++++
void rtc_list_for_each3(const RTCList *list, int (*func)(char *, int, void *, char *), int streamid, void *request, char *endtime, int index)
{
	RTCList *next = NULL;
	RTCList *elem = (RTCList *)list;
	// int count = 20;
	os_printf("rtc_list_for_each3执行\n");
	while (elem != NULL)
	{

		next = elem->next;

		// os_printf("list size = %d,data=%s\n",rtc_list_size(list),rtc_list_nth_data(list,rtc_list_position(list,elem)));

		if (rtc_list_position(list, elem) >= index) //&& count >= 1
		{
			func(elem->data, streamid, request, endtime);
			os_printf("index:%d,data:%s\n", rtc_list_position(list, elem), elem->data);
			// count--;
		}
		elem = next;
	}
}

void rtc_list_for_each4(const RTCList *list, int (*func)(char *, size_t), char *user_data)
{
	RTCList *next = NULL;
	RTCList *elem = (RTCList *)list;
	while (elem != NULL)
	{
		next = elem->next;
		if (strcmp(elem->data,user_data) != 0 )
		{
			func(elem->data, strlen(elem->data));
			os_printf("%s\n",elem->data);
			//rtc_list_remove(list,elem->data);
		}
		elem = next;
	}
}

//+++++
// 移除一个结点
RTCList *rtc_list_remove_link(RTCList *list, RTCList *elem)
{
	RTCList *ret = NULL;
	if (elem == list)
	{
		ret = elem->next;
		elem->prev = NULL;
		elem->next = NULL;
		if (ret != NULL)
			ret->prev = NULL;
		rtc_bk_free(elem);
		return ret;
	}
	elem->prev->next = elem->next;
	if (elem->next != NULL)
		elem->next->prev = elem->prev;
	elem->next = NULL;
	elem->prev = NULL;
	rtc_bk_free(elem);
	return list;
}

// 查找特定数据的结点
RTCList *rtc_list_find(RTCList *list, void *data)
{
	RTCList *next = NULL;
	RTCList *elem = list;
	while (elem != NULL)
	{
		next = elem->next;
		if (elem->data == data)
			return elem;
		elem = next;
	}
	return NULL;
}

// 使用自定义比较函数在链表中查找匹配的结点
RTCList *rtc_list_find_custom(RTCList *list, int (*compare_func)(const void *, const void *), const void *user_data)
{
	RTCList *next = NULL;
	RTCList *elem = list;
	while (elem != NULL)
	{
		next = elem->next;
		if (compare_func(elem->data, user_data) == 0)
			return elem;
		elem = next;
	}
	return NULL;
}

// 获取链表中特定索引位置的结点数据
void *rtc_list_nth_data(const RTCList *list, int index)
{
	int i;
	for (i = 0; list != NULL; list = list->next, ++i)
	{
		if (i == index)
			return list->data;
	}
	return NULL;
}

// 获取链表中结点的位置
int rtc_list_position(const RTCList *list, RTCList *elem)
{
	int i;
	for (i = 0; list != NULL; list = list->next, ++i)
	{
		if (elem == list)
			return i;
	}
	return -1;
}

// 获取链表中数据的索引
int rtc_list_index(const RTCList *list, void *data)
{
	int i;
	for (i = 0; list != NULL; list = list->next, ++i)
	{
		if (data == list->data)
			return i;
	}
	return -1;
}

// 将链表中的结点按自定义比较函数排序后插入
RTCList *rtc_list_insert_sorted(RTCList *list, void *data, int (*compare_func)(void *, void *, int), int type)
{
	RTCList *it, *previt = NULL;
	RTCList *nelem = NULL;
	RTCList *ret = list;
	if (list == NULL)
		return rtc_list_append(list, data);
	else
	{
		nelem = rtc_list_new(data);
		for (it = list; it != NULL; it = it->next)
		{
			previt = it;
			if (compare_func(data, it->data, type) <= 0)
			{
				nelem->prev = it->prev;
				nelem->next = it;
				if (it->prev != NULL)
					it->prev->next = nelem;
				else
				{
					ret = nelem;
				}
				it->prev = nelem;
				return ret;
			}
		}
		previt->next = nelem;
		nelem->prev = previt;
	}
	return ret;
}

// 在链表的指定位置之前插入新结点
RTCList *rtc_list_insert(RTCList *list, RTCList *before, void *data)
{
	RTCList *elem = NULL;
	if (list == NULL || before == NULL)
		return rtc_list_append(list, data);
	for (elem = list; elem != NULL; elem = rtc_list_next(elem))
	{
		if (elem == before)
		{
			if (elem->prev == NULL)
				return rtc_list_prepend(list, data);
			else
			{
				RTCList *nelem = rtc_list_new(data);
				nelem->prev = elem->prev;
				nelem->next = elem;
				elem->prev->next = nelem;
				elem->prev = nelem;
			}
		}
	}
	return list;
}

RTCList *rtc_list_copy(const RTCList *list)
{
	RTCList *copy = NULL;
	const RTCList *iter;
	for (iter = list; iter != NULL; iter = rtc_list_next(iter))
	{
		copy = rtc_list_append(copy, iter->data);
	}
	return copy;
}
