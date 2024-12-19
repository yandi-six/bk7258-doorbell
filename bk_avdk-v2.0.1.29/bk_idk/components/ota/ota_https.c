#include "sdkconfig.h"
#include <string.h>
#include "cli.h"
#include "components/system.h"
#include "components/log.h"
#include "driver/flash.h"
#include "modules/ota.h"
#include "utils_httpc.h"
#include "modules/wifi.h"
#include "bk_https.h"

#if CONFIG_PSA_MBEDTLS
#include "psa/crypto.h"
#endif


#define TAG "HTTPS_OTA"

extern UINT8  ota_flag ;
#ifdef CONFIG_HTTP_AB_PARTITION
extern part_flag update_part_flag;
#endif

#define HTTPS_INPUT_SIZE   (5120)

/* this crt for url https://docs.bekencorp.com , support test*/
// const char ca_crt_rsa[] = {
// "-----BEGIN CERTIFICATE-----\r\n"
// "MIIGbzCCBFegAwIBAgIRAInZWbILnINXOGsRKfqm8u0wDQYJKoZIhvcNAQEMBQAw\r\n"
// "SzELMAkGA1UEBhMCQVQxEDAOBgNVBAoTB1plcm9TU0wxKjAoBgNVBAMTIVplcm9T\r\n"
// "U0wgUlNBIERvbWFpbiBTZWN1cmUgU2l0ZSBDQTAeFw0yMzAxMTcwMDAwMDBaFw0y\r\n"
// "MzA0MTcyMzU5NTlaMBoxGDAWBgNVBAMMDyouYmVrZW5jb3JwLmNvbTCCASIwDQYJ\r\n"
// "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAK2u5m6nnEETeJ+Qdxv8k9Pb6bKxs1Pd\r\n"
// "DjowS/59+U7LMOZW/5zNzyfe40fEHyEDH2PFS1+VDvlRVX7PRYdIkpGfEfHEKo5k\r\n"
// "jT2UQW7NIZ4jcHXLw+htnhCQHCjM4mvc7jOnkidTkEx/1A9cug75C/UwaDq7MW0G\r\n"
// "aX/8fl69tt3pQFhdUXb9lC56zjcBlDm5gFtElORCJ5zdvBaVcdl2Lj2AuO5B3fXq\r\n"
// "Dr44BgoyLFWtxnPTYJECaLYBrPCBW1orpEmj3XbtCuNkmNStlqRXr6tbZtxQikgb\r\n"
// "zimtkvXDXlO29jwb65OrsUIsY5synz16XaJ6MKb/6ogeBb4hdTSxLWkCAwEAAaOC\r\n"
// "An0wggJ5MB8GA1UdIwQYMBaAFMjZeGii2Rlo1T1y3l8KPty1hoamMB0GA1UdDgQW\r\n"
// "BBSyAThY+hOxGkRuvG0LEITFPUFVKDAOBgNVHQ8BAf8EBAMCBaAwDAYDVR0TAQH/\r\n"
// "BAIwADAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwSQYDVR0gBEIwQDA0\r\n"
// "BgsrBgEEAbIxAQICTjAlMCMGCCsGAQUFBwIBFhdodHRwczovL3NlY3RpZ28uY29t\r\n"
// "L0NQUzAIBgZngQwBAgEwgYgGCCsGAQUFBwEBBHwwejBLBggrBgEFBQcwAoY/aHR0\r\n"
// "cDovL3plcm9zc2wuY3J0LnNlY3RpZ28uY29tL1plcm9TU0xSU0FEb21haW5TZWN1\r\n"
// "cmVTaXRlQ0EuY3J0MCsGCCsGAQUFBzABhh9odHRwOi8vemVyb3NzbC5vY3NwLnNl\r\n"
// "Y3RpZ28uY29tMIIBBgYKKwYBBAHWeQIEAgSB9wSB9ADyAHcArfe++nz/EMiLnT2c\r\n"
// "Hj4YarRnKV3PsQwkyoWGNOvcgooAAAGFvuMP6AAABAMASDBGAiEAz8Nxhittofny\r\n"
// "/mZbg/tSnOHCEZxLdr7/A42OhEC/z8UCIQCDzRa4/lkxdRCbU0YzWyJncaZNJVwl\r\n"
// "uwEZa7yLbzKIcwB3AHoyjFTYty22IOo44FIe6YQWcDIThU070ivBOlejUutSAAAB\r\n"
// "hb7jD+8AAAQDAEgwRgIhALZ8PcYB8///ouVATvL5+YZMf03lCudhszT8U7rKm9PK\r\n"
// "AiEA5kDQyDhvYAooxVhG2EvXtz+vDq/x8ArGawsXSPDRAP8wGgYDVR0RBBMwEYIP\r\n"
// "Ki5iZWtlbmNvcnAuY29tMA0GCSqGSIb3DQEBDAUAA4ICAQAF5qAQUFl0z7zpDPES\r\n"
// "7bLc7Vh+mA+BgLzbDzwVXXZG9I5a2sO9eqy/FW74FzZtvzaBfem3YwOrbrzNNAZ+\r\n"
// "HQdDfq3vBzGlCFLIma8iZS3NHHrxHIRZlyXKWit/xXH0zelAwEpee8wTUguDt0wP\r\n"
// "8NuI3jMevsJJix0a4Y/R0SdTeW8yCSZXddi8sEkOM2YCMpwN016jdlNeN9w1NKwT\r\n"
// "oZpVQLOD+L2+1+H4dlwoc/ZsByCT00WYFLrOUlANNrWT8Jjar8b1SBuqiIft2YFe\r\n"
// "8IC1YeJQncbnyY/X6gI3Z1eKTjTLELVu1keGtArEuRHRO7+5+1cglpZwNCZc/RAW\r\n"
// "SUlAsLbmOP8e8gHFFKO8VR7txempsWPal09bfKSnukhLCW6XRUWAOm39OriiP9rR\r\n"
// "VXrBLnohwOGh2IvdALc0jOriz+iD08FBojnh8v9VV8PrYoqjwCTyme0X2Gi3gGJL\r\n"
// "8UzHYILwJ8NIxFIZQbdF5q0gi4JqM38+GSf70w6KoAjiFiW6z4oUjTrbQGx2bOd2\r\n"
// "4gstpMm5SZAb/A4tWtRvZBS1T1PcaAHtplr2CWMZGW1QfDGX5duqOJ9f79kifwJH\r\n"
// "uw/FqCeOPgYmxV2lk2JalIOOhHrAKNbCVahdWlum5XDSrhsu9bhorLelifPwPrQE\r\n"
// "clib3BcxKZX9qK4A6FAATghuSQ==\r\n"
// "-----END CERTIFICATE-----\r\n"
// };
const char ca_crt_rsa[] = {
"-----BEGIN CERTIFICATE-----\r\n"
"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\r\n"
"-----END CERTIFICATE-----\r\n"
};


//+++ neway07 root ca
const char root_crt_rsa[] = {
"-----BEGIN CERTIFICATE-----\r\n"
"MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\r\n"
"MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\r\n"
"GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\r\n"
"YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\r\n"
"MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\r\n"
"BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\r\n"
"GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\r\n"
"ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\r\n"
"BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\r\n"
"3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\r\n"
"YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\r\n"
"rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\r\n"
"ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\r\n"
"oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\r\n"
"MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\r\n"
"QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\r\n"
"b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\r\n"
"AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\r\n"
"GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\r\n"
"Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\r\n"
"G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\r\n"
"l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\r\n"
"smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\r\n"
"-----END CERTIFICATE-----\r\n"
};

// const char intermediate_crt_rsa[] = {
// "-----BEGIN CERTIFICATE-----\r\n"
// "MIIF4jCCA8qgAwIBAgIRANVuJGyU7WOrsUbvwZa2T7AwDQYJKoZIhvcNAQEMBQAw\r\n"
// "gYgxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpOZXcgSmVyc2V5MRQwEgYDVQQHEwtK\r\n"
// "ZXJzZXkgQ2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMS4wLAYD\r\n"
// "VQQDEyVVU0VSVHJ1c3QgUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTIw\r\n"
// "MDEwODAwMDAwMFoXDTMwMDEwNzIzNTk1OVowXDELMAkGA1UEBhMCQ04xGjAYBgNV\r\n"
// "BAoTEVdvVHJ1cyBDQSBMaW1pdGVkMTEwLwYDVQQDDChXb1RydXMgRFYgU2VydmVy\r\n"
// "IENBICBbRun by IHRoZSBJc3N1ZXJdMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A\r\n"
// "MIIBCgKCAQEA7IE0rmVTVdRdNOzK1jsR/VppyukZ/XbQgakJHOhg6XGDsiHe/l5B\r\n"
// "3PxyXw18jEdN+7YxP0qsGz+HlQbsQh6XlwIyjpz/2gFMiqa7y1v+dHOgj6xNOF5a\r\n"
// "oaPm/Qhb0N+JYQidgaC+1Zp6W+YeC736rzCMr9vL1Usa3QLzRoQEo0DzbG4sPeP1\r\n"
// "US0Ia/i8o6szArH+DAcvrzCZ2kkpTScQ9QfOsvkMBP1W2otICdKUyZHaBc+ztTAd\r\n"
// "ovSlOR+GPf29dYfGQkZAp0tffIRw/na3WB86WGZPpNFfo2QxxsHYoL3oSWKfSWTY\r\n"
// "FgW22J8eA03TFHYowm/NqYuJ7GW553HppQIDAQABo4IBcDCCAWwwHwYDVR0jBBgw\r\n"
// "FoAUU3m/WqorSs9UgOHYm8Cd8rIDZsswHQYDVR0OBBYEFJmbLfaL8KPbidSe++V0\r\n"
// "L2jSkE/kMA4GA1UdDwEB/wQEAwIBhjASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1Ud\r\n"
// "JQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAiBgNVHSAEGzAZMA0GCysGAQQBsjEB\r\n"
// "AgIWMAgGBmeBDAECATBQBgNVHR8ESTBHMEWgQ6BBhj9odHRwOi8vY3JsLnVzZXJ0\r\n"
// "cnVzdC5jb20vVVNFUlRydXN0UlNBQ2VydGlmaWNhdGlvbkF1dGhvcml0eS5jcmww\r\n"
// "cQYIKwYBBQUHAQEEZTBjMDoGCCsGAQUFBzAChi5odHRwOi8vY3J0LnVzZXJ0cnVz\r\n"
// "dC5jb20vVVNFUlRydXN0UlNBQUFBQ0EuY3J0MCUGCCsGAQUFBzABhhlodHRwOi8v\r\n"
// "b2NzcC51c2VydHJ1c3QuY29tMA0GCSqGSIb3DQEBDAUAA4ICAQB5t8v3uYzHa4EL\r\n"
// "0rOb9g/YAmptUbILcBMKk1x188ucsGVPaG1DG9bpVamxbmCtFA1MlrA7iUC8SGop\r\n"
// "KBnuWFsNKiC7jCbRoahT1/FSwFsSuDlDmOjr1MqDXE+or08UkXsJB57XxXxdVOPl\r\n"
// "DcZHII4qHi1XKK4iurMqb+kbdpAWadyfidRRCGPopYCVYLLYhRJgpFGtfr6Gk8N0\r\n"
// "j81jq/7QbN0dRSDzMNdadKTc7c3+i9fIrXj79lV5Wvva+OL7nh8MxQhG1Ekek7Rv\r\n"
// "en++jSZvaEhCrMsSedFTA/aIy7oJg85tfglF2ybK61HsobjYzdDNICKJlIm4chlA\r\n"
// "XIDDqw2mw0Kz2snrkp9dpvMBqahF/Uy1kHzPcrq1/w5OqZWAuDKxZ68PuZ/ME2hI\r\n"
// "YbIDG9dWT6Y7eqtjQ2TmAQbOqdAG2LeikPMl2DMrPEa4lcKJzsFbHfHAW3hVgPSQ\r\n"
// "hRfS4TtbNnxijbsp8GguMHxP2R7dpAAYybwfZdXP7WYAnwEr1mzIf0Y3J0m7GDyX\r\n"
// "JhaflN3G2wIm2HzRd39NvnDRmFEraqui/YYO9ym0pwq1d0S+bGG6876QCto0u3Cg\r\n"
// "ItFh3Za2ZeIY+g5mWrejSaDs9LT7eu44iCyebfgekdMRqFeCuGAsJdsun3LOHHJo\r\n"
// "tCVPRjyFg9NDeJeMa4Z8QuXAXLd9cw==\r\n"
// "-----END CERTIFICATE-----\r\n"
// };




bk_http_client_handle_t bk_https_client_flash_init(bk_http_input_t config)
{
	bk_http_client_handle_t client = NULL;

	ota_flag = 1;
#if CONFIG_SYSTEM_CTRL
	bk_wifi_ota_dtim(1);
#endif
#if HTTP_WR_TO_FLASH
	http_flash_init();
#endif
	client = bk_http_client_init(&config);

	return client;

}

bk_err_t bk_https_client_flash_deinit(bk_http_client_handle_t client)
{
	int err;

	ota_flag = 0;
#if CONFIG_SYSTEM_CTRL
	bk_wifi_ota_dtim(0);
#endif
#if HTTP_WR_TO_FLASH
	http_flash_deinit();
#endif
	if(!client)
		err = bk_http_client_cleanup(client);
	else
		return BK_FAIL;

	return err;

}

bk_err_t https_ota_event_cb(bk_http_client_event_t *evt)
{
    if(!evt)
    {
        return BK_FAIL;
    }

    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
	BK_LOGE(TAG, "HTTPS_EVENT_ERROR\r\n");
	break;
    case HTTP_EVENT_ON_CONNECTED:
	BK_LOGE(TAG, "HTTPS_EVENT_ON_CONNECTED\r\n");
#ifdef CONFIG_HTTP_OTA_WITH_BLE
#if CONFIG_BLUETOOTH
        bk_ble_register_sleep_state_callback(ble_sleep_cb);
#endif
#endif
	break;
    case HTTP_EVENT_HEADER_SENT:
	BK_LOGE(TAG, "HTTPS_EVENT_HEADER_SENT\r\n");
	break;
    case HTTP_EVENT_ON_HEADER:
	BK_LOGE(TAG, "HTTPS_EVENT_ON_HEADER\r\n");
	break;
    case HTTP_EVENT_ON_DATA:
	//do something: evt->data, evt->data_len
#if HTTP_WR_TO_FLASH
	http_wr_to_flash((char *)evt->data,evt->data_len);
#endif
	BK_LOGD(TAG, "HTTP_EVENT_ON_DATA, length:%d\r\n", evt->data_len);
	break;
    case HTTP_EVENT_ON_FINISH:
#if HTTP_WR_TO_FLASH
	http_flash_wr(bk_http_ptr->wr_buf, bk_http_ptr->wr_last_len);
#endif
	bk_https_client_flash_deinit(evt->client);
	BK_LOGI(TAG, "HTTPS_EVENT_ON_FINISH\r\n");
	break;
    case HTTP_EVENT_DISCONNECTED:
	BK_LOGE(TAG, "HTTPS_EVENT_DISCONNECTED\r\n");
	break;

    }
    return BK_OK;
}


int bk_https_ota_download(const char *url)
{
	int err;

      if(!url)
      {
          err = BK_FAIL;
          BK_LOGI(TAG, "url is NULL\r\n");

          return err;
      }
	bk_http_input_t config = {
	    .url = url,
	    .cert_pem = ca_crt_rsa,//ca_crt_rsa_newway07, //ca_crt_rsa,
	    .event_handler = https_ota_event_cb,
	    .buffer_size = HTTPS_INPUT_SIZE,
	    .timeout_ms = 15000
	};

#ifdef CONFIG_HTTP_AB_PARTITION
	ota_temp_exec_flag temp_exec_flag = 6;
	exec_flag exec_temp_part = 6;
	uint8 current_partition;
	current_partition = bk_ota_get_current_partition();
	BK_LOGI(TAG, "current_partition :0x%x",current_partition);
	if(current_partition == EXEX_A_PART ||current_partition == 0xFF)
		update_part_flag = UPDATE_B_PART;
	else if(current_partition == EXEC_B_PART)
		update_part_flag = UPDATE_A_PART;
	else
		return -1;
#endif

	bk_http_client_handle_t client = bk_https_client_flash_init(config);
	if (client == NULL) {
		BK_LOGI(TAG, "client is NULL\r\n");
		err = BK_FAIL;
		return err;
	}
	err = bk_http_client_perform(client);
	if(err == BK_OK){
		BK_LOGI(TAG, "bk_http_client_perform ok\r\n");

#ifdef CONFIG_HTTP_AB_PARTITION
        #ifndef CONFIG_OTA_UPDATE_DEFAULT_PARTITION
            temp_exec_flag = ota_temp_execute_partition(ret); //temp_exec_flag :3 :A ,4:B
        #else
            #ifdef CONFIG_OTA_UPDATE_B_PARTITION
                temp_exec_flag = CONFIRM_EXEC_B; //update B Partition;
            #else
                temp_exec_flag = CONFIRM_EXEC_A; //update A Partition;
            #endif
        #endif

        BK_LOGI(TAG, "from cus temp_exec_flag:0x%x \r\n",temp_exec_flag);

        if(temp_exec_flag == CONFIRM_EXEC_A){
	        BK_LOGI(TAG, "B>>>A \r\n");
	        exec_temp_part = EXEX_A_PART;
        }
        else if(temp_exec_flag == CONFIRM_EXEC_B){
		BK_LOGI(TAG, "A>>B \r\n");
		exec_temp_part = EXEC_B_PART;
        }

        BK_LOGI(TAG, "temp_exec_flag:0x%x \r\n",exec_temp_part);
        ota_write_flash(BK_PARTITION_OTA_FINA_EXECUTIVE, exec_temp_part, 4); //
	 bk_reboot();
#else
        //bk_reboot();
#endif
	}
	else{
		bk_https_client_flash_deinit(client);
		BK_LOGI(TAG, "bk_http_client_perform fail, err:%x\r\n", err);
	}

	return err;
}
