#ifndef RTC_LIST_H_
#define RTC_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

	struct _RTCList
	{
		struct _RTCList *next;
		struct _RTCList *prev;
		void *data;
	};

	typedef struct _RTCList RTCList;

#define rtc_list_next(elem) ((elem)->next)
	typedef int (*RTCCompareFunc)(const void *a, const void *b);

	RTCList *rtc_list_append(RTCList *elem, void *data);
	RTCList *rtc_list_append_link(RTCList *elem, RTCList *new_elem);
	RTCList *rtc_list_prepend(RTCList *elem, void *data);
	RTCList *rtc_list_free(RTCList *elem);
	RTCList *rtc_list_concat(RTCList *first, RTCList *second);
	RTCList *rtc_list_remove(RTCList *first, void *data);
	int rtc_list_size(const RTCList *first);
	void rtc_list_for_each(const RTCList *list, void (*func)(void *));
	void rtc_list_for_each2(const RTCList *list, void (*func)(void *, void *), void *user_data);
	void rtc_list_for_each3(const RTCList *list, int (*func)(char *, int, void *, char *), int streamid, void *request, char *endtime, int index); //+++
	void rtc_list_for_each4(const RTCList *list, int (*func)(char *, size_t), char *user_data);//+++
	RTCList *rtc_list_remove_link(RTCList *list, RTCList *elem);
	RTCList *rtc_list_find(RTCList *list, void *data);
	RTCList *rtc_list_find_custom(RTCList *list, int (*compare_func)(const void *, const void *), const void *user_data);
	void *rtc_list_nth_data(const RTCList *list, int index);
	int rtc_list_position(const RTCList *list, RTCList *elem);
	int rtc_list_index(const RTCList *list, void *data);
	RTCList *rtc_list_insert_sorted(RTCList *list, void *data, int (*compare_func)(void *, void *, int), int type);
	RTCList *rtc_list_insert(RTCList *list, RTCList *before, void *data);
	RTCList *rtc_list_copy(const RTCList *list);

#ifdef __cplusplus
}
#endif
#endif
