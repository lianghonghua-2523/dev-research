/*
 * @Author: lianghonghua-2523 93458223+lianghonghua-2523@users.noreply.github.com 📱17727819640
 * @Date: 2023-11-22 22:43:20
 * @LastEditTime: 2023-11-23 02:41:33
 * @FilePath: \demo1\software\wifi\Wifi.c
 * @Description: The code by lhh
 * @Function:
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "Wifi.h"
#include "Public.h"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10
static const char *TAG = "wifi station";
static int s_retry_num = 0;

static EventGroupHandle_t wifi_event_group;

static int g_wifi_connect_status = WIFI_DISCONNECTED;        /*配网状态标志位*/
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


int GetWiFiStatus(void)
{
    return g_wifi_connect_status;
}

void SetWiFiStatus(int status)
{
    g_wifi_connect_status = status;
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            //s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP...");
        }
        else
        {

            ESP_LOGI(TAG,"connect to the AP fail");
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));

        sprintf(ipaddr,"%d.%d.%d.%d",IP2STR(&event->ip_info.ip));
        printf("ip地址string:%s\r\n",ipaddr);
        s_retry_num = 0;

        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
           }
    else{
        printf("%s,%d\r\n",event_base,event_id);
    }
}

esp_err_t WifiInit(void){

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT,
                                                ESP_EVENT_ANY_ID,
                                                &event_handler,
                                                NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT,
                                                IP_EVENT_STA_GOT_IP,
                                                &event_handler,
                                                NULL) );
    return  ESP_OK;
}

esp_err_t WifiConnect(const char *ssid,const char *password)
{
    wifi_event_group = xEventGroupCreate();
    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config_t));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_PSK;

    memcpy(wifi_config.sta.ssid,ssid,sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password,password,sizeof(wifi_config.sta.password));

    printf("connect ssid:%s\r\n",wifi_config.sta.ssid);
    printf("connect password:%s\r\n",wifi_config.sta.password);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    // esp_event_handler_instance_t instance_any_id;
    // esp_event_handler_instance_t instance_got_ip;

    //等待freertos任务通知
    EventBits_t bits = xEventGroupWaitBits( wifi_event_group,
                                            WIFI_FAIL_BIT|WIFI_CONNECTED_BIT,
                                            pdFALSE,
                                            pdFALSE,
                                            portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        SetWiFiStatus(WIFI_CONNECTED);
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ssid, password);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        SetWiFiStatus(WIFI_DISCONNECTED);
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ssid, password);
    } else {
        SetWiFiStatus(WIFI_DISCONNECTED);
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    return -1;
}


esp_err_t WifiDisconnect(void)
{
    return esp_wifi_stop();

}
