#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bk_private/bk_init.h"
#include <common/bk_typedef.h>
#include <components/system.h>
//#include "common.h"
#include <os/os.h>
#include <os/mem.h>
#include <components/log.h>
#include "cli.h"
#include "network_configure.h"
#if CONFIG_BLE
#include "ble_api_5_x.h"
#include "bluetooth_legacy_include.h"

#include "components/bluetooth/bk_ble.h"
#include "components/bluetooth/bk_dm_ble.h"
#include "components/bluetooth/bk_dm_bluetooth.h"
#include "cJSON.h"
#include "cfg_all.h"

#define BLE_GATT_SERVER_TAG "BLE-GATTS"
#define TAG "BLE-GATTS"
#define BLEGATTS_LOGI(...) BK_LOGW(BLE_GATT_SERVER_TAG, ##__VA_ARGS__)
#define BLEGATTS_LOGW(...) BK_LOGW(BLE_GATT_SERVER_TAG, ##__VA_ARGS__)
#define BLEGATTS_LOGE(...) BK_LOGW(BLE_GATT_SERVER_TAG, ##__VA_ARGS__)
#define BLEGATTS_LOGD(...) BK_LOGW(BLE_GATT_SERVER_TAG, ##__VA_ARGS__)
#define LOGI(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGW(TAG, ##__VA_ARGS__)

#define ADV_MAX_SIZE (251)
#define ADV_NAME_HEAD "doorbell"

#define ADV_TYPE_FLAGS                      (0x01)
#define ADV_TYPE_LOCAL_NAME                 (0x09)
#define ADV_TYPE_SERVICE_UUIDS_16BIT        (0x14)
#define ADV_TYPE_SERVICE_DATA               (0x16)
#define ADV_TYPE_MANUFACTURER_SPECIFIC      (0xFF)

#define BEKEN_COMPANY_ID                    (0x05F0)

#define GATTS_UUID                       (0xFE01)


#define BLE_GATT_CMD_CNT sizeof(s_gatt_commands)/sizeof(struct cli_command)

#define CMD_RSP_SUCCEED               "BLE GATTS RSP:OK\r\n"
#define CMD_RSP_ERROR                 "BLE GATTS RSP:ERROR\r\n"

#define GATT_SYNC_CMD_TIMEOUT_MS          4000
#define INVALID_HANDLE          0xFF

#define GATTS_SERVICE_UUID 					(0xFA00)

#define GATTS_CHARA_PROPERTIES_UUID 			(0xEA01)
#define GATTS_CHARA_N1_UUID 			(0xEA02)

#define GATTS_CHARA_N2_UUID 				(0xEA05)
#define GATTS_CHARA_N3_UUID 			(0xEA06)

#define DECL_PRIMARY_SERVICE_128     {0x00,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define DECL_CHARACTERISTIC_128      {0x03,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define DESC_CLIENT_CHAR_CFG_128     {0x02,0x29,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

#define BLE_GATTS_ADV_INTERVAL_MIN 120
#define BLE_GATTS_ADV_INTERVAL_MAX 160


#define UNKNOW_ACT_IDX         0xFFU

#define BLE_MAX_ACTV                  bk_ble_get_max_actv_idx_count()
#define BLE_MAX_CONN                  bk_ble_get_max_conn_idx_count()
static char ncserialNumber[64]= {0};
static int deviceType= 0;
static beken_semaphore_t gatt_sema = NULL;
static ble_err_t gatt_cmd_status = BK_ERR_BLE_SUCCESS;
static uint8_t gatt_conn_ind = INVALID_HANDLE;
static network_configure_callback g_network_callback=NULL;
static network_configure_ble_disconnect_callback g_disconnect_callback = NULL;
//N2
static uint8_t *s_v = 0;
static uint8_t s_len = 0;
//N3
static char *p_v = 0;
static uint8_t p_len = 0;
uint8_t notify_v[2];

void gatts_demo_event_notify(uint16_t opcode, int status);
enum
{
	PRF_TASK_ID_GATTS = 10,
	PRF_TASK_ID_MAX,
};

enum {
    	GATTS_IDX_SVC,
    	GATTS_IDX_CHAR_DECL,
    	GATTS_IDX_CHAR_VALUE,
	GATTS_IDX_CHAR_DESC,

	GATTS_IDX_CHAR_N1_DECL,
	GATTS_IDX_CHAR_N1_VALUE,

	GATTS_IDX_CHAR_N2_DECL,
	GATTS_IDX_CHAR_N2_VALUE,

	GATTS_IDX_CHAR_N3_DECL,
	GATTS_IDX_CHAR_N3_VALUE,

	GATTS_IDX_NB,
};

static ble_attm_desc_t gatts_service_db[GATTS_IDX_NB] = {
    //  Service Declaration
    [GATTS_IDX_SVC]        = {{GATTS_SERVICE_UUID & 0xFF, GATTS_SERVICE_UUID >> 8}, BK_BLE_PERM_SET(RD, ENABLE), 0, 0},

    [GATTS_IDX_CHAR_DECL]  = {DECL_CHARACTERISTIC_128,  BK_BLE_PERM_SET(RD, ENABLE), 0, 0},
    // Characteristic Value
    [GATTS_IDX_CHAR_VALUE] = {{GATTS_CHARA_PROPERTIES_UUID & 0xFF, GATTS_CHARA_PROPERTIES_UUID >> 8}, BK_BLE_PERM_SET(NTF, ENABLE), BK_BLE_PERM_SET(RI, ENABLE) | BK_BLE_PERM_SET(UUID_LEN, UUID_16), 128},
	//Client Characteristic Configuration Descriptor
	[GATTS_IDX_CHAR_DESC] = {DESC_CLIENT_CHAR_CFG_128, BK_BLE_PERM_SET(RD, ENABLE) | BK_BLE_PERM_SET(WRITE_REQ, ENABLE), 0, 0},

    //opreation
    [GATTS_IDX_CHAR_N1_DECL]  = {DECL_CHARACTERISTIC_128, BK_BLE_PERM_SET(RD, ENABLE), 0, 0},
    [GATTS_IDX_CHAR_N1_VALUE] = {{GATTS_CHARA_N1_UUID & 0xFF, GATTS_CHARA_N1_UUID >> 8, 0}, BK_BLE_PERM_SET(WRITE_REQ, ENABLE), BK_BLE_PERM_SET(RI, ENABLE) | BK_BLE_PERM_SET(UUID_LEN, UUID_16), 128},

    //s_v
    [GATTS_IDX_CHAR_N2_DECL]    = {DECL_CHARACTERISTIC_128, BK_BLE_PERM_SET(RD, ENABLE), 0, 0},
    [GATTS_IDX_CHAR_N2_VALUE]   = {{GATTS_CHARA_N2_UUID & 0xFF, GATTS_CHARA_N2_UUID >> 8, 0}, BK_BLE_PERM_SET(WRITE_REQ, ENABLE) | BK_BLE_PERM_SET(RD, ENABLE), BK_BLE_PERM_SET(RI, ENABLE) | BK_BLE_PERM_SET(UUID_LEN, UUID_16), 128},

    //p_v
    [GATTS_IDX_CHAR_N3_DECL]    = {DECL_CHARACTERISTIC_128, BK_BLE_PERM_SET(RD, ENABLE), 0, 0},
    [GATTS_IDX_CHAR_N3_VALUE]   = {{GATTS_CHARA_N3_UUID & 0xFF, GATTS_CHARA_N3_UUID >> 8, 0}, BK_BLE_PERM_SET(WRITE_REQ, ENABLE) | BK_BLE_PERM_SET(RD, ENABLE), BK_BLE_PERM_SET(RI, ENABLE) | BK_BLE_PERM_SET(UUID_LEN, UUID_16), 128},
};

static void ble_gatt_cmd_cb(ble_cmd_t cmd, ble_cmd_param_t *param){
    //LOGW("%s %d  cmd = %d\n", __func__, __LINE__,cmd);
    gatt_cmd_status = param->status;
    switch (cmd)
    {
        case BLE_CREATE_ADV:
        case BLE_SET_ADV_DATA:
        case BLE_SET_RSP_DATA:
        case BLE_START_ADV:
        case BLE_STOP_ADV:
        case BLE_CREATE_SCAN:
        case BLE_START_SCAN:
        case BLE_STOP_SCAN:
        case BLE_INIT_CREATE:
        case BLE_INIT_START_CONN:
        case BLE_INIT_STOP_CONN:
        case BLE_CONN_DIS_CONN:
        case BLE_CONN_UPDATE_PARAM:
        case BLE_DELETE_ADV:
        case BLE_DELETE_SCAN:
        case BLE_CONN_READ_PHY:
        case BLE_CONN_SET_PHY:
        case BLE_CONN_UPDATE_MTU:
            if (gatt_sema != NULL){
                rtos_set_semaphore( &gatt_sema );
	    }
            break;
        default:
            break;
    }

}
static uint16_t calculateCRC(uint8_t *data, int len, uint16_t polynomial, uint16_t initialValue) {
    uint16_t crc = initialValue;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= polynomial;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void get_wifi_info(void *data){
    LOGW("%s %d  \n", __func__, __LINE__);
    char* ssid = NULL;
    char* password = NULL;
    char* key = NULL;
    char* name = NULL;
    cJSON *root =  cJSON_Parse((char*)data);
    if(root!= NULL){
		cJSON *jssid = cJSON_GetObjectItem(root,"ssid");
		if(jssid!= NULL){
			ssid = cJSON_GetStringValue(jssid);
		}
		cJSON *jpassword = cJSON_GetObjectItem(root,"password");
		if(jpassword!= NULL){
			password = cJSON_GetStringValue(jpassword);
		}
		cJSON *jkey = cJSON_GetObjectItem(root,"key");
		if(jkey!= NULL){
			key = cJSON_GetStringValue(jkey);
		}
		cJSON *jname = cJSON_GetObjectItem(root,"name");
		if(jname!= NULL){
			name = cJSON_GetStringValue(jname);
		}
	        LOGW("%s %d wiri_ssid: %s\n", __func__, __LINE__,ssid);
	        LOGW("%s %d wifi_key: %s\n", __func__, __LINE__,password);
	        LOGW("%s %d Key: %s\n", __func__, __LINE__,key);
	        LOGW("%s %d ip: %s\n", __func__, __LINE__,name);   
	        if(g_network_callback!= NULL){
	 	  	g_network_callback(ssid,password,key,name);
	       }
	       cJSON_Delete(root);
    }
}
static void ble_gatts_notice_cb(ble_notice_t notice, void *param){
    //LOGW("%s %d  \n", __func__, __LINE__);
    char *value;
    switch (notice) {
    case BLE_5_STACK_OK:
        BLEGATTS_LOGI("%s %d BLE_5_STACK_OK ble stack ok",__func__, __LINE__);
        break;
    case BLE_5_WRITE_EVENT: {
        ble_write_req_t *w_req = (ble_write_req_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_WRITE_EVENT write_cb:conn_idx:%d, prf_id:%d, att_idx:%d, len:%d, data[0]:0x%02x\r\n",__func__, __LINE__,
                w_req->conn_idx, w_req->prf_id, w_req->att_idx, w_req->len, w_req->value[0]);
//#if (CONFIG_BTDM_5_2)
        value = (char *)(w_req->value);
		if (bk_ble_get_controller_stack_type() == BK_BLE_CONTROLLER_STACK_TYPE_BTDM_5_2
            && w_req->prf_id == PRF_TASK_ID_GATTS) {
	   switch(w_req->att_idx)
            {
            case GATTS_IDX_CHAR_DECL:
		LOGW("%s %d BLE_5_WRITE_EVENT GATTS_IDX_CHAR_DECL\n", __func__, __LINE__);
                break;
            case GATTS_IDX_CHAR_VALUE:
		LOGW("%s %d BLE_5_WRITE_EVENT GATTS_IDX_CHAR_VALUE\n", __func__, __LINE__);
                break;
	    case GATTS_IDX_CHAR_DESC: {
			BLEGATTS_LOGI("write notify: %02X %02X, length: %d\n", w_req->value[0], w_req->value[1], w_req->len);
			break;
	    }

	    case GATTS_IDX_CHAR_N1_DECL:
		LOGW("%s %d BLE_5_WRITE_EVENT GATTS_IDX_CHAR_N1_DECL\n", __func__, __LINE__);
		break;

	   case GATTS_IDX_CHAR_N1_VALUE:
		LOGW("%s %d BLE_5_WRITE_EVENT GATTS_IDX_CHAR_N1_VALUE\n", __func__, __LINE__);
		break;

            case GATTS_IDX_CHAR_N2_DECL:
		LOGW("%s %d BLE_5_WRITE_EVENT GATTS_IDX_CHAR_N2_DECL\n", __func__, __LINE__);
                break;
            case GATTS_IDX_CHAR_N2_VALUE:{
		   
		    if (s_v != NULL){
			  os_free(s_v);
			  s_v = NULL;
		    }
                    int data_len = value[2] | value[3] << 8;
                    //int data_len = (value[2] & 0xFF) | (value[3] & 0xFF) << 8;//+++
		    LOGW("%s %d BLE_5_WRITE_EVENT GATTS_IDX_CHAR_N2_VALUE len = %d  data_len = %d\n", __func__, __LINE__,w_req->len,data_len);
                    if(w_req->len < 7 || data_len > w_req->len ){ //|| data_len < (w_req->len - 6)
                        os_printf("+++value[3]:%#x+++\n", __func__, __LINE__,value[3]);//+++
                        os_printf("%s %d +++w_req->len:%d,data_len:%d,+++\n", __func__, __LINE__,w_req->len,data_len);//+++
                       break;
                    }

                        os_printf("666 value[3]:%#x 666\n", __func__, __LINE__,value[3]);//+++
                        os_printf("%s %d 666, w_req->len:%d,data_len:%d   ,666成功\n", __func__, __LINE__,w_req->len,data_len);//+++
                    if(1 == value[0]){
                        //os_printf("=======================value111====================\r\n");
                        cJSON *jresps = cJSON_CreateObject();
                        cJSON_AddNumberToObject(jresps,"type",deviceType);                      //type
                        cJSON_AddStringToObject(jresps,"uuid",ncserialNumber);                  //uuid 
                        char *msg = cJSON_Print(jresps);
                        s_len = strlen(msg);

                        uint16_t crc = calculateCRC((uint8_t *)msg,s_len,0x07BA, 0xFFFF);

                        s_v = (uint8_t *)os_malloc(s_len + 7);
                        os_memset((void *)s_v , 0, s_len + 7);
                        os_memcpy((void *)(s_v + 6), (void *)msg, s_len);
                        #if 1
                        s_v[0] = 1;
                        s_v[1] = 1;
                        s_v[2] = s_len;
                        s_v[3] = s_len >> 8;
                        s_v[4] = crc;
                        s_v[5] = crc >> 8;
                        #endif
			if(msg!= NULL){
			   os_free(msg);
			   msg = NULL;
			}
                        cJSON_Delete(jresps);
                    }else if(2 == value[0]){
                        //os_printf("=======================value222====================\r\n");
                        cJSON *jresps = cJSON_CreateObject();
                        cJSON_AddNumberToObject(jresps,"code",200);                                             //ok
                        char *msg = cJSON_Print(jresps);                      
                        s_len = strlen(msg);
                        uint16_t crc = calculateCRC((uint8_t *)msg,s_len,0x07BA, 0xFFFF);
                        s_v = (uint8_t *)os_malloc(s_len + 7);
                        os_memset((void *)s_v, 0, s_len + 7);
	                    os_memcpy((void *)(s_v + 6), (void *)msg, s_len);
                        cJSON_Delete(jresps);
                        
                        get_wifi_info((void *)(value + 6));
                        #if 1
                        s_v[0] = 2;
                        s_v[1] = 1;
                        s_v[2] = s_len;
                        s_v[3] = s_len >> 8;
                        s_v[4] = crc;
                        s_v[5] = crc >> 8;
                        #endif
 			if(msg!= NULL){//+++
			   os_free(msg);
			   msg = NULL;
			}                       
                    }else{
                        os_printf("value[0] = %#x\n",value[0]);
                    }
		   LOGW("%s %d write N2: %s, length: %d\n", __func__, __LINE__, s_v, s_len);
		}
                break;

            case GATTS_IDX_CHAR_N3_DECL:
		LOGW("%s %d BLE_5_WRITE_EVENT GATTS_IDX_CHAR_N3_DECL\n", __func__, __LINE__);
                break;
            case GATTS_IDX_CHAR_N3_VALUE:{
		LOGW("%s %d BLE_5_WRITE_EVENT GATTS_IDX_CHAR_N3_DECL\n", __func__, __LINE__);
		if (p_v != NULL){
				os_free(p_v);
				p_v = NULL;
		}
		p_len = w_req->len;
		p_v = os_malloc(p_len + 1);
		os_memset((uint8_t *)p_v, 0, p_len + 1);
	        os_memcpy((uint8_t *)p_v, w_req->value, p_len);
		BLEGATTS_LOGI("%s %d write N3: %s, length: %d\n", __func__, __LINE__,p_v, p_len);
		}
		break;

            default:
                break;
            }
        }
//#endif
        break;
    }
    case BLE_5_READ_EVENT: {
        ble_read_req_t *r_req = (ble_read_req_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_READ_EVENT read_cb:conn_idx:%d, prf_id:%d, att_idx:%d\r\n",__func__, __LINE__,
                r_req->conn_idx, r_req->prf_id, r_req->att_idx);

        if (r_req->prf_id == PRF_TASK_ID_GATTS) {
            switch(r_req->att_idx)
            {
                case GATTS_IDX_CHAR_DECL:
		    LOGW("%s %d BLE_5_READ_EVENT GATTS_IDX_CHAR_DECL\n", __func__, __LINE__);
                    break;
                case GATTS_IDX_CHAR_VALUE:
		    LOGW("%s %d BLE_5_READ_EVENT GATTS_IDX_CHAR_VALUE \n", __func__, __LINE__);
                    break;
		case GATTS_IDX_CHAR_DESC:
		    LOGW("%s %d BLE_5_READ_EVENT GATTS_IDX_CHAR_DESC\n", __func__, __LINE__);
		    bk_ble_read_response_value(r_req->conn_idx, sizeof(notify_v), &notify_v[0], r_req->prf_id, r_req->att_idx);
		    break;

                case GATTS_IDX_CHAR_N2_DECL:
		     LOGW("%s %d BLE_5_READ_EVENT GATTS_IDX_CHAR_N2_DECL\n", __func__, __LINE__);
                    break;
                case GATTS_IDX_CHAR_N2_VALUE:
		     LOGW("%s %d BLE_5_READ_EVENT GATTS_IDX_CHAR_N2_VALUE\n", __func__, __LINE__);
                    //确保 s_v 已经被正确分配了内存
                    if (s_v == NULL) {
                        BLEGATTS_LOGE(" %s %d s_v is NULL, cannot read N2 value\n", __func__, __LINE__);
                        break;
                    }

                    // 确保 s_len 有效
                    if (s_len < 0 || s_len > (sizeof(s_v) - 7)) {
                        BLEGATTS_LOGE(" %s %d s_len is invalid: %d, max allowed: %zu\n", __func__, __LINE__, s_len, sizeof(s_v) - 7);
                        break;
                    }             
                     bk_ble_read_response_value(r_req->conn_idx, s_len + 7, (uint8_t*)s_v, r_req->prf_id, r_req->att_idx);
                     BLEGATTS_LOGI(" %s %d  BLE_5_READ_EVENT GATTS_IDX_CHAR_N2_VALUE read N2: %s, length: %d\n",__func__, __LINE__, s_v, s_len + 7);
                    break;

                case GATTS_IDX_CHAR_N3_DECL:
		     LOGW("%s %d BLE_5_READ_EVENT GATTS_IDX_CHAR_N3_DECL\n", __func__, __LINE__);
                    break;
                case GATTS_IDX_CHAR_N3_VALUE:
		    LOGW(" %s %d BLE_5_READ_EVENT GATTS_IDX_CHAR_N3_VALUE \n", __func__, __LINE__);
                    bk_ble_read_response_value(r_req->conn_idx, p_len, (uint8_t*)p_v, r_req->prf_id, r_req->att_idx);
		    BLEGATTS_LOGI("%s %d GATTS_IDX_CHAR_N2_VALUE read N3: %s, length: %d\n",__func__, __LINE__, p_v, p_len);
                    break;

                default:
                    break;
            }
        }
        break;
    }
    case BLE_5_REPORT_ADV: {
        break;
    }
    case BLE_5_MTU_CHANGE: {
        ble_mtu_change_t *m_ind = (ble_mtu_change_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_MTU_CHANGE  m_ind:conn_idx:%d, mtu_size:%d\r\n", __func__,  __LINE__,m_ind->conn_idx, m_ind->mtu_size);
        break;
    }
    case BLE_5_CONNECT_EVENT: {
            ble_conn_ind_t *c_ind = (ble_conn_ind_t *)param;
            BLEGATTS_LOGI("%s %d BLE_5_CONNECT_EVENT c_ind:conn_idx:%d, addr_type:%d, peer_addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",__func__, __LINE__, 
                    c_ind->conn_idx, c_ind->peer_addr_type, c_ind->peer_addr[0], c_ind->peer_addr[1],
                    c_ind->peer_addr[2], c_ind->peer_addr[3], c_ind->peer_addr[4], c_ind->peer_addr[5]);
            gatt_conn_ind = c_ind->conn_idx;
        break;
    }
    case BLE_5_DISCONNECT_EVENT: {
        ble_discon_ind_t *d_ind = (ble_discon_ind_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_DISCONNECT_EVENT d_ind:conn_idx:%d,reason:%d\r\n", __func__, __LINE__,d_ind->conn_idx, d_ind->reason);
	if(g_disconnect_callback !=NULL){
		g_disconnect_callback(d_ind->conn_idx);
	}
	gatt_conn_ind = ~0;
	if (p_v != NULL){
				os_free(p_v);
				p_v = NULL;
	}
	if (s_v != NULL){
			  os_free(s_v);
			  s_v = NULL;
        }
        break;
    }
    case BLE_5_ATT_INFO_REQ: {
        ble_att_info_req_t *a_ind = (ble_att_info_req_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_ATT_INFO_REQ a_ind:conn_idx:%d\r\n", __func__, __LINE__,a_ind->conn_idx);
        a_ind->length = 128;
        a_ind->status = BK_ERR_BLE_SUCCESS;
        break;
    }
    case BLE_5_CREATE_DB: {
         ble_create_db_t *cd_ind = (ble_create_db_t *)param;
         BLEGATTS_LOGI("%s %d BLE_5_CREATE_DB cd_ind:prf_id:%d, status:%d\r\n", __func__, __LINE__,cd_ind->prf_id, cd_ind->status);
         gatt_cmd_status = cd_ind->status;
         if (gatt_sema != NULL){
                rtos_set_semaphore( &gatt_sema );
	  }
        break;
    }
    case BLE_5_INIT_CONNECT_EVENT: {
        ble_conn_ind_t *c_ind = (ble_conn_ind_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_INIT_CONNECT_EVENT:conn_idx:%d, addr_type:%d, peer_addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",__func__, __LINE__,
                c_ind->conn_idx, c_ind->peer_addr_type, c_ind->peer_addr[0], c_ind->peer_addr[1],
                c_ind->peer_addr[2], c_ind->peer_addr[3], c_ind->peer_addr[4], c_ind->peer_addr[5]);
        break;
    }
    case BLE_5_INIT_DISCONNECT_EVENT: {
        ble_discon_ind_t *d_ind = (ble_discon_ind_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_INIT_DISCONNECT_EVENT:conn_idx:%d,reason:%d\r\n",__func__, __LINE__, d_ind->conn_idx, d_ind->reason);
	if(g_disconnect_callback !=NULL){
		g_disconnect_callback(d_ind->conn_idx);
	}
        break;
    }
    case BLE_5_SDP_REGISTER_FAILED:
        BLEGATTS_LOGI("%s %d BLE_5_SDP_REGISTER_FAILED\r\n",__func__, __LINE__);
        break;
    case BLE_5_READ_PHY_EVENT: {
        ble_read_phy_t *phy_param = (ble_read_phy_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_READ_PHY_EVENT:tx_phy:0x%02x, rx_phy:0x%02x\r\n",__func__, __LINE__,phy_param->tx_phy, phy_param->rx_phy);
        break;
    }
    case BLE_5_TX_DONE:
        break;
    case BLE_5_CONN_UPDATA_EVENT: {
        ble_conn_param_t *updata_param = (ble_conn_param_t *)param;
        BLEGATTS_LOGI("%s %d BLE_5_CONN_UPDATA_EVENT:conn_interval:0x%04x, con_latency:0x%04x, sup_to:0x%04x\r\n", __func__, __LINE__,updata_param->intv_max,
        updata_param->con_latency, updata_param->sup_to);
        break;
    }
    case BLE_5_PAIRING_REQ:
        LOGW("%s %d BLE_5_PAIRING_REQ\r\n",__func__, __LINE__);
        break;

    case BLE_5_PARING_PASSKEY_REQ:
        LOGW("%s %d BLE_5_PARING_PASSKEY_REQ\r\n",__func__, __LINE__);
        break;

    case BLE_5_ENCRYPT_EVENT:
        LOGW("%s %d BLE_5_ENCRYPT_EVENT\r\n",__func__, __LINE__);
        break;

    case BLE_5_PAIRING_SUCCEED:
        LOGW("%s %d BLE_5_PAIRING_SUCCEED\r\n",__func__, __LINE__);
        break;
    default:
        break;
    }
}


void network_configure_start(char *serialNumber,int type,network_configure_callback callback,network_configure_ble_disconnect_callback disconnect_callback){
    LOGW("%s %d\n", __func__, __LINE__);
    snprintf(ncserialNumber,sizeof(ncserialNumber),"%s",serialNumber);
    deviceType= type;
    g_network_callback = callback;
    g_disconnect_callback = disconnect_callback;
    bt_err_t ret = BK_FAIL;
    struct bk_ble_db_cfg ble_db_cfg;
    ret = rtos_init_semaphore(&gatt_sema, 1);
    if(ret != BK_OK){
        LOGE("%s %d gatts init sema fial \n",__func__, __LINE__);
        return;
    }

    
    bk_ble_set_notice_cb(ble_gatts_notice_cb);
    ble_db_cfg.att_db = (ble_attm_desc_t *)gatts_service_db;
    ble_db_cfg.att_db_nb = GATTS_IDX_NB;
    ble_db_cfg.prf_task_id = PRF_TASK_ID_GATTS;
    ble_db_cfg.start_hdl = 0;
    ble_db_cfg.svc_perm = BK_BLE_PERM_SET(SVC_UUID_LEN, UUID_16);
    ble_db_cfg.uuid[0] = GATTS_SERVICE_UUID & 0xFF;
    ble_db_cfg.uuid[1] = GATTS_SERVICE_UUID >> 8;
    ret = bk_ble_create_db(&ble_db_cfg);

    if (ret != BK_ERR_BLE_SUCCESS){
        LOGE("%s %d create gatt db failed %d\n", __func__, __LINE__,ret);
        goto error;
    }
    ret = rtos_get_semaphore(&gatt_sema, GATT_SYNC_CMD_TIMEOUT_MS);
    if (ret != BK_OK){
        LOGE("%s %d wait semaphore failed at %d\n", __func__, __LINE__,ret);
        goto error;
    }else{
        LOGW("%s %d create gatt db success\n",__func__, __LINE__);
    }

    //set adv data
    uint8_t adv_data[ADV_MAX_SIZE] = {0};
	uint8_t adv_index = 0;
	uint8_t len_index = 0;
	uint8_t mac[6];
	/* flags */
	len_index = adv_index;
	adv_data[adv_index++] = 0x00;
	adv_data[adv_index++] = ADV_TYPE_FLAGS;
	adv_data[adv_index++] = 0x06;
	adv_data[len_index] = 2;

	/* local name */
	bk_bluetooth_get_address(mac);

	len_index = adv_index;
	adv_data[adv_index++] = 0x00;
	adv_data[adv_index++] = ADV_TYPE_LOCAL_NAME;

	ret = sprintf((char *)&adv_data[adv_index], "%s_%02X%02X%02X",
	              ADV_NAME_HEAD, mac[0], mac[1], mac[2]);

    	bk_ble_appm_set_dev_name(ret, &adv_data[adv_index]);
    	LOGW("%s %d dev_name:%s, ret:%d \n", __func__, __LINE__,(char *)&adv_data[adv_index], ret);
	adv_index += ret;
	adv_data[len_index] = ret + 1;

	/* 16bit uuid */
	len_index = adv_index;
	adv_data[adv_index++] = 0x00;
	adv_data[adv_index++] = ADV_TYPE_SERVICE_DATA;
	adv_data[adv_index++] = GATTS_UUID & 0xFF;
	adv_data[adv_index++] = GATTS_UUID >> 8;
	adv_data[len_index] = 3;

	/* manufacturer */
	len_index = adv_index;
	adv_data[adv_index++] = 0x00;
	adv_data[adv_index++] = ADV_TYPE_MANUFACTURER_SPECIFIC;
	adv_data[adv_index++] = BEKEN_COMPANY_ID & 0xFF;
	adv_data[adv_index++] = BEKEN_COMPANY_ID >> 8;
	adv_data[len_index] = 3;
    	//BLEGATTS_LOGI("adv data length :%d \n", adv_index);

	/* set adv paramters */
    	ble_adv_param_t adv_param;
	int actv_idx = 0;
	os_memset(&adv_param, 0, sizeof(ble_adv_param_t));
	adv_param.chnl_map = ADV_ALL_CHNLS;
	adv_param.adv_intv_min = BLE_GATTS_ADV_INTERVAL_MIN;
	adv_param.adv_intv_max = BLE_GATTS_ADV_INTERVAL_MAX;
	adv_param.own_addr_type = OWN_ADDR_TYPE_PUBLIC_OR_STATIC_ADDR;
	adv_param.adv_type = ADV_TYPE_LEGACY;
	adv_param.adv_prop = ADV_PROP_CONNECTABLE_BIT | ADV_PROP_SCANNABLE_BIT;
	adv_param.prim_phy = INIT_PHY_TYPE_LE_1M;
	adv_param.second_phy = INIT_PHY_TYPE_LE_1M;
	ret = bk_ble_create_advertising(actv_idx, &adv_param, ble_gatt_cmd_cb);

	if (ret != BK_ERR_BLE_SUCCESS){
		BLEGATTS_LOGE("%s %d config adv paramters failed %d\n", __func__, __LINE__,ret);
		goto error;
	}
	ret = rtos_get_semaphore(&gatt_sema, GATT_SYNC_CMD_TIMEOUT_MS);
	if (ret != BK_OK){
		BLEGATTS_LOGE("%s %d wait semaphore failed at %d \n",  __func__, __LINE__,ret);
		goto error;
	}else{
		BLEGATTS_LOGI("%s %d set adv paramters success\n", __func__, __LINE__);
	}
	/* set adv paramters */
	ret = bk_ble_set_adv_data(actv_idx, adv_data, adv_index, ble_gatt_cmd_cb);
	if (ret != BK_ERR_BLE_SUCCESS){
		BLEGATTS_LOGE("%s %d set adv data failed %d\n",  __func__, __LINE__,ret);
		goto error;
	}
	ret = rtos_get_semaphore(&gatt_sema, GATT_SYNC_CMD_TIMEOUT_MS);
	if (ret != BK_OK){
		BLEGATTS_LOGE("%s %d wait semaphore failed at %d\n", __func__, __LINE__,ret);
		goto error;
	}else{
		BLEGATTS_LOGI("%s %d set adv data success\n", __func__, __LINE__);
	}
	/* sart adv */
	ret = bk_ble_start_advertising(actv_idx, 0, ble_gatt_cmd_cb);

	if (ret != BK_ERR_BLE_SUCCESS){
		BLEGATTS_LOGE("%s %d start adv failed %d\n", __func__, __LINE__,ret);
		goto error;
	}
	ret = rtos_get_semaphore(&gatt_sema, GATT_SYNC_CMD_TIMEOUT_MS);
	if (ret != BK_OK){
		BLEGATTS_LOGE("%s %d wait semaphore failed at %d \n", __func__, __LINE__,ret);
		goto error;
	}else{
		BLEGATTS_LOGI("%s %d sart adv success\n", __func__, __LINE__);
	}
       BLEGATTS_LOGI("%s success\n", __func__);
    return;
error:
    BLEGATTS_LOGE("%s fail \n", __func__);
}

void network_configure_stop_advertising(void){	
	int actv_idx = 0;
        int err = BK_FAIL;
	LOGW("%s %d  \n", __func__, __LINE__);
        actv_idx = bk_ble_find_actv_state_idx_handle(BLE_ACTV_ADV_STARTED);
        if (actv_idx == BLE_MAX_ACTV){
            BLEGATTS_LOGE("%s %d ble is advertising or create a new ble adv\n", __func__, __LINE__);
              return ;
        }
        err = bk_ble_stop_advertising(actv_idx, ble_gatt_cmd_cb);
        if(err != BK_OK){
            BLEGATTS_LOGE("%s %d start or stop adv fail \n", __func__, __LINE__);
              return ;
        }
        err = rtos_get_semaphore(&gatt_sema, GATT_SYNC_CMD_TIMEOUT_MS);

    	if (err != BK_OK){
    		BLEGATTS_LOGE("%s %d wait semaphore failed at %d\n", __func__, __LINE__,err);
    		  return ;
    	}else{
    		BLEGATTS_LOGI("%s %d sart adv success\n", __func__, __LINE__);
    	}	

}
void network_configure_restart(void){
        int actv_idx = 0;
        int err = BK_FAIL;
        LOGW("%s %d  \n", __func__, __LINE__);
        actv_idx = bk_ble_find_actv_state_idx_handle(BLE_ACTV_ADV_CREATED);
        if (actv_idx == BLE_MAX_ACTV){
            BLEGATTS_LOGE("%s %d ble is advertising or create a new ble adv\n", __func__, __LINE__);
            return ;
        }

        err = bk_ble_start_advertising(actv_idx, 0, ble_gatt_cmd_cb);
  
        if(err != BK_OK){
            BLEGATTS_LOGE("%s %d start or stop adv fail \n", __func__, __LINE__);
              return ;
        }
        err = rtos_get_semaphore(&gatt_sema, GATT_SYNC_CMD_TIMEOUT_MS);

    	if (err != BK_OK){
    		BLEGATTS_LOGE("%s %d wait semaphore failed at %d\n", __func__, __LINE__,err);
    		  return ;
    	}else{
    		BLEGATTS_LOGI("%s %d sart adv success\n", __func__, __LINE__);
    	}

}
void gatts_demo_event_notify(uint16_t opcode, int status){
	uint8_t data[] ={
		opcode & 0xFF, 
		opcode >> 8,
		status & 0xFF,                                                          /* status           */
		0, 0,                                                                   /* payload length   */
	};
    if(gatt_conn_ind == INVALID_HANDLE){
        BLEGATTS_LOGI("%s %d BLE is disconnected, can not send data !!!\r\n", __func__, __LINE__);
    }else{
        bk_ble_send_noti_value(gatt_conn_ind, sizeof(data), data, PRF_TASK_ID_GATTS, GATTS_IDX_CHAR_VALUE);
    }
}
#endif
