
#ifndef WEBRTC_MDNS_H_F360898B84CE49FF9F88F5DB59B5950C
#define WEBRTC_MDNS_H_F360898B84CE49FF9F88F5DB59B5950C

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


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
#include "rtc_list.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MDNS_PORT 5353
#define MDNS_ADDR "224.0.0.251"



#define MDNS_INVALID_POS ((size_t)-1)

#define MDNS_STRING_CONST(s) (s), (sizeof((s)) - 1)
#define MDNS_STRING_ARGS(s) s.str, s.length
#define MDNS_STRING_FORMAT(s) (int)((s).length), s.str

#define MDNS_POINTER_OFFSET(p, ofs) ((void*)((char*)(p) + (ptrdiff_t)(ofs)))
#define MDNS_POINTER_OFFSET_CONST(p, ofs) ((const void*)((const char*)(p) + (ptrdiff_t)(ofs)))
#define MDNS_POINTER_DIFF(a, b) ((size_t)((const char*)(a) - (const char*)(b)))


#define MDNS_UNICAST_RESPONSE 0x8000U
#define MDNS_CACHE_FLUSH 0x8000U
#define MDNS_MAX_SUBSTRINGS 64

enum webrtc_mdns_record_type {
	MDNS_RECORDTYPE_IGNORE = 0,
	// Address
	MDNS_RECORDTYPE_A = 1,
	// Domain Name pointer
	MDNS_RECORDTYPE_PTR = 12,
	// Arbitrary text string
	MDNS_RECORDTYPE_TXT = 16,
	// IP6 Address [Thomson]
	MDNS_RECORDTYPE_AAAA = 28,
	// Server Selection [RFC2782]
	MDNS_RECORDTYPE_SRV = 33,
	// Any available records
	MDNS_RECORDTYPE_ANY = 255
};

enum webrtc_mdns_entry_type {
	MDNS_ENTRYTYPE_QUESTION = 0,
	MDNS_ENTRYTYPE_ANSWER = 1,
	MDNS_ENTRYTYPE_AUTHORITY = 2,
	MDNS_ENTRYTYPE_ADDITIONAL = 3
};

enum webrtc_mdns_class { MDNS_CLASS_IN = 1 };

typedef enum webrtc_mdns_record_type webrtc_mdns_record_type_t;
typedef enum webrtc_mdns_entry_type webrtc_mdns_entry_type_t;
typedef enum webrtc_mdns_class webrtc_mdns_class_t;

typedef int (*webrtc_mdns_record_callback_fn)(int sock, const struct sockaddr* from, size_t addrlen,
                                       webrtc_mdns_entry_type_t entry, uint16_t query_id, uint16_t rtype,
                                       uint16_t rclass, uint32_t ttl, const void* data, size_t size,
                                       size_t name_offset, size_t name_length, size_t record_offset,
                                       size_t record_length, void* user_data);
typedef void(*webrtc_mdns_query_callback_fn)(void *webrtc,void *user,char* sessionId,void* candidate,char *service,int lable);
typedef void(*webrtc_mdns_callback_fn)(void *user,char* data,int len);
typedef struct webrtc_mdns_string_t webrtc_mdns_string_t;
typedef struct webrtc_mdns_string_pair_t webrtc_mdns_string_pair_t;
typedef struct webrtc_mdns_string_table_item_t webrtc_mdns_string_table_item_t;
typedef struct webrtc_mdns_string_table_t webrtc_mdns_string_table_t;
typedef struct webrtc_mdns_record_t webrtc_mdns_record_t;
typedef struct webrtc_mdns_record_srv_t webrtc_mdns_record_srv_t;
typedef struct webrtc_mdns_record_ptr_t webrtc_mdns_record_ptr_t;
typedef struct webrtc_mdns_record_a_t webrtc_mdns_record_a_t;
typedef struct webrtc_mdns_record_aaaa_t webrtc_mdns_record_aaaa_t;
typedef struct webrtc_mdns_record_txt_t webrtc_mdns_record_txt_t;


typedef size_t webrtc_mdns_size_t;
typedef size_t webrtc_mdns_ssize_t;


struct webrtc_mdns_string_t {
	const char* str;
	size_t length;
};

struct webrtc_mdns_string_pair_t {
	size_t offset;
	size_t length;
	int ref;
};

struct webrtc_mdns_string_table_t {
	size_t offset[16];
	size_t count;
	size_t next;
};

struct webrtc_mdns_record_srv_t {
	uint16_t priority;
	uint16_t weight;
	uint16_t port;
	webrtc_mdns_string_t name;
};

struct webrtc_mdns_record_ptr_t {
	webrtc_mdns_string_t name;
};

struct webrtc_mdns_record_a_t {
	struct sockaddr_in addr;
};

struct webrtc_mdns_record_aaaa_t {
#ifdef WEBRTC_ENABLE_IPV6
        struct sockaddr_in6 addr;
#else
	struct sockaddr_in addr;
#endif
};

struct webrtc_mdns_record_txt_t {
	webrtc_mdns_string_t key;
	webrtc_mdns_string_t value;
};

struct webrtc_mdns_record_t {
	webrtc_mdns_string_t name;
	webrtc_mdns_record_type_t type;
	union webrtc_mdns_record_data {
		webrtc_mdns_record_ptr_t ptr;
		webrtc_mdns_record_srv_t srv;
		webrtc_mdns_record_a_t a;
		webrtc_mdns_record_aaaa_t aaaa;
		webrtc_mdns_record_txt_t txt;
	} data;
};

struct webrtc_mdns_header_t {
	uint16_t query_id;
	uint16_t flags;
	uint16_t questions;
	uint16_t answer_rrs;
	uint16_t authority_rrs;
	uint16_t additional_rrs;
};
typedef struct service_record{
	const char* service;
	const char* hostname;
	uint32_t address_ipv4;
	uint8_t* address_ipv6;
	int port;
    char serno[64];
} service_record_t;
typedef struct query_record{
	char service[128];
    char szsessionId[128];
	void *candidate;
    void *webrtc;
	void *user;
    webrtc_mdns_query_callback_fn callback_;
	int lable;
	uint16_t queryid;
} query_record_t;

typedef struct {
	webrtc_mdns_string_t service;
	webrtc_mdns_string_t hostname;
	webrtc_mdns_string_t service_instance;
	webrtc_mdns_string_t hostname_qualified;
	struct sockaddr_in address_ipv4;
#ifdef WEBRTC_ENABLE_IPV6
	struct sockaddr_in6 address_ipv6;
#endif
	int port;
	webrtc_mdns_record_t record_ptr;
	webrtc_mdns_record_t record_srv;
	webrtc_mdns_record_t record_a;
	webrtc_mdns_record_t record_aaaa;
	webrtc_mdns_record_t txt_record[5];
} service_t;
typedef struct _mdns_socket{
        bool haveip;
        char szlocal_addr_[64];
        int mdns_sock_;
        struct ip_mreq req;
}mdns_socket;
typedef struct _Webrtc_mdns
{
       service_t mdns_service_;
       service_record_t service_record;
       RTCList *querys_;
       int mdns_sock_;
       uint32_t mdns_stop_time_;
       bool mdns_send_txt_;
       bool runing;
	bool mdns_runing;
	bool mdns_thread_runing;
	bool mdns_thread_exit_;
        int web_https_port;
        int web_http_port;
	char szlocal_addr_[64];
	char szweb_http_port_[64];
	char szweb_https_port_[64];
	char szweb_local_addr_[64];
        mdns_socket mdnssockets[16];
	char szversion_[64];
	char szRegNum_[64];
	char szEquipmentName_[64];
	char szEquipmentType_[32];
	char szEquipmentId_[64];
	char szMdnsservername_[64];
}Webrtc_mdns,*PWebrtc_mdns;
// Implementations
webrtc_mdns_string_t webrtc_ip_address_to_string(char* buffer, size_t capacity, const struct sockaddr* addr, size_t addrlen);
int webrtc_mdns_socket_setup_ipv4(int sock, const struct sockaddr_in* saddr,struct ip_mreq *mreq);
void webrtc_mdns_socket_close(int sock);

int webrtc_mdns_socket_open_ipv4(const struct sockaddr_in* saddr,struct ip_mreq *mreq);

int webrtc_mdns_announce_multicast(int sock, void* buffer, size_t capacity, webrtc_mdns_record_t answer,
                        webrtc_mdns_record_t* authority, size_t authority_count, webrtc_mdns_record_t* additional,
                        size_t additional_count) ;
int webrtc_mdns_query_answer_multicast(int sock, void* buffer, size_t capacity, webrtc_mdns_record_t answer,
                            webrtc_mdns_record_t* authority, size_t authority_count,
                            webrtc_mdns_record_t* additional, size_t additional_count);
int webrtc_mdns_goodbye_multicast(int sock, void* buffer, size_t capacity, webrtc_mdns_record_t answer,
                        webrtc_mdns_record_t* authority, size_t authority_count, webrtc_mdns_record_t* additional,
                        size_t additional_count);
size_t webrtc_mdns_handle(int sock,struct sockaddr* from, size_t addrlen,void* buffer, size_t size,char *outbuf,size_t outsize,void *user_data);
size_t webrtc_mdns_records_parse(int sock, const struct sockaddr* from, size_t addrlen, const void* buffer,
                   size_t size, size_t* offset, webrtc_mdns_entry_type_t type, uint16_t query_id,
                   size_t records, webrtc_mdns_record_callback_fn callback, void* user_data) ;
int webrtc_mdns_unicast_send(int sock, const void* address, size_t address_size, const void* buffer,size_t size); 

int webrtc_mdns_multicast_send(int sock, const void* buffer, size_t size);
int webrtc_mdns_discovery_send(int sock);
int  webrtc_mdns_send_query(const char* service,void *webrtc,void *owner,void *candidate,char *szsessionId,int lable,webrtc_mdns_query_callback_fn callback);
int webrtc_mdns_set_equipment_name(void *webrtc, char *name);
int webrtc_mdns_set_equipment_type(void *webrtc, char *type);
int webrtc_mdns_set_equipment_http(void *webrtc, char *http);
int webrtc_mdns_set_equipment_https(void *webrtc, char *https);
void webrtc_streamer_mdns_announce_multicast(void *webrtc);
void webrtc_mdns_set_callback(webrtc_mdns_callback_fn callback,void* user);
#ifdef __cplusplus
}
#endif
#endif

