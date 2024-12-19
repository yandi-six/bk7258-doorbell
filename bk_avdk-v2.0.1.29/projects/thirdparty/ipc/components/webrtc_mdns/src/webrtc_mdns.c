#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <components/log.h>
#include "components/netif.h"
#include <lwip/sockets.h>
#include <lwip/dns.h>
#include <lwip/tcpip.h>
#include <lwip/netif.h>
#include "lwip/netdb.h"
#include "rtc_bk.h"
#include "webrtc_mdns.h"
#include "cJSON.h"


#define TAG "webrtc_mdns"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)

static webrtc_mdns_callback_fn g_callback = NULL;
static void *g_user = NULL;
static const uint8_t webrtc_mdns_services_query[] = {
    // Query ID
    0x00, 0x00,
    // Flags
    0x00, 0x00,
    // 1 question
    0x00, 0x01,
    // No answer, authority or additional RRs
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // _services._dns-sd._udp.local.
    0x09, '_', 's', 'e', 'r', 'v', 'i', 'c', 'e', 's', 0x07, '_', 'd', 'n', 's', '-', 's', 'd',
    0x04, '_', 'u', 'd', 'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00,
    // PTR record
    0x00, MDNS_RECORDTYPE_PTR,
    // QU (unicast response) and class IN
    0x80, MDNS_CLASS_IN};
int webrtc_mdns_discovery_send(int sock) {
    return webrtc_mdns_multicast_send(sock, webrtc_mdns_services_query, sizeof(webrtc_mdns_services_query));
}

static uint16_t
webrtc_mdns_ntohs(const void* data) {
	uint16_t aligned;
    os_memcpy((void*)&aligned, (void*)data, sizeof(uint16_t));
	return ntohs(aligned);
}

static uint32_t
webrtc_mdns_ntohl(const void* data) {
	uint32_t aligned;
    os_memcpy((void*)&aligned, (void*)data, sizeof(uint32_t));
	return ntohl(aligned);
}

static void*
webrtc_mdns_htons(void* data, uint16_t val) {
	val = htons(val);
    os_memcpy((void*)data, (void*)&val, sizeof(uint16_t));
	return MDNS_POINTER_OFFSET(data, sizeof(uint16_t));
}

static void*
webrtc_mdns_htonl(void* data, uint32_t val) {
	val = htonl(val);
    os_memcpy((void*)data, (void*)&val, sizeof(uint32_t));
	return MDNS_POINTER_OFFSET(data, sizeof(uint32_t));
}
char * webrtc_mdns_inet_ntoa(struct sockaddr *addr, int addrlen, char *dest, int destlen){
    struct sockaddr_in *aaddr  = (struct sockaddr_in *)addr;
	struct in_addr inaddr  = aaddr->sin_addr;
    snprintf(dest,destlen,"%s",inet_ntoa(inaddr));
	return dest;
}
char * webrtc_mdns_inet_ntoa_port(struct sockaddr *addr, int addrlen, char *dest, int destlen){
        struct sockaddr_in *aaddr  = (struct sockaddr_in *)addr;
	struct in_addr inaddr  = aaddr->sin_addr;
        snprintf(dest,destlen,"%s:%d",inet_ntoa(inaddr),ntohs(aaddr->sin_port));
	return dest;
}
static webrtc_mdns_string_t
ipv4_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in* addr,
                       size_t addrlen) {
	
        webrtc_mdns_inet_ntoa_port((struct sockaddr *)addr, addrlen, buffer, capacity);
	webrtc_mdns_string_t str;
	str.str = buffer;
	str.length = strlen(buffer);
        return str;

}
#ifdef WEBRTC_ENABLE_IPV6
static webrtc_mdns_string_t
ipv6_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in6* addr,
                       size_t addrlen) {
	char host[NI_MAXHOST] = {0};
	char service[NI_MAXSERV] = {0};
	int ret = getnameinfo((const struct sockaddr*)addr, (socklen_t)addrlen, host, NI_MAXHOST,
	                      service, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
	int len = 0;
	if (ret == 0) {
		if (addr->sin6_port != 0)
			len = snprintf(buffer, capacity, "[%s]:%s", host, service);
		else
			len = snprintf(buffer, capacity, "%s", host);
	}
	if (len >= (int)capacity)
		len = (int)capacity - 1;
	webrtc_mdns_string_t str;
	str.str = buffer;
	str.length = len;
	return str;
}
#endif
static webrtc_mdns_string_t
ip_address_to_string(char* buffer, size_t capacity, const struct sockaddr* addr, size_t addrlen) {
#ifdef WEBRTC_ENABLE_IPV6
	if (addr->sa_family == AF_INET6)
		return ipv6_address_to_string(buffer, capacity, (const struct sockaddr_in6*)addr, addrlen);
#else
	return ipv4_address_to_string(buffer, capacity, (const struct sockaddr_in*)addr, addrlen);
#endif
}
webrtc_mdns_string_t webrtc_ip_address_to_string(char* buffer, size_t capacity, const struct sockaddr* addr, size_t addrlen){
    return ip_address_to_string(buffer,capacity,addr,addrlen);
}
static int
webrtc_mdns_is_string_ref(uint8_t val) {
	return (0xC0 == (val & 0xC0));
}

static webrtc_mdns_string_pair_t
webrtc_mdns_get_next_substring(const void* rawdata, size_t size, size_t offset) {
	const uint8_t* buffer = (const uint8_t*)rawdata;
	webrtc_mdns_string_pair_t pair = {MDNS_INVALID_POS, 0, 0};
	if (offset >= size)
		return pair;
	if (!buffer[offset]) {
		pair.offset = offset;
		return pair;
	}
	int recursion = 0;
	while (webrtc_mdns_is_string_ref(buffer[offset])) {
		if (size < offset + 2)
			return pair;

		offset = webrtc_mdns_ntohs(MDNS_POINTER_OFFSET(buffer, offset)) & 0x3fff;
		if (offset >= size)
			return pair;

		pair.ref = 1;
		if (++recursion > 16)
			return pair;
	}

	size_t length = (size_t)buffer[offset++];
	if (size < offset + length)
		return pair;

	pair.offset = offset;
	pair.length = length;

	return pair;
}

static int
webrtc_mdns_string_skip(const void* buffer, size_t size, size_t* offset) {
	size_t cur = *offset;
	webrtc_mdns_string_pair_t substr;
    os_memset(&substr,0,sizeof(webrtc_mdns_string_pair_t));
	unsigned int counter = 0;
	do {
		substr = webrtc_mdns_get_next_substring(buffer, size, cur);
		if ((substr.offset == MDNS_INVALID_POS) || (counter++ > MDNS_MAX_SUBSTRINGS))
			return 0;
		if (substr.ref) {
			*offset = cur + 2;
			return 1;
		}
		cur = substr.offset + substr.length;
	} while (substr.length);

	*offset = cur + 1;
	return 1;
}

static int
webrtc_mdns_string_equal(const void* buffer_lhs, size_t size_lhs, size_t* ofs_lhs, const void* buffer_rhs,
                  size_t size_rhs, size_t* ofs_rhs) {
	size_t lhs_cur = *ofs_lhs;
	size_t rhs_cur = *ofs_rhs;
	size_t lhs_end = MDNS_INVALID_POS;
	size_t rhs_end = MDNS_INVALID_POS;
	webrtc_mdns_string_pair_t lhs_substr;
	webrtc_mdns_string_pair_t rhs_substr;
    os_memset(&lhs_substr,0,sizeof(webrtc_mdns_string_pair_t));
    os_memset(&rhs_substr,0,sizeof(webrtc_mdns_string_pair_t));
	unsigned int counter = 0;
	do {
		lhs_substr = webrtc_mdns_get_next_substring(buffer_lhs, size_lhs, lhs_cur);
		rhs_substr = webrtc_mdns_get_next_substring(buffer_rhs, size_rhs, rhs_cur);
		if ((lhs_substr.offset == MDNS_INVALID_POS) || (rhs_substr.offset == MDNS_INVALID_POS) ||
		    (counter++ > MDNS_MAX_SUBSTRINGS))
			return 0;
		if (lhs_substr.length != rhs_substr.length)
			return 0;
		if (strncasecmp((const char*)MDNS_POINTER_OFFSET_CONST(buffer_rhs, rhs_substr.offset),
		                (const char*)MDNS_POINTER_OFFSET_CONST(buffer_lhs, lhs_substr.offset),
		                rhs_substr.length))
			return 0;
		if (lhs_substr.ref && (lhs_end == MDNS_INVALID_POS))
			lhs_end = lhs_cur + 2;
		if (rhs_substr.ref && (rhs_end == MDNS_INVALID_POS))
			rhs_end = rhs_cur + 2;
		lhs_cur = lhs_substr.offset + lhs_substr.length;
		rhs_cur = rhs_substr.offset + rhs_substr.length;
	} while (lhs_substr.length);

	if (lhs_end == MDNS_INVALID_POS)
		lhs_end = lhs_cur + 1;
	*ofs_lhs = lhs_end;

	if (rhs_end == MDNS_INVALID_POS)
		rhs_end = rhs_cur + 1;
	*ofs_rhs = rhs_end;

	return 1;
}
static size_t
webrtc_mdns_string_find(const char* str, size_t length, char c, size_t offset) {
	const void* found;
	if (offset >= length)
		return MDNS_INVALID_POS;
	found = memchr(str + offset, c, length - offset);
	if (found)
		return (size_t)MDNS_POINTER_DIFF(found, str);
	return MDNS_INVALID_POS;
}
static webrtc_mdns_string_t
webrtc_mdns_string_extract(const void* buffer, size_t size, size_t* offset, char* str, size_t capacity) {
	size_t cur = *offset;
	size_t end = MDNS_INVALID_POS;
	webrtc_mdns_string_pair_t substr;
    os_memset(&substr,0,sizeof(webrtc_mdns_string_pair_t));
	webrtc_mdns_string_t result;
	result.str = str;
	result.length = 0;
	char* dst = str;
	unsigned int counter = 0;
	size_t remain = capacity;
	do {
		substr = webrtc_mdns_get_next_substring(buffer, size, cur);
		if ((substr.offset == MDNS_INVALID_POS) || (counter++ > MDNS_MAX_SUBSTRINGS))
			return result;
		if (substr.ref && (end == MDNS_INVALID_POS))
			end = cur + 2;
		if (substr.length) {
			size_t to_copy = (substr.length < remain) ? substr.length : remain;
            os_memcpy(dst, (char*)buffer + substr.offset, to_copy);
			dst += to_copy;
			remain -= to_copy;
			if (remain) {
				*dst++ = '.';
				--remain;
			}
		}
		cur = substr.offset + substr.length;
	} while (substr.length);

	if (end == MDNS_INVALID_POS)
		end = cur + 1;
	*offset = end;

	result.length = capacity - remain;
	return result;
}

static size_t
webrtc_mdns_string_table_find(webrtc_mdns_string_table_t* string_table, const void* buffer, size_t capacity,
                       const char* str, size_t first_length, size_t total_length) {
	if (!string_table)
		return MDNS_INVALID_POS;
        size_t istr = 0;
	for (istr = 0; istr < string_table->count; ++istr) {
		if (string_table->offset[istr] >= capacity)
			continue;
		size_t offset = 0;
		webrtc_mdns_string_pair_t sub_string =
		    webrtc_mdns_get_next_substring(buffer, capacity, string_table->offset[istr]);
		if (!sub_string.length || (sub_string.length != first_length))
			continue;
		if (memcmp(str, MDNS_POINTER_OFFSET(buffer, sub_string.offset), sub_string.length))
			continue;

		// Initial substring matches, now match all remaining substrings
		offset += first_length + 1;
		while (offset < total_length) {
			size_t dot_pos = webrtc_mdns_string_find(str, total_length, '.', offset);
			if (dot_pos == MDNS_INVALID_POS)
				dot_pos = total_length;
			size_t current_length = dot_pos - offset;

			sub_string =
			    webrtc_mdns_get_next_substring(buffer, capacity, sub_string.offset + sub_string.length);
			if (!sub_string.length || (sub_string.length != current_length))
				break;
			if (memcmp(str + offset, MDNS_POINTER_OFFSET(buffer, sub_string.offset),
			           sub_string.length))
				break;

			offset = dot_pos + 1;
		}

		// Return reference offset if entire string matches
		if (offset >= total_length)
			return string_table->offset[istr];
	}

	return MDNS_INVALID_POS;
}

static void
webrtc_mdns_string_table_add(webrtc_mdns_string_table_t* string_table, size_t offset) {
	if (!string_table)
		return;

	string_table->offset[string_table->next] = offset;

	size_t table_capacity = sizeof(string_table->offset) / sizeof(string_table->offset[0]);
	if (++string_table->count > table_capacity)
		string_table->count = table_capacity;
	if (++string_table->next >= table_capacity)
		string_table->next = 0;
}

static void*
webrtc_mdns_string_make_ref(void* data, size_t capacity, size_t ref_offset) {
	if (capacity < 2)
		return 0;
	return webrtc_mdns_htons(data, 0xC000 | (uint16_t)ref_offset);
}

static void*
webrtc_mdns_string_make(void* buffer, size_t capacity, void* data, const char* name, size_t length,
                 webrtc_mdns_string_table_t* string_table) {
	size_t last_pos = 0;
	size_t remain = capacity - MDNS_POINTER_DIFF(data, buffer);
	if (name[length - 1] == '.')
		--length;
	while (last_pos < length) {
		size_t pos = webrtc_mdns_string_find(name, length, '.', last_pos);
		size_t sub_length = ((pos != MDNS_INVALID_POS) ? pos : length) - last_pos;
		size_t total_length = length - last_pos;

		size_t ref_offset =
		    webrtc_mdns_string_table_find(string_table, buffer, capacity,
		                           (char*)MDNS_POINTER_OFFSET(name, last_pos), sub_length,
		                           total_length);
		if (ref_offset != MDNS_INVALID_POS)
			return webrtc_mdns_string_make_ref(data, remain, ref_offset);

		if (remain <= (sub_length + 1))
			return 0;

		*(unsigned char*)data = (unsigned char)sub_length;
        os_memcpy((char*)MDNS_POINTER_OFFSET(data, 1), (char*)(name + last_pos), sub_length);
		webrtc_mdns_string_table_add(string_table, MDNS_POINTER_DIFF(data, buffer));

		data = MDNS_POINTER_OFFSET(data, sub_length + 1);
		last_pos = ((pos != MDNS_INVALID_POS) ? pos + 1 : length);
		remain = capacity - MDNS_POINTER_DIFF(data, buffer);
	}

	if (!remain)
		return 0;

	*(unsigned char*)data = 0;
	return MDNS_POINTER_OFFSET(data, 1);
}





size_t webrtc_mdns_records_parse(int sock, const struct sockaddr* from, size_t addrlen, const void* buffer,
                   size_t size, size_t* offset, webrtc_mdns_entry_type_t type, uint16_t query_id,
                   size_t records, webrtc_mdns_record_callback_fn callback, void* user_data) {
        
	size_t parsed = 0;
	size_t i = 0;
	for ( i = 0; i < records; ++i) {
		size_t name_offset = *offset;
		webrtc_mdns_string_skip(buffer, size, offset);
		if (((*offset) + 10) > size)
			return parsed;
		size_t name_length = (*offset) - name_offset;
		const uint16_t* data = (const uint16_t*)MDNS_POINTER_OFFSET(buffer, *offset);

		uint16_t rtype = webrtc_mdns_ntohs(data++);
		uint16_t rclass = webrtc_mdns_ntohs(data++);
		uint32_t ttl = webrtc_mdns_ntohl(data);
		data += 2;
		uint16_t length = webrtc_mdns_ntohs(data++);

		*offset += 10;

		if (length <= (size - (*offset))) {
			++parsed;
			if (callback &&
			    callback(sock, from, addrlen, type, query_id, rtype, rclass, ttl, buffer, size,
			             name_offset, name_length, *offset, length, user_data))
				break;
		}

		*offset += length;
	}
	return parsed;
}


static void*
webrtc_mdns_answer_add_question_unicast(void* buffer, size_t capacity, void* data,
                                 webrtc_mdns_record_type_t record_type, const char* name,
                                 size_t name_length, webrtc_mdns_string_table_t* string_table) {
	data = webrtc_mdns_string_make(buffer, capacity, data, name, name_length, string_table);
	if (!data)
		return 0;
	size_t remain = capacity - MDNS_POINTER_DIFF(data, buffer);
	if (remain < 4)
		return 0;

	data = webrtc_mdns_htons(data, record_type);
	data = webrtc_mdns_htons(data, MDNS_UNICAST_RESPONSE | MDNS_CLASS_IN);

	return data;
}

static void*
webrtc_mdns_answer_add_record_header(void* buffer, size_t capacity, void* data, webrtc_mdns_record_t record,
                              uint16_t rclass, uint32_t ttl, webrtc_mdns_string_table_t* string_table) {
	data = webrtc_mdns_string_make(buffer, capacity, data, record.name.str, record.name.length, string_table);
	if (!data)
		return 0;
	size_t remain = capacity - MDNS_POINTER_DIFF(data, buffer);
	if (remain < 10)
		return 0;

	data = webrtc_mdns_htons(data, record.type);
	data = webrtc_mdns_htons(data, rclass);
	data = webrtc_mdns_htonl(data, ttl);
	data = webrtc_mdns_htons(data, 0);  // Length, to be filled later
	return data;
}

static void*
webrtc_mdns_answer_add_record(void* buffer, size_t capacity, void* data, webrtc_mdns_record_t record,
                       uint16_t rclass, uint32_t ttl, webrtc_mdns_string_table_t* string_table) {
	// TXT records will be coalesced into one record later
	if (!data || (record.type == MDNS_RECORDTYPE_TXT))
		return data;

	data = webrtc_mdns_answer_add_record_header(buffer, capacity, data, record, rclass, ttl, string_table);
	if (!data)
		return 0;

	// Pointer to length of record to be filled at end
	void* record_length = MDNS_POINTER_OFFSET(data, -2);
	void* record_data = data;

	size_t remain = capacity - MDNS_POINTER_DIFF(data, buffer);
	switch (record.type) {
		case MDNS_RECORDTYPE_PTR:
			data = webrtc_mdns_string_make(buffer, capacity, data, record.data.ptr.name.str,
			                        record.data.ptr.name.length, string_table);
			break;

		case MDNS_RECORDTYPE_SRV:
			if (remain <= 6)
				return 0;
			data = webrtc_mdns_htons(data, record.data.srv.priority);
			data = webrtc_mdns_htons(data, record.data.srv.weight);
			data = webrtc_mdns_htons(data, record.data.srv.port);
			data = webrtc_mdns_string_make(buffer, capacity, data, record.data.srv.name.str,
			                        record.data.srv.name.length, string_table);
			break;

		case MDNS_RECORDTYPE_A:
			if (remain < 4)
				return 0;
            os_memcpy(data, &record.data.a.addr.sin_addr.s_addr, 4);
			data = MDNS_POINTER_OFFSET(data, 4);
			break;

		case MDNS_RECORDTYPE_AAAA:
			if (remain < 16)
				return 0;
#ifdef WEBRTC_ENABLE_IPV6
            os_memcpy(data, &record.data.aaaa.addr.sin6_addr, 16);  // ipv6 address
#endif
			data = MDNS_POINTER_OFFSET(data, 16);
			break;

		default:
			break;
	}

	if (!data)
		return 0;

	// Fill record length
	webrtc_mdns_htons(record_length, (uint16_t)MDNS_POINTER_DIFF(data, record_data));
	return data;
}

static void*
webrtc_mdns_answer_add_txt_record(void* buffer, size_t capacity, void* data, webrtc_mdns_record_t* records,
                           size_t record_count, uint16_t rclass, uint32_t ttl,
                           webrtc_mdns_string_table_t* string_table) {
	// Pointer to length of record to be filled at end
	void* record_length = 0;
	void* record_data = 0;
        size_t length = 0;

	size_t remain = 0;
	size_t irec = 0;
	for ( irec = 0; data && (irec < record_count); ++irec) {
		if (records[irec].type != MDNS_RECORDTYPE_TXT)
			continue;

		if (!record_data) {
			data = webrtc_mdns_answer_add_record_header(buffer, capacity, data, records[irec], rclass, ttl,
			                                     string_table);
			record_length = MDNS_POINTER_OFFSET(data, -2);
			record_data = data;
		}

		// TXT strings are unlikely to be shared, just make then raw. Also need one byte for
		// termination, thus the <= check
		size_t string_length =
		    records[irec].data.txt.key.length + records[irec].data.txt.value.length + 1;
		if (!data)
			return 0;
		remain = capacity - MDNS_POINTER_DIFF(data, buffer);
		if ((remain <= string_length) || (string_length > 0x3FFF))
			return 0;
		


		unsigned char* strdata = (unsigned char*)data;
		*strdata++ = (unsigned char)string_length;
		length = strlen((char*)records[irec].data.txt.key.str);
        //printf("webrtc_mdns_answer_add_txt_record -------  %d     %d\n",length,records[irec].data.txt.key.length);
                //snprintf((char*)strdata,records[irec].data.txt.key.length,"%s",(char*)records[irec].data.txt.key.str);
        os_memcpy((char*)strdata, (char*)records[irec].data.txt.key.str, records[irec].data.txt.key.length);
		strdata += records[irec].data.txt.key.length;
		*strdata++ = '=';
		//length = strlen((char*)records[irec].data.txt.value.str);
		//snprintf((char*)strdata,records[irec].data.txt.value.length,"%s",(char*)records[irec].data.txt.value.str);
        os_memcpy((char*)strdata, (char*)records[irec].data.txt.value.str, records[irec].data.txt.value.length);
		strdata += records[irec].data.txt.value.length;

		data = strdata;
	}

	// Fill record length
	if (record_data)
		webrtc_mdns_htons(record_length, (uint16_t)MDNS_POINTER_DIFF(data, record_data));

	return data;
}

static uint16_t
webrtc_mdns_answer_get_record_count(webrtc_mdns_record_t* records, size_t record_count) {
	// TXT records will be coalesced into one record
	uint16_t total_count = 0;
	uint16_t txt_record = 0;
	size_t irec = 0; 
	for ( irec = 0; irec < record_count; ++irec) {
		if (records[irec].type == MDNS_RECORDTYPE_TXT)
			txt_record = 1;
		else
			++total_count;
	}
	return total_count + txt_record;
}

static int
webrtc_mdns_query_answer_unicast(int sock, const void* address, size_t address_size, void* buffer,
                          size_t capacity, uint16_t query_id, webrtc_mdns_record_type_t record_type,
                          const char* name, size_t name_length, webrtc_mdns_record_t answer,
                          webrtc_mdns_record_t* authority, size_t authority_count,
                          webrtc_mdns_record_t* additional, size_t additional_count) {
	if (capacity < (sizeof(struct webrtc_mdns_header_t) + 32 + 4))
		return -1;

	uint16_t rclass = MDNS_CACHE_FLUSH | MDNS_CLASS_IN;
	uint32_t ttl = 10;
	size_t irec = 0;

	// Basic answer structure
	struct webrtc_mdns_header_t* header = (struct webrtc_mdns_header_t*)buffer;
	header->query_id = htons(query_id);
	header->flags = htons(0x8400);
	header->questions = htons(1);
	header->answer_rrs = htons(1);
	header->authority_rrs = htons(webrtc_mdns_answer_get_record_count(authority, authority_count));
	header->additional_rrs = htons(webrtc_mdns_answer_get_record_count(additional, additional_count));

	webrtc_mdns_string_table_t string_table;
    os_memset(&string_table,0,sizeof(webrtc_mdns_string_table_t));
	void* data = MDNS_POINTER_OFFSET(buffer, sizeof(struct webrtc_mdns_header_t));

	// Fill in question
	data = webrtc_mdns_answer_add_question_unicast(buffer, capacity, data, record_type, name, name_length,
	                                        &string_table);

	// Fill in answer
	data = webrtc_mdns_answer_add_record(buffer, capacity, data, answer, rclass, ttl, &string_table);

	// Fill in authority records
	for ( irec = 0; data && (irec < authority_count); ++irec)
		data = webrtc_mdns_answer_add_record(buffer, capacity, data, authority[irec], rclass, ttl,
		                              &string_table);
	data = webrtc_mdns_answer_add_txt_record(buffer, capacity, data, authority, authority_count, rclass,
	                                  ttl, &string_table);

	// Fill in additional records
	for ( irec = 0; data && (irec < additional_count); ++irec)
		data = webrtc_mdns_answer_add_record(buffer, capacity, data, additional[irec], rclass, ttl,
		                              &string_table);
	data = webrtc_mdns_answer_add_txt_record(buffer, capacity, data, additional, additional_count, rclass,
	                                  ttl, &string_table);
	if (!data)
		return -1;

	size_t tosend = MDNS_POINTER_DIFF(data, buffer);
	return webrtc_mdns_unicast_send(sock, address, address_size, buffer, tosend);
}


static int
webrtc_mdns_answer_multicast_rclass_ttl(int sock, void* buffer, size_t capacity, uint16_t rclass,
                             webrtc_mdns_record_t answer, webrtc_mdns_record_t* authority, size_t authority_count,
                             webrtc_mdns_record_t* additional, size_t additional_count, uint32_t ttl) {
	if (capacity < (sizeof(struct webrtc_mdns_header_t) + 32 + 4))
		return -1;
        size_t irec = 0;
	// Basic answer structure
	struct webrtc_mdns_header_t* header = (struct webrtc_mdns_header_t*)buffer;
	header->query_id = 0;
	header->flags = htons(0x8400);
	header->questions = 0;
	header->answer_rrs = htons(1);
	header->authority_rrs = htons(webrtc_mdns_answer_get_record_count(authority, authority_count));
	header->additional_rrs = htons(webrtc_mdns_answer_get_record_count(additional, additional_count));

	webrtc_mdns_string_table_t string_table;
        os_memset(&string_table,0,sizeof(webrtc_mdns_string_table_t));
	void* data = MDNS_POINTER_OFFSET(buffer, sizeof(struct webrtc_mdns_header_t));

	// Fill in answer
	data = webrtc_mdns_answer_add_record(buffer, capacity, data, answer, rclass, ttl, &string_table);

	// Fill in authority records
	for ( irec = 0; data && (irec < authority_count); ++irec)
		data = webrtc_mdns_answer_add_record(buffer, capacity, data, authority[irec], rclass, ttl,
		                              &string_table);
	data = webrtc_mdns_answer_add_txt_record(buffer, capacity, data, authority, authority_count, rclass,
	                                  ttl, &string_table);

	// Fill in additional records
	for ( irec = 0; data && (irec < additional_count); ++irec)
		data = webrtc_mdns_answer_add_record(buffer, capacity, data, additional[irec], rclass, ttl,
		                              &string_table);
	data = webrtc_mdns_answer_add_txt_record(buffer, capacity, data, additional, additional_count, rclass,
	                                  ttl, &string_table);
	if (!data)
		return -1;

	size_t tosend = MDNS_POINTER_DIFF(data, buffer);
	return webrtc_mdns_multicast_send(sock, buffer, tosend);
}

static int
webrtc_mdns_answer_multicast_rclass(int sock, void* buffer, size_t capacity, uint16_t rclass,
                             webrtc_mdns_record_t answer, webrtc_mdns_record_t* authority, size_t authority_count,
                             webrtc_mdns_record_t* additional, size_t additional_count) {
	return webrtc_mdns_answer_multicast_rclass_ttl(sock, buffer, capacity, rclass, answer, authority,
	                                        authority_count, additional, additional_count, 60);
}

int webrtc_mdns_query_answer_multicast(int sock, void* buffer, size_t capacity, webrtc_mdns_record_t answer,
                            webrtc_mdns_record_t* authority, size_t authority_count,
                            webrtc_mdns_record_t* additional, size_t additional_count) {
	uint16_t rclass = MDNS_CLASS_IN;
	return webrtc_mdns_answer_multicast_rclass(sock, buffer, capacity, rclass, answer, authority,
	                                    authority_count, additional, additional_count);
}

int webrtc_mdns_announce_multicast(int sock, void* buffer, size_t capacity, webrtc_mdns_record_t answer,
                        webrtc_mdns_record_t* authority, size_t authority_count, webrtc_mdns_record_t* additional,
                        size_t additional_count) {
	uint16_t rclass = MDNS_CLASS_IN | MDNS_CACHE_FLUSH;
	return webrtc_mdns_answer_multicast_rclass(sock, buffer, capacity, rclass, answer, authority,
	                                    authority_count, additional, additional_count);
}

int webrtc_mdns_goodbye_multicast(int sock, void* buffer, size_t capacity, webrtc_mdns_record_t answer,
                        webrtc_mdns_record_t* authority, size_t authority_count, webrtc_mdns_record_t* additional,
                        size_t additional_count) {
	uint16_t rclass = MDNS_CLASS_IN | MDNS_CACHE_FLUSH;
	return webrtc_mdns_answer_multicast_rclass_ttl(sock, buffer, capacity, rclass, answer, authority,
	                                    authority_count, additional, additional_count, 0);
}

static webrtc_mdns_string_t
webrtc_mdns_record_parse_ptr(const void* buffer, size_t size, size_t offset, size_t length,
                      char* strbuffer, size_t capacity) {
	// PTR record is just a string
	if ((size >= offset + length) && (length >= 2))
		return webrtc_mdns_string_extract(buffer, size, &offset, strbuffer, capacity);
	webrtc_mdns_string_t empty = {0, 0};
	return empty;
}

static webrtc_mdns_record_srv_t
webrtc_mdns_record_parse_srv(const void* buffer, size_t size, size_t offset, size_t length,
                      char* strbuffer, size_t capacity) {
	webrtc_mdns_record_srv_t srv;
	os_memset(&srv, 0, sizeof(webrtc_mdns_record_srv_t));
	// Read the service priority, weight, port number and the discovery name
	// SRV record format (http://www.ietf.org/rfc/rfc2782.txt):
	// 2 bytes network-order unsigned priority
	// 2 bytes network-order unsigned weight
	// 2 bytes network-order unsigned port
	// string: discovery (domain) name, minimum 2 bytes when compressed
	if ((size >= offset + length) && (length >= 8)) {
		const uint16_t* recorddata = (const uint16_t*)MDNS_POINTER_OFFSET_CONST(buffer, offset);
		srv.priority = webrtc_mdns_ntohs(recorddata++);
		srv.weight = webrtc_mdns_ntohs(recorddata++);
		srv.port = webrtc_mdns_ntohs(recorddata++);
		offset += 6;
		srv.name = webrtc_mdns_string_extract(buffer, size, &offset, strbuffer, capacity);
	}
	return srv;
}

static struct sockaddr_in*
webrtc_mdns_record_parse_a(const void* buffer, size_t size, size_t offset, size_t length,
                    struct sockaddr_in* addr) {
    os_memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
#ifdef __APPLE__
	addr->sin_len = sizeof(struct sockaddr_in);
#endif
	if ((size >= offset + length) && (length == 4))
        os_memcpy(&addr->sin_addr.s_addr, MDNS_POINTER_OFFSET(buffer, offset), 4);
	return addr;
}
#ifdef WEBRTC_ENABLE_IPV6
static struct sockaddr_in6*
webrtc_mdns_record_parse_aaaa(const void* buffer, size_t size, size_t offset, size_t length,
                       struct sockaddr_in6* addr) {
    os_memset(addr, 0, sizeof(struct sockaddr_in6));
	addr->sin6_family = AF_INET6;
#ifdef __APPLE__
	addr->sin6_len = sizeof(struct sockaddr_in6);
#endif
	if ((size >= offset + length) && (length == 16))
        os_memcpy(&addr->sin6_addr, MDNS_POINTER_OFFSET(buffer, offset), 16);
	return addr;
}
#endif
static size_t
webrtc_mdns_record_parse_txt(const void* buffer, size_t size, size_t offset, size_t length,
                      webrtc_mdns_record_txt_t* records, size_t capacity) {
    size_t parsed = 0;
    const char* strdata = NULL;
    size_t end = offset + length;
        size_t c = 0;
    if (size < end)
        end = size;

    while ((offset < end) && (parsed < capacity)) {
        strdata = (const char*)MDNS_POINTER_OFFSET(buffer, offset);
        size_t sublength = *(const unsigned char*)strdata;

        ++strdata;
        offset += sublength + 1;

        size_t separator = 0;
        for ( c = 0; c < sublength; ++c) {
            // DNS-SD TXT record keys MUST be printable US-ASCII, [0x20, 0x7E]
            if ((strdata[c] < 0x20) || (strdata[c] > 0x7E))
                break;
            if (strdata[c] == '=') {
                separator = c;
                break;
            }
        }

        if (!separator)
            continue;

        if (separator < sublength) {
            records[parsed].key.str = strdata;
            records[parsed].key.length = separator;
            records[parsed].value.str = strdata + separator + 1;
            records[parsed].value.length = sublength - (separator + 1);
        } else {
            records[parsed].key.str = strdata;
            records[parsed].key.length = sublength;
        }

        ++parsed;
    }

    return parsed;
}
int webrtc_mdns_unicast_send(int sock, const void* address, size_t address_size, const void* buffer,
                  size_t size) {
    struct sockaddr_in *aaddr  = (struct sockaddr_in *)address;
    struct in_addr inaddr  = aaddr->sin_addr;
     LOGD("webrtc_mdns_unicast_send sendto>>>>>>>>>  %s:%d\n",inet_ntoa(inaddr),ntohs(aaddr->sin_port));
	if (sendto(sock, (const char*)buffer, (webrtc_mdns_size_t)size, 0, (const struct sockaddr*)address,
	           (socklen_t)address_size) < 0)
		return -1;
	return 0;
}

int webrtc_mdns_multicast_send(int sock, const void* buffer, size_t size) {
	struct sockaddr_storage addr_storage;
	struct sockaddr_in addr;
#ifdef WEBRTC_ENABLE_IPV6
	struct sockaddr_in6 addr6;
#endif
	struct sockaddr* saddr = (struct sockaddr*)&addr_storage;
	socklen_t saddrlen = sizeof(struct sockaddr_storage);
	if (getsockname(sock, saddr, &saddrlen))
		return -1;
#ifdef WEBRTC_ENABLE_IPV6
	if (saddr->sa_family == AF_INET6) {
        os_memset(&addr6, 0, sizeof(addr6));
		addr6.sin6_family = AF_INET6;
#ifdef __APPLE__
		addr6.sin6_len = sizeof(addr6);
#endif
		addr6.sin6_addr.s6_addr[0] = 0xFF;
		addr6.sin6_addr.s6_addr[1] = 0x02;
		addr6.sin6_addr.s6_addr[15] = 0xFB;
		addr6.sin6_port = htons((unsigned short)MDNS_PORT);
		saddr = (struct sockaddr*)&addr6;
		saddrlen = sizeof(addr6);
	} else {
#endif
        os_memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl((((uint32_t)224U) << 24U) | ((uint32_t)251U));
        addr.sin_port = htons((unsigned short)MDNS_PORT);
        saddr = (struct sockaddr*)&addr;
        saddrlen = sizeof(addr);
#ifdef WEBRTC_ENABLE_IPV6
	}
#endif

        LOGD("webrtc_mdns_multicast_send sendto ------------>>> %d \n",size);
	if (sendto(sock, (const char*)buffer, (webrtc_mdns_size_t)size, 0, saddr, saddrlen) < 0)
		return -1;
	return 0;
}
static int
webrtc_mdns_discovery_query_send(int sock, webrtc_mdns_record_type_t type, const char* name, size_t length, void* buffer,
                size_t capacity, uint16_t query_id,const void* address, size_t address_size) {
    if (capacity < (17 + length))
        return -1;
     // LOGD(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>webrtc_mdns_query_send <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    // Ask for a unicast response since it's a one-shot query
    uint16_t rclass = MDNS_CLASS_IN | MDNS_UNICAST_RESPONSE;


    struct webrtc_mdns_header_t* header = (struct webrtc_mdns_header_t*)buffer;
    // Query ID
    header->query_id = htons(query_id);
    // Flags
    header->flags = 0;
    // Questions
    header->questions = htons(1);
    // No answer, authority or additional RRs
    header->answer_rrs = 0;
    header->authority_rrs = 0;
    header->additional_rrs = 0;
    // Fill in question
    // Name string
    void* data = MDNS_POINTER_OFFSET(buffer, sizeof(struct webrtc_mdns_header_t));
    data = webrtc_mdns_string_make(buffer, capacity, data, name, length, 0);
    if (!data)
        return -1;
    // Record type
    data = webrtc_mdns_htons(data, type);
    //! Optional unicast response based on local port, class IN
    data = webrtc_mdns_htons(data, rclass);

    size_t tosend = MDNS_POINTER_DIFF(data, buffer);
    if (webrtc_mdns_unicast_send(sock, address, address_size,buffer, (size_t)tosend))
        return -1;
    return query_id;
}

static int
webrtc_mdns_query_send(int sock, webrtc_mdns_record_type_t type, const char* name, size_t length, void* buffer,
                size_t capacity, uint16_t query_id) {
	if (capacity < (17 + length))
		return -1;
         //LOGD(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>webrtc_mdns_query_send <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	// Ask for a unicast response since it's a one-shot query
	uint16_t rclass = MDNS_CLASS_IN | MDNS_UNICAST_RESPONSE;

	struct sockaddr_storage addr_storage;
	struct sockaddr* saddr = (struct sockaddr*)&addr_storage;
	socklen_t saddrlen = sizeof(addr_storage);
	if (getsockname(sock, saddr, &saddrlen) == 0) {
		if ((saddr->sa_family == AF_INET) &&
		    (ntohs(((struct sockaddr_in*)saddr)->sin_port) == MDNS_PORT)){
			rclass &= ~MDNS_UNICAST_RESPONSE;
                   }
#ifdef WEBRTC_ENABLE_IPV6
		else if ((saddr->sa_family == AF_INET6) &&
		         (ntohs(((struct sockaddr_in6*)saddr)->sin6_port) == MDNS_PORT)){
			rclass &= ~MDNS_UNICAST_RESPONSE;
			}
#endif
	}

	struct webrtc_mdns_header_t* header = (struct webrtc_mdns_header_t*)buffer;
	// Query ID
	header->query_id = htons(query_id);
	// Flags
	header->flags = 0;
	// Questions
	header->questions = htons(1);
	// No answer, authority or additional RRs
	header->answer_rrs = 0;
	header->authority_rrs = 0;
	header->additional_rrs = 0;
	// Fill in question
	// Name string
	void* data = MDNS_POINTER_OFFSET(buffer, sizeof(struct webrtc_mdns_header_t));
	data = webrtc_mdns_string_make(buffer, capacity, data, name, length, 0);
	if (!data)
		return -1;
	// Record type
	data = webrtc_mdns_htons(data, type);
	//! Optional unicast response based on local port, class IN
	data = webrtc_mdns_htons(data, rclass);

	size_t tosend = MDNS_POINTER_DIFF(data, buffer);
	if (webrtc_mdns_multicast_send(sock, buffer, (size_t)tosend))
		return -1;
	return query_id;
}
static int webrtc_find_query(const query_record_t *record, const webrtc_mdns_string_t *querystr)
{                
	if(strncmp(record->service,querystr->str,querystr->length)==0 )
	{
			return 0;
	}
        
        return 1;
}
int webrtc_mdns_set_equipment_name(void *webrtc, char *name){
     PWebrtc_mdns pmdns = (PWebrtc_mdns)webrtc;
     pmdns->mdns_service_.txt_record[1].data.txt.value.length = strlen(name);

      return 1;
}
int webrtc_mdns_set_equipment_type(void *webrtc, char *type){
     PWebrtc_mdns pmdns = (PWebrtc_mdns)webrtc;
     pmdns->mdns_service_.txt_record[2].data.txt.value.length = strlen(type);

     return 1;

}
int webrtc_mdns_set_equipment_http(void *webrtc, char *http){
     PWebrtc_mdns pmdns = (PWebrtc_mdns)webrtc;
     pmdns->mdns_service_.txt_record[3].data.txt.value.length = strlen(pmdns->szweb_http_port_);

    return 1;

}
int webrtc_mdns_set_equipment_https(void *webrtc, char *https){
     PWebrtc_mdns pmdns = (PWebrtc_mdns)webrtc;
      pmdns->mdns_service_.txt_record[4].data.txt.value.length = strlen(pmdns->szweb_https_port_);

    return 1;

}
void webrtc_mdns_set_callback(webrtc_mdns_callback_fn callback,void* user){
    g_callback = callback;
    g_user = user;
}
int service_callback(int sock, const struct sockaddr* from, size_t addrlen, webrtc_mdns_entry_type_t entry,
                 uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
                 size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                 size_t record_length, char *outbuf,size_t outsize,void* user_data) {
	(void)sizeof(ttl);
	 PWebrtc_mdns pmdns = (PWebrtc_mdns)user_data;
         char *addrbuffer = NULL;
         char *entrybuffer= NULL;
	 char *namebuffer= NULL;
	 char *sendbuffer= outbuf;
         addrbuffer = malloc(64);
	 if(addrbuffer == NULL){
       	 	LOGE("service_callback failed malloc \n");
		goto call_back_end;
	 }
     	 os_memset(addrbuffer,0,64);
         entrybuffer = malloc(256);
	 if(entrybuffer == NULL){
        	LOGE("service_callback failed malloc \n");
		goto call_back_end;
	 }
      	 os_memset(entrybuffer,0,64);
         namebuffer = malloc(256);
	 if(namebuffer == NULL){
        	LOGE("service_callback failed malloc \n");
		goto call_back_end;
	 }
         os_memset(namebuffer,0,64);
      
	const char dns_sd[] = "_services._dns-sd._udp.local.";
	const service_t* service = (const service_t*)&pmdns->mdns_service_;
        LOGD(" mdns service_callback ---------------------- entry type -------------------%d\n",entry);
	
	if (entry != MDNS_ENTRYTYPE_QUESTION){
                 LOGE(" mdns service_callback entry type %d\n",entry);
                 goto call_back_end;
	}
	webrtc_mdns_string_t fromaddrstr = ip_address_to_string(addrbuffer, 64, from, addrlen);

	size_t offset = name_offset;
	webrtc_mdns_string_t name = webrtc_mdns_string_extract(data, size, &offset, namebuffer, 256);

	const char* record_name = 0;
	if (rtype == MDNS_RECORDTYPE_PTR)
		record_name = "PTR";
	else if (rtype == MDNS_RECORDTYPE_SRV)
		record_name = "SRV";
	else if (rtype == MDNS_RECORDTYPE_A)
		record_name = "A";
	else if (rtype == MDNS_RECORDTYPE_AAAA)
		record_name = "AAAA";
	else if (rtype == MDNS_RECORDTYPE_ANY)
		record_name = "ANY";
	else{
		 goto call_back_end;
	}
    	LOGD("mDns service_callback Recv Query %s  from %.*s   %.*s\n", record_name, MDNS_STRING_FORMAT(fromaddrstr),MDNS_STRING_FORMAT(name));

	if ((name.length == (strlen(dns_sd))) &&
	    (strncmp(name.str, dns_sd, strlen(dns_sd)) == 0) && ((rtype == MDNS_RECORDTYPE_PTR) || (rtype == MDNS_RECORDTYPE_ANY))) {
		
			// The PTR query was for the DNS-SD domain, send answer with a PTR record for the
			// service name we advertise, typically on the "<_service-name>._tcp.local." format

			// Answer PTR record reverse mapping "<_service-name>._tcp.local." to
			// "<hostname>.<_service-name>._tcp.local."
			webrtc_mdns_record_t answer = {
			    .name = name, .type = MDNS_RECORDTYPE_PTR, .data.ptr.name = service->service};

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
            		LOGI("mDns service_callback ********************dns_sd   ************--> answer %.*s (%s)\n",MDNS_STRING_FORMAT(answer.data.ptr.name),
			       (unicast ? "unicast" : "multicast"));
             		os_memset(sendbuffer,0,1024);
			if (unicast) {
				webrtc_mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, 1024,
				                          query_id, rtype, name.str, name.length, answer, 0, 0, 0,
				                          0);
			} else {
				webrtc_mdns_query_answer_multicast(sock, sendbuffer, 1024, answer, 0, 0, 0,
				                            0);
			}
		
	} else if ((name.length == service->service.length) &&
	           (strncmp(name.str, service->service.str, name.length) == 0) && (service->address_ipv4.sin_family == AF_INET) && ((rtype == MDNS_RECORDTYPE_PTR) || (rtype == MDNS_RECORDTYPE_ANY))) {
		
			// The PTR query was for our service (usually "<_service-name._tcp.local"), answer a PTR
			// record reverse mapping the queried service name to our service instance name
			// (typically on the "<hostname>.<_service-name>._tcp.local." format), and add
			// additional records containing the SRV record mapping the service instance name to our
			// qualified hostname (typically "<hostname>.local.") and port, as well as any IPv4/IPv6
			// address for the hostname as A/AAAA records, and two test TXT records

			// Answer PTR record reverse mapping "<_service-name>._tcp.local." to
			// "<hostname>.<_service-name>._tcp.local."
			webrtc_mdns_record_t answer = service->record_ptr;

			webrtc_mdns_record_t additional[10];
			size_t additional_count = 0;

			// SRV record mapping "<hostname>.<_service-name>._tcp.local." to
			// "<hostname>.local." with port. Set weight & priority to 0.
			additional[additional_count++] = service->record_srv;

			// A/AAAA records mapping "<hostname>.local." to IPv4/IPv6 addresses
			if (service->address_ipv4.sin_family == AF_INET)
				additional[additional_count++] = service->record_a;
#ifdef WEBRTC_ENABLE_IPV6
			if (service->address_ipv6.sin6_family == AF_INET6)
				additional[additional_count++] = service->record_aaaa;
#endif

			// Add two test TXT records for our service instance name, will be coalesced into
			// one record with both key-value pair strings by the library
			if(pmdns->mdns_send_txt_== true){
				additional[additional_count++] = service->txt_record[0];
				additional[additional_count++] = service->txt_record[1];
				additional[additional_count++] = service->txt_record[2];
				additional[additional_count++] = service->txt_record[3];
				additional[additional_count++] = service->txt_record[4];
				additional[additional_count++] = service->txt_record[5];
			}

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
            		LOGD("mDns service_callback MDNS_RECORDTYPE_PTR  --> answer %.*s (%s)\n",
			       MDNS_STRING_FORMAT(service->record_ptr.data.ptr.name),
			       (unicast ? "unicast" : "multicast"));
             		os_memset(sendbuffer,0,1024);
			if (unicast) {
				webrtc_mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, 1024,
				                          query_id, rtype, name.str, name.length, answer, 0, 0,
				                          additional, additional_count);
			} else {
				webrtc_mdns_query_answer_multicast(sock, sendbuffer, 1024, answer, 0, 0,
				                            additional, additional_count);
			}
		
	} else if ((name.length == service->service_instance.length) &&
	           (strncmp(name.str, service->service_instance.str, name.length) == 0) && (service->address_ipv4.sin_family == AF_INET) && ((rtype == MDNS_RECORDTYPE_SRV) || (rtype == MDNS_RECORDTYPE_ANY))) {
		
			// The SRV query was for our service instance (usually
			// "<hostname>.<_service-name._tcp.local"), answer a SRV record mapping the service
			// instance name to our qualified hostname (typically "<hostname>.local.") and port, as
			// well as any IPv4/IPv6 address for the hostname as A/AAAA records, and two test TXT
			// records

			// Answer PTR record reverse mapping "<_service-name>._tcp.local." to
			// "<hostname>.<_service-name>._tcp.local."
			webrtc_mdns_record_t answer = service->record_srv;

			webrtc_mdns_record_t additional[10];
			size_t additional_count = 0;

			// A/AAAA records mapping "<hostname>.local." to IPv4/IPv6 addresses
			if (service->address_ipv4.sin_family == AF_INET)
				additional[additional_count++] = service->record_a;
#ifdef WEBRTC_ENABLE_IPV6
			if (service->address_ipv6.sin6_family == AF_INET6)
				additional[additional_count++] = service->record_aaaa;
#endif

			// Add two test TXT records for our service instance name, will be coalesced into
			// one record with both key-value pair strings by the library
			if(pmdns->mdns_send_txt_== true){
				additional[additional_count++] = service->txt_record[0];
				additional[additional_count++] = service->txt_record[1];
				additional[additional_count++] = service->txt_record[2];
				additional[additional_count++] = service->txt_record[3];
				additional[additional_count++] = service->txt_record[4];
				additional[additional_count++] = service->txt_record[5];
			}

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
            		LOGD("mDns service_callback MDNS_RECORDTYPE_SRV  --> answer %.*s port %d (%s)\n",
			       MDNS_STRING_FORMAT(service->record_srv.data.srv.name), service->port,
			       (unicast ? "unicast" : "multicast"));
             		os_memset(sendbuffer,0,1024);
			if (unicast) {
				webrtc_mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, 1024,
				                          query_id, rtype, name.str, name.length, answer, 0, 0,
				                          additional, additional_count);
			} else {
				webrtc_mdns_query_answer_multicast(sock, sendbuffer, 1024, answer, 0, 0,
				                            additional, additional_count);
			}
		
	} else if ((name.length == service->hostname_qualified.length) &&
	           (strncmp(name.str, service->hostname_qualified.str, name.length) == 0)) {

		if (((rtype == MDNS_RECORDTYPE_A) || (rtype == MDNS_RECORDTYPE_ANY)) &&
		    (service->address_ipv4.sin_family == AF_INET)) {
			// The A query was for our qualified hostname (typically "<hostname>.local.") and we
			// have an IPv4 address, answer with an A record mappiing the hostname to an IPv4
			// address, as well as any IPv6 address for the hostname, and two test TXT records

			// Answer A records mapping "<hostname>.local." to IPv4 address
			webrtc_mdns_record_t answer = service->record_a;

			webrtc_mdns_record_t additional[10];
			size_t additional_count = 0;

			// AAAA record mapping "<hostname>.local." to IPv6 addresses
#ifdef WEBRTC_ENABLE_IPV6
			if (service->address_ipv6.sin6_family == AF_INET6)
				additional[additional_count++] = service->record_aaaa;
#endif

			// Add two test TXT records for our service instance name, will be coalesced into
			// one record with both key-value pair strings by the library
			if(pmdns->mdns_send_txt_== true){
				additional[additional_count++] = service->txt_record[0];
				additional[additional_count++] = service->txt_record[1];
				additional[additional_count++] = service->txt_record[2];
				additional[additional_count++] = service->txt_record[3];
				additional[additional_count++] = service->txt_record[4];
				additional[additional_count++] = service->txt_record[5];
			}

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
			webrtc_mdns_string_t addrstr = ip_address_to_string(
			    addrbuffer, 64, (struct sockaddr*)&service->record_a.data.a.addr,
			    sizeof(service->record_a.data.a.addr));
            		LOGD("mDns service_callback MDNS_RECORDTYPE_A --> answer %.*s IPv4 %.*s (%s)\n", MDNS_STRING_FORMAT(service->record_a.name),
			       MDNS_STRING_FORMAT(addrstr), (unicast ? "unicast" : "multicast"));
             		os_memset(sendbuffer,0,1024);
			if (unicast) {
				webrtc_mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, 1024,
				                          query_id, rtype, name.str, name.length, answer, 0, 0,
				                          additional, additional_count);
			} else {
				webrtc_mdns_query_answer_multicast(sock, sendbuffer, 1024, answer, 0, 0,
				                            additional, additional_count);
			}
		} else if (((rtype == MDNS_RECORDTYPE_AAAA) || (rtype == MDNS_RECORDTYPE_ANY)) ) {
#ifdef WEBRTC_ENABLE_IPV6
			if(service->address_ipv6.sin6_family == AF_INET6){
			// The AAAA query was for our qualified hostname (typically "<hostname>.local.") and we
			// have an IPv6 address, answer with an AAAA record mappiing the hostname to an IPv6
			// address, as well as any IPv4 address for the hostname, and two test TXT records

			// Answer AAAA records mapping "<hostname>.local." to IPv6 address
			webrtc_mdns_record_t answer = service->record_aaaa;

			webrtc_mdns_record_t additional[10];
			size_t additional_count = 0;

			// A record mapping "<hostname>.local." to IPv4 addresses
			if (service->address_ipv4.sin_family == AF_INET)
				additional[additional_count++] = service->record_a;

			// Add two test TXT records for our service instance name, will be coalesced into
			// one record with both key-value pair strings by the library
			if(pmdns->mdns_send_txt_== true){
				additional[additional_count++] = service->txt_record[0];
				additional[additional_count++] = service->txt_record[1];
				additional[additional_count++] = service->txt_record[2];
				additional[additional_count++] = service->txt_record[3];
				additional[additional_count++] = service->txt_record[4];
				additional[additional_count++] = service->txt_record[5];
			}

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
			webrtc_mdns_string_t addrstr =
			    ip_address_to_string(addrbuffer, 64,
			                         (struct sockaddr*)&service->record_aaaa.data.aaaa.addr,
			                         sizeof(service->record_aaaa.data.aaaa.addr));
            LOGD("mDns service_callback  MDNS_RECORDTYPE_AAAA --> answer %.*s IPv6 %.*s (%s)\n",
			       MDNS_STRING_FORMAT(service->record_aaaa.name), MDNS_STRING_FORMAT(addrstr),
			       (unicast ? "unicast" : "multicast"));
             os_memset(sendbuffer,0,1024);
			if (unicast) {
				webrtc_mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, 1024,
				                          query_id, rtype, name.str, name.length, answer, 0, 0,
				                          additional, additional_count);
			} else {
				webrtc_mdns_query_answer_multicast(sock, sendbuffer, 1024, answer, 0, 0,
				                            additional, additional_count);
			}
			}
#endif
        }
	}
call_back_end:
      
       if(addrbuffer!= NULL){
        free(addrbuffer);
		addrbuffer = NULL;
       }
       if(entrybuffer!= NULL){
        free(entrybuffer);
		entrybuffer = NULL;
       }
       if(namebuffer!= NULL){
        free(namebuffer);
		namebuffer = NULL;
       }
   
   

	return 0;
}


// Callback handling parsing answers to queries sent
static int
query_callback(int sock, const struct sockaddr* from, size_t addrlen, webrtc_mdns_entry_type_t entry,
               uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
               size_t size, size_t name_offset, size_t name_length, size_t record_offset,
               size_t record_length, void* user_data) {
       //LOGD("%s %d \n", __func__, __LINE__);

        PWebrtc_mdns pmdns = (PWebrtc_mdns)user_data;
        const service_t* service = (const service_t*)&pmdns->mdns_service_;
	char addrbuffer[64]={0};
 	char entrybuffer[256]={0};
 	char namebuffer[256]={0};
        //RTCList *elem = NULL;
 	webrtc_mdns_record_txt_t txtbuffer[128];
    // LOGD(" mdns query_callback<<<<<<<<<<<<<<<<<<<<<<< rtype  >>>>>>>>>>>>>>>>>>%d\n",rtype);
	webrtc_mdns_string_t fromaddrstr = ip_address_to_string(addrbuffer, sizeof(addrbuffer), from, addrlen);
	const char* entrytype = (entry == MDNS_ENTRYTYPE_ANSWER) ?
                                "answer" :
                                ((entry == MDNS_ENTRYTYPE_AUTHORITY) ? "authority" : "additional");
	webrtc_mdns_string_t entrystr =
	    webrtc_mdns_string_extract(data, size, &name_offset, entrybuffer, sizeof(entrybuffer));

#if 1
	if (rtype == MDNS_RECORDTYPE_PTR) {

		webrtc_mdns_string_t namestr = webrtc_mdns_record_parse_ptr(data, size, record_offset, record_length,
		                                              namebuffer, sizeof(namebuffer));
        	LOGD("mDns query_callback MDNS_RECORDTYPE_PTR recv answer from %.*s : %s %.*s PTR %.*s rclass 0x%x ttl %u length %d\n",
		       MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr),
		       MDNS_STRING_FORMAT(namestr), rclass, ttl, (int)record_length);

        if ((namestr.length == service->service.length) &&
                       (strncmp(namestr.str, service->service.str, namestr.length) == 0)){
                size_t capacity = 2*1024;
                void* buffer = os_malloc(capacity);
                if(buffer!= NULL){
		         os_memset(buffer,0,capacity);
		         webrtc_mdns_discovery_query_send(sock,MDNS_RECORDTYPE_A, entrystr.str,
		                                         entrystr.length, buffer, capacity, query_id,from,addrlen);
		         os_free(buffer);
                }
        }

	} else if (rtype == MDNS_RECORDTYPE_SRV) {

		webrtc_mdns_record_srv_t srv = webrtc_mdns_record_parse_srv(data, size, record_offset, record_length,
		                                              namebuffer, sizeof(namebuffer));
        	LOGD("mDns query_callback MDNS_RECORDTYPE_SRV recv answer from %.*s : %s %.*s SRV %.*s priority %d weight %d port %d\n",
		       MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr),
		       MDNS_STRING_FORMAT(srv.name), srv.priority, srv.weight, srv.port);

	} else if (rtype == MDNS_RECORDTYPE_A) {

		struct sockaddr_in addr;
		webrtc_mdns_record_parse_a(data, size, record_offset, record_length, &addr);
		webrtc_mdns_string_t addrstr =
		    ipv4_address_to_string(namebuffer, sizeof(namebuffer), &addr, sizeof(addr));
        	LOGD("mDns query_callback MDNS_RECORDTYPE_A recv answer from %.*s : %s %.*s A %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
		       MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(addrstr));
#if 0
                 elem = rtc_list_find_custom(pmdns->querys_, (RTCCompareFunc)webrtc_find_query,  &entrystr);
		 if(elem != NULL)
		 {
			 
			 query_record_t * query_record = (query_record_t *)elem->data;
			 pmdns->querys_ = rtc_list_remove_link(pmdns->querys_,elem);
			 if(query_record!= NULL){
				  
                 if(query_record->callback_!= NULL){
					//snprintf(query_record->candidate->addr,sizeof(query_record->candidate->addr),"%s",addrstr.str);
                    //printf("mdns query_callback  session %s update candidate address  = %s \n", query_record->szsessionId,query_record->candidate->addr);
					//query_record->callball_(query_record->webrtc,query_record->user,query_record->szsessionId,query_record->candidate,query_record->service,query_record->lable);
					

				 }
                    free(query_record);
                    query_record = NULL;
			 }
			 
		 }else{
			
		 }
#endif


	} else if (rtype == MDNS_RECORDTYPE_AAAA) {
#ifdef WEBRTC_ENABLE_IPV6
		struct sockaddr_in6 addr;
		webrtc_mdns_record_parse_aaaa(data, size, record_offset, record_length, &addr);
		webrtc_mdns_string_t addrstr =
		    ipv6_address_to_string(namebuffer, sizeof(namebuffer), &addr, sizeof(addr));
        	LOGD("mDns query_callback recv answer from %.*s : %s %.*s AAAA %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
		       MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(addrstr));
#endif


	} else if (rtype == MDNS_RECORDTYPE_TXT) {
                LOGD("%s %d MDNS_RECORDTYPE_TXT  %d  %d  %d  \n", __func__, __LINE__,size,record_offset,record_length);

#if 0
		size_t itxt =0;
		size_t parsed = webrtc_mdns_record_parse_txt(data, size, record_offset, record_length, txtbuffer,128);
		//printf("%s %d ----MDNS_RECORDTYPE_TXT size %d\n", __func__, __LINE__,parsed);
		char szkeybuf[256] ={0};
		char szvaluebuf[256] ={0};
		if(parsed>0){
	
		    cJSON *root = cJSON_CreateObject();
		    if(root!= NULL){

		    for (itxt = 0; itxt < parsed; ++itxt) {
		        if (txtbuffer[itxt].value.length) {
		            if(root!= NULL){
		                snprintf(szkeybuf,sizeof(szkeybuf),"%.*s",MDNS_STRING_FORMAT(txtbuffer[itxt].key));
		                snprintf(szvaluebuf,sizeof(szvaluebuf),"%.*s",MDNS_STRING_FORMAT(txtbuffer[itxt].value));
		                cJSON_AddStringToObject(root, szkeybuf, szvaluebuf);
		            }
		            printf("mDns query_callback recv answer MDNS_RECORDTYPE_TXT from %.*s : %s %.*s TXT %.*s = %.*s\n", MDNS_STRING_FORMAT(fromaddrstr),
		                   entrytype, MDNS_STRING_FORMAT(entrystr),
		                   MDNS_STRING_FORMAT(txtbuffer[itxt].key),
		                   MDNS_STRING_FORMAT(txtbuffer[itxt].value));
		        } else {
		            printf("mDns query_callback recv answer MDNS_RECORDTYPE_TXT from %.*s : %s %.*s TXT %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
		                   MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(txtbuffer[itxt].key));
		        }
		    }
		    if(g_callback!= NULL && g_user!= NULL){
		        if(root!= NULL){
		            snprintf(szvaluebuf,sizeof(szvaluebuf),"%.*s",MDNS_STRING_FORMAT(fromaddrstr));
		            cJSON_AddStringToObject(root, "ipaddr", szvaluebuf);
		            cJSON_AddNumberToObject(root,"ttl",ttl);
		            char *str = cJSON_PrintUnformatted(root);
		            if(str){
		               printf("JSON String:\n%s\n", str);
		              //g_callback(g_user,str,strlen(str));
		              os_free(str);

		            }
		          

		        }

		    }
		    cJSON_Delete(root);
		    }
	
		}

#endif



	} else {

        LOGD("mDns query_callback recv answer from %.*s : %s %.*s type %u rclass 0x%x ttl %u length %d\n",
		       MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr), rtype,
		       rclass, ttl, (int)record_length);

	}
#endif

	return 0;
}
size_t webrtc_mdns_handle(int sock,struct sockaddr* from, size_t addrlen,void* buffer, size_t size,char *outbuf,size_t outsize,void *user_data){
        //printf("%s %d   %d\n", __func__, __LINE__,size);
        size_t parsed = 0;
        size_t data_size = (size_t)size;
	uint16_t* data = (uint16_t*)buffer;

	uint16_t query_id = webrtc_mdns_ntohs(data++);
	uint16_t flags = webrtc_mdns_ntohs(data++);
	uint16_t questions = webrtc_mdns_ntohs(data++);
        uint16_t headerBitQR = 1 << 15;
        uint16_t response = flags & headerBitQR;
       // printf("mdns  webrtc_mdns_handle -------  %d   %d  %d  %d\n",questions,flags,query_id,response);
	if(response == 0){
			data += 3;
            //printf("mdns  webrtc_mdns_handle ----questions---  %d   %d  %d\n",questions,flags,query_id);
			int iquestion = 0;

			for ( iquestion = 0; iquestion < questions; ++iquestion) {
				size_t question_offset = MDNS_POINTER_DIFF(data, buffer);
				size_t offset = question_offset;
				size_t verify_ofs = 12;
				int dns_sd = 0;
				if (webrtc_mdns_string_equal(buffer, data_size, &offset, webrtc_mdns_services_query,
						      sizeof(webrtc_mdns_services_query), &verify_ofs)) {
			
					dns_sd = 1;
                   			// printf("mdns  webrtc_mdns_handle ------- dns_sd %d \n",dns_sd);
				} else {
					offset = question_offset;
					if (!webrtc_mdns_string_skip(buffer, data_size, &offset)){
                        			LOGD("mdns  webrtc_mdns_handle ------- webrtc_mdns_string_skip data_size =%d offset = %d\n",data_size,offset);
						break;
					 }
				}
				size_t length = offset - question_offset;
				data = (uint16_t*)MDNS_POINTER_OFFSET_CONST(buffer, offset);

				uint16_t rtype = webrtc_mdns_ntohs(data++);
				uint16_t rclass = webrtc_mdns_ntohs(data++);

				// Make sure we get a question of class IN
				if ((rclass & 0x7FFF) != MDNS_CLASS_IN){
                    			LOGD("mdns  webrtc_mdns_handle we get a question of no class in -------  %d \n",(rclass & 0x7FFF));
					break;
				}
				if (dns_sd && flags){
                    			LOGD("mdns  webrtc_mdns_handle ------- dns_sd =%d flags = %d\n",dns_sd,flags);
					continue;
				}

				++parsed;
				if (service_callback(sock, from, addrlen, MDNS_ENTRYTYPE_QUESTION, query_id, rtype,
						         rclass, 0, buffer, data_size, question_offset, length,
						         question_offset, length, outbuf,outsize,user_data)){
					break;
				}
			}

        }else if(response == headerBitQR){
		  
              	  uint16_t answer_rrs = webrtc_mdns_ntohs(data++);
	          uint16_t authority_rrs = webrtc_mdns_ntohs(data++);
	          uint16_t additional_rrs = webrtc_mdns_ntohs(data++);

	          if (questions > 1)
		     return 0;

		 int i;
		 for (i = 0; i < questions; ++i) {
			size_t ofs = MDNS_POINTER_DIFF(data, buffer);
			if (!webrtc_mdns_string_skip(buffer, data_size, &ofs))
				return 0;
			data = ( uint16_t*)MDNS_POINTER_OFFSET_CONST(buffer, ofs);
			/* Record type and class not used, skip
			uint16_t rtype = webrtc_mdns_ntohs(data++);
			uint16_t rclass = webrtc_mdns_ntohs(data++);*/
			data += 2;
		 }

		size_t records = 0;
		size_t total_records = 0;
		size_t offset = MDNS_POINTER_DIFF(data, buffer);
         	//printf("mdns  webrtc_mdns_handle ----query  query_id = %d \n",query_id);
         	//printf("mdns  webrtc_mdns_handle ----query  answer_rrs = %d \n",answer_rrs);
         	//printf("mdns  webrtc_mdns_handle ----query  authority_rrs = %d \n",authority_rrs);
        	//printf("mdns  webrtc_mdns_handle ----query  additional_rrs = %d  \n",additional_rrs);
         	//printf("mdns  webrtc_mdns_handle ----query  offset = %d \n",offset);
	        records = webrtc_mdns_records_parse(sock, from, addrlen, buffer, data_size, &offset,
			                     MDNS_ENTRYTYPE_ANSWER, query_id, answer_rrs, query_callback, user_data);
		total_records += records;
		if (records != answer_rrs)
			return total_records;

		records =
		    webrtc_mdns_records_parse(sock, from, addrlen, buffer, data_size, &offset,
			               MDNS_ENTRYTYPE_AUTHORITY, query_id, authority_rrs, query_callback, user_data);
		total_records += records;
		if (records != authority_rrs)
			return total_records;

		records = webrtc_mdns_records_parse(sock, from, addrlen, buffer, data_size, &offset,
			                     MDNS_ENTRYTYPE_ADDITIONAL, query_id, additional_rrs, query_callback,
			                     user_data);
		total_records += records;
		if (records != additional_rrs)
			return total_records;


            return total_records;
		
        }else{
        	LOGD("mdns  webrtc_mdns_handle  unhandle ================ flags = %d response = %d\n",flags,response);
        }

	return parsed;

}
int  webrtc_mdns_send_query(const char* service,void *webrtc,void *owner,void *candidate,char *szsessionId,int lable,webrtc_mdns_query_callback_fn callback) {
#if 0
         PWebrtc_mdns pmdns = (PWebrtc_mdns)webrtc;
    if(pmdns->runing ==true ){
                char szservice[128]={0};
                snprintf(szservice,sizeof(szservice),"%s.",service);
                uint16_t queryid = 128;
 
		srand((unsigned)time(NULL));
		queryid = (uint16_t)rand();

        query_record_t * record = (query_record_t *)malloc(sizeof(query_record_t));
		if(record!= NULL){
			  snprintf(record->service,sizeof(record->service),"%s",szservice);
			  snprintf(record->szsessionId,sizeof(record->szsessionId),"%s",szsessionId);
			  record->webrtc = webrtc;
			  record->user = owner;
              record->callback_ = callback;
			  record->candidate = candidate;
			  record->lable = lable;
			  record->queryid =  queryid;

              //pmdns->querys_=rtc_list_append(pmdns->querys_,record);

	
		
			size_t capacity = 2*1024;
            void* buffer = malloc(capacity);
			if(buffer!= NULL){
                for(int i = 0;i<16;i++){
               if(pmdns->mdnssockets[i].mdns_sock_!= INVALID_SOCKET && pmdns->mdnssockets[i].haveip){
                os_memset(buffer,0,capacity);
                printf("mdns  Sending mDNS query: %s\n", szservice);
                webrtc_mdns_query_send(pmdns->mdnssockets[i].mdns_sock_, MDNS_RECORDTYPE_A, szservice,
								          strlen(szservice), buffer, capacity, queryid);
                }
                }

                free(buffer);
			}else{
                printf("webrtc_mdns_send_query failed malloc capacity\n");
			}

		
		}else{
            printf("webrtc_mdns_send_query failed malloc query_record_t\n");
		}
	
        }
#endif
	return 0;
}
void webrtc_mdns_socket_close(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}
int webrtc_mdns_socket_open_ipv4(const struct sockaddr_in* saddr,struct ip_mreq *mreq) {
    int sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
        return -1;
    if (webrtc_mdns_socket_setup_ipv4(sock, saddr,mreq)) {
        webrtc_mdns_socket_close(sock);
        return -1;
    }
    return sock;
}
int webrtc_mdns_socket_setup_ipv4(int sock, const struct sockaddr_in* saddr,struct ip_mreq *mreq){
    unsigned char ttl = 1;
    unsigned char loopback = 0;
    unsigned int reuseaddr = 1;
    struct ip_mreq req;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
#ifdef SO_REUSEPORT
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuseaddr, sizeof(reuseaddr));
#endif
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl));
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*)&loopback, sizeof(loopback));

    os_memset(&req, 0, sizeof(req));
    req.imr_multiaddr.s_addr = htonl((((uint32_t)224U) << 24U) | ((uint32_t)251U));
    if (saddr)
        req.imr_interface = saddr->sin_addr;
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&req, sizeof(req)))
        return -1;
    os_memcpy(mreq,&req,sizeof(struct ip_mreq));
    struct sockaddr_in sock_addr;
    if (!saddr) {
        os_memset(&sock_addr, 0, sizeof(struct sockaddr_in));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = INADDR_ANY;
#ifdef __APPLE__
        sock_addr.sin_len = sizeof(struct sockaddr_in);
#endif
    } else {
        os_memcpy(&sock_addr, saddr, sizeof(struct sockaddr_in));
        setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&sock_addr.sin_addr,
                   sizeof(sock_addr.sin_addr));
#ifndef _WIN32
        sock_addr.sin_addr.s_addr = INADDR_ANY;
#endif
    }

    if (bind(sock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_in)))
        return -1;

#ifdef _WIN32
    unsigned long param = 1;
    ioctlsocket(sock, FIONBIO, &param);
#else
    const int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

    return 0;
}



