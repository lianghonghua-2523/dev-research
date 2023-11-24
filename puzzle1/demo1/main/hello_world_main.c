/*
 * @Author: lianghonghua-2523 93458223+lianghonghua-2523@users.noreply.github.com 📱17727819640
 * @Date: 2023-11-22 22:10:13
 * @LastEditTime: 2023-11-24 07:38:10
 * @FilePath: \demo1\main\hello_world_main.c
 * @Description: The code by lhh
 * @Function:
 */
/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "Public.h"
#include "Wifi.h"
#include "cJSON.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "esp_crt_bundle.h"
static const char *TAG = "HTTPS_REQUEST";

static const char * wifi_ssid = "UPGRADE_AP";               //LHH_cy
static const char * wifi_password = "TEST1234";             //lhh123456789

static void https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST);
static void ProcessFunc(char *buf,int len);

#define http_url "https://dummyjson.com/products/1"

static const char HOWSMYSSL_REQUEST[] = "GET https://dummyjson.com/products/1 HTTP/1.1\r\n"
                             "Host: dummyjson.com\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048


static void https_get_request_using_crt_bundle(void)
{
    ESP_LOGI(TAG, "https_request using crt bundle");
    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    https_get_request(cfg, http_url, HOWSMYSSL_REQUEST);
}

static void https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST)
{
    char buf[4096];
    int ret, len;

    struct esp_tls *tls = esp_tls_conn_http_new(WEB_SERVER_URL, &cfg);

    if (tls != NULL) {
        ESP_LOGI(TAG, "Connection established...");
    } else {
        ESP_LOGE(TAG, "Connection failed...");
        goto exit;
    }

    size_t written_bytes = 0;
    do {
        ret = esp_tls_conn_write(tls,
                                 REQUEST + written_bytes,
                                 strlen(REQUEST) - written_bytes);
        if (ret >= 0) {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ  && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned: [0x%02X](%s)", ret, esp_err_to_name(ret));
            goto exit;
        }
    } while (written_bytes < strlen(REQUEST));

    ESP_LOGI(TAG, "Reading HTTPS response...");

    do {
        len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);
        if (ret == ESP_TLS_ERR_SSL_WANT_WRITE  || ret == ESP_TLS_ERR_SSL_WANT_READ) {
            continue;
        }

        if (ret < 0) {
            ESP_LOGI(TAG, "esp_tls_conn_read  returned [-0x%02X](%s)", -ret, esp_err_to_name(ret));
            break;
        }

        if (ret == 0) {
            ESP_LOGI(TAG, "connection closed");
            break;
        }

        len = ret;
        ESP_LOGI(TAG, "%d bytes read", len);
        ProcessFunc(buf,len);
        break; //

        /* Print response directly to stdout as it is read */

    } while (1);

exit:
    esp_tls_conn_delete(tls);
    for (int countdown = 1; countdown >= 0; countdown--) {
        // ESP_LOGI(TAG, "%d...", countdown);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

//处理函数
static void ProcessFunc(char *buf,int len)
{
    int i,rev_len;
    for ( i = 0; i < len; i++)
    {
        /* code */
        if(buf[i]=='{')
            break;

    }
    rev_len = len - i +1;
    char * rev_buf = (char *)malloc(rev_len*sizeof(char));
    memcpy(rev_buf,buf+i,rev_len);
    printf("rev:%s\r\n",rev_buf);
    //json解析
    cJSON* cjson_body = NULL;
    cJSON* cjson_brand = NULL;
    cjson_body = cJSON_Parse(rev_buf);
    if(cjson_body == NULL)
    {
        printf("json parse fail.\n");
        return ;
    }
    cjson_brand = cJSON_GetObjectItem(cjson_body, "brand");
    if(cjson_brand == NULL)
    {
        printf("json parse fail.\n");
        return ;
    }

    ESP_LOGI("brand","%s",cjson_brand->valuestring);
    // printf("brand: %s\n", cjson_brand->valuestring);


    //释放资源
    cJSON_Delete(cjson_body);
    free(rev_buf);
    rev_buf = NULL;
}

static void https_test_task(void *pvParameters)
{
    https_get_request_using_crt_bundle();

    // ESP_LOGI(TAG, "Finish https_request example");
    //关闭wifi
    WifiDisconnect();
    vTaskDelete(NULL);

}

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(WifiInit());
    if(GetWiFiStatus()!=WIFI_CONNECTED)
        WifiConnect(wifi_ssid,wifi_password); //连接wifi

    //连接WiFi成功创建https请求任务
    xTaskCreate(&https_test_task, "https_test_task", 8192, NULL, 5, NULL);



}
