
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
RTCList *rtc_list_new(void *data){
	RTCList *new_elem=(RTCList *)rtc_bk_malloc(sizeof(RTCList));
	if(new_elem!= NULL){
		new_elem->prev=NULL;
		new_elem->next=NULL;
		new_elem->data=data;
		return new_elem;
	}
	return NULL;
}

RTCList *rtc_list_append_link(RTCList *elem, RTCList *new_elem){
	RTCList *it=elem;
	if (elem==NULL) return new_elem;
	while (it->next!=NULL) it=rtc_list_next(it);
	it->next=new_elem;
	new_elem->prev=it;
	return elem;
}

RTCList * rtc_list_append(RTCList *elem, void * data){
	RTCList *new_elem=rtc_list_new(data);
	return rtc_list_append_link(elem,new_elem);
}

RTCList * rtc_list_prepend(RTCList *elem, void *data){
	RTCList *new_elem=rtc_list_new(data);
	if (elem!=NULL) {
		new_elem->next=elem;
		elem->prev=new_elem;
	}
	return new_elem;
}


RTCList * rtc_list_concat(RTCList *first, RTCList *second){
	RTCList *it=first;
	if (it==NULL) return second;
	while(it->next!=NULL) it=rtc_list_next(it);
	it->next=second;
	second->prev=it;
	return first;
}

RTCList * rtc_list_free(RTCList *list){
	RTCList *elem = list;
	RTCList *tmp = NULL;
	if (list==NULL) return NULL;
	while(elem->next!=NULL) {
		tmp = elem;
		elem = elem->next;
		rtc_bk_free(tmp);
	}
	rtc_bk_free(elem);
	return NULL;
}

RTCList * rtc_list_remove(RTCList *first, void *data){
	RTCList *it = NULL;
	it=rtc_list_find(first,data);
	if (it) return rtc_list_remove_link(first,it);
	else {
		return first;
	}
}

int rtc_list_size(const RTCList *first){
	int n=0;
	RTCList *elem = (RTCList *)first;
	while(elem!=NULL){
		++n;
		elem=elem->next;
	}
	return n;
}

void rtc_list_for_each(const RTCList *list, void (*func)(void *)){
	RTCList *next;
	RTCList *elem = (RTCList *)list;
	while(elem!=NULL){
		next=elem->next;
		func(elem->data);
		elem = next;

	}

}

void rtc_list_for_each2(const RTCList *list, void (*func)(void *, void *), void *user_data){

	RTCList *next = NULL;
	RTCList *elem = (RTCList *)list;
	while(elem!=NULL){
		next=elem->next;
		func(elem->data,user_data);
		elem = next;

	}
}

RTCList *rtc_list_remove_link(RTCList *list, RTCList *elem){
	RTCList *ret = NULL;
	if (elem==list){
		ret=elem->next;
		elem->prev=NULL;
		elem->next=NULL;
		if (ret!=NULL) ret->prev=NULL;
		rtc_bk_free(elem);
		return ret;
	}
	elem->prev->next=elem->next;
	if (elem->next!=NULL) elem->next->prev=elem->prev;
	elem->next=NULL;
	elem->prev=NULL;
	rtc_bk_free(elem);
	return list;
}

RTCList *rtc_list_find(RTCList *list, void *data){

	RTCList *next = NULL;
	RTCList *elem = list;
	while(elem!=NULL){
		next=elem->next;
		if (elem->data==data) return elem;
		elem = next;

	}
	return NULL;
}

RTCList *rtc_list_find_custom(RTCList *list, int (*compare_func)(const void *, const void*), const void *user_data){

	RTCList *next = NULL;
	RTCList *elem = list;
	while(elem!=NULL){
		next=elem->next;
		if (compare_func(elem->data,user_data)==0) return elem;
		elem = next;

	}
	return NULL;
}

void * rtc_list_nth_data(const RTCList *list, int index){
	int i;
	for(i=0;list!=NULL;list=list->next,++i){
		if (i==index) return list->data;
	}
	return NULL;
}

int rtc_list_position(const RTCList *list, RTCList *elem){
	int i;
	for(i=0;list!=NULL;list=list->next,++i){
		if (elem==list) return i;
	}
	return -1;
}

int rtc_list_index(const RTCList *list, void *data){
	int i;
	for(i=0;list!=NULL;list=list->next,++i){
		if (data==list->data) return i;
	}
	return -1;
}

RTCList *rtc_list_insert_sorted(RTCList *list, void *data, int (*compare_func)(const void *, const void*)){
	RTCList *it,*previt=NULL;
	RTCList *nelem = NULL;
	RTCList *ret=list;
	if (list==NULL) return rtc_list_append(list,data);
	else{
		nelem=rtc_list_new(data);
		for(it=list;it!=NULL;it=it->next){
			previt=it;
			if (compare_func(data,it->data)<=0){
				nelem->prev=it->prev;
				nelem->next=it;
				if (it->prev!=NULL)
					it->prev->next=nelem;
				else{
					ret=nelem;
				}
				it->prev=nelem;
				return ret;
			}
		}
		previt->next=nelem;
		nelem->prev=previt;
	}
	return ret;
}

RTCList *rtc_list_insert(RTCList *list, RTCList *before, void *data){
	RTCList *elem = NULL;
	if (list==NULL || before==NULL) return rtc_list_append(list,data);
	for(elem=list;elem!=NULL;elem=rtc_list_next(elem)){
		if (elem==before){
			if (elem->prev==NULL)
				return rtc_list_prepend(list,data);
			else{
				RTCList *nelem=rtc_list_new(data);
				nelem->prev=elem->prev;
				nelem->next=elem;
				elem->prev->next=nelem;
				elem->prev=nelem;
			}
		}
	}
	return list;
}

RTCList *rtc_list_copy(const RTCList *list){
	RTCList *copy=NULL;
	const RTCList *iter;
	for(iter=list;iter!=NULL;iter=rtc_list_next(iter)){
		copy=rtc_list_append(copy,iter->data);
	}
	return copy;
}
