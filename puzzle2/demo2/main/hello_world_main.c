/*
 * @Author: lianghonghua-2523 93458223+lianghonghua-2523@users.noreply.github.com 📱17727819640
 * @Date: 2023-11-22 22:10:13
 * @LastEditTime: 2023-11-24 03:32:06
 * @FilePath: \demo2\main\hello_world_main.c
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

//flash
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
// #include "ffconf.h"
// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
// Mount path for the partition
const char *base_path = "/extflash";


static const char *TAG = "HTTPS_REQUEST";

static const char * wifi_ssid = "UPGRADE_AP";               //LHH_cy
static const char * wifi_password = "TEST1234";             //lhh123456789

static void https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST);
static void ProcessFunc(char *buf,int len);

static esp_flash_t* example_init_ext_flash(void);
static const esp_partition_t* example_add_partition(esp_flash_t* ext_flash, const char* partition_label);
static void example_list_data_partitions(void);
static bool example_mount_fatfs(const char* partition_label);
static void example_get_fatfs_usage(size_t* out_total_bytes, size_t* out_free_bytes);



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
    rev_len = len - i + 1;
    char * rev_buf = (char *)malloc(rev_len*sizeof(char));
    memcpy(rev_buf,buf+i,rev_len);
    //rev_buf[rev_len]='\0';

    //写入flash
    // Set up SPI bus and initialize the external SPI Flash chip
    esp_flash_t* flash = example_init_ext_flash();
    if (flash == NULL) {
        printf("error init ext flash\r\n");
        return;
    }

    // Add the entire external flash chip as a partition
    const char *partition_label = "storage";
    example_add_partition(flash, partition_label);

    // List the available partitions
    example_list_data_partitions();

    // Initialize FAT FS in the partition
    if (!example_mount_fatfs(partition_label)) {
        return;
    }

    // Print FAT FS size information
    size_t bytes_total, bytes_free;
    example_get_fatfs_usage(&bytes_total, &bytes_free);
    ESP_LOGI(TAG, "FAT FS: %d kB total, %d kB free", bytes_total / 1024, bytes_free / 1024);

    // Create a file in FAT FS
    ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/extflash/hello_this_is_long_name_products.json", "wb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "%s\r\n",rev_buf);
    fclose(f);

    // 清空buffer
    memset(rev_buf,0,rev_len);
    //
    ESP_LOGI(TAG, "File written");

    // Open file for reading
    ESP_LOGI(TAG, "Reading file");
    f = fopen("/extflash/hello_this_is_long_name_products.json", "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    fgets(rev_buf, rev_len, f);
    fclose(f);

    ESP_LOGI(TAG, "Read from file: \n'%s'", rev_buf);


    //释放资源
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

static esp_flash_t* example_init_ext_flash(void)
{
    // const spi_bus_config_t bus_config = {
    //     .mosi_io_num = GPIO_NUM_17,
    //     .miso_io_num = GPIO_NUM_16,
    //     .sclk_io_num = GPIO_NUM_15,
    //     .quadwp_io_num = -1,
    //     .quadhd_io_num = -1,
    // };

    const esp_flash_spi_device_config_t device_config = {
        .host_id = SPI1_HOST,
        .cs_id = 0,
        .cs_io_num = GPIO_NUM_14,
        .io_mode = SPI_FLASH_DIO,
        .speed = ESP_FLASH_40MHZ,

    };

    // ESP_LOGI(TAG, "Initializing external SPI Flash");
    // ESP_LOGI(TAG, "Pin assignments:");
    // ESP_LOGI(TAG, "MOSI: %2d   MISO: %2d   SCLK: %2d   CS: %2d",
    //     bus_config.mosi_io_num, bus_config.miso_io_num,
    //     bus_config.sclk_io_num, device_config.cs_io_num
    // );

    // Initialize the SPI bus
    //ESP_ERROR_CHECK(spi_bus_initialize(SPI1_HOST, &bus_config, 1));

    // Add device to the SPI bus
    esp_flash_t* ext_flash;
    ESP_ERROR_CHECK(spi_bus_add_flash_device(&ext_flash, &device_config));

    // Probe the Flash chip and initialize it
    esp_err_t err = esp_flash_init(ext_flash);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize external Flash: %s (0x%x)", esp_err_to_name(err), err);
        return NULL;
    }

    // Print out the ID and size
    uint32_t id;
    ESP_ERROR_CHECK(esp_flash_read_id(ext_flash, &id));
    ESP_LOGI(TAG, "Initialized external Flash, size=%d KB, ID=0x%x", ext_flash->size / 1024, id);

    return ext_flash;
}


static const esp_partition_t* example_add_partition(esp_flash_t* ext_flash, const char* partition_label)
{
    ESP_LOGI(TAG, "Adding external Flash as a partition, label=\"%s\", size=%d KB", partition_label, ext_flash->size / 1024);
    const esp_partition_t* fat_partition;
    ESP_ERROR_CHECK(esp_partition_register_external(ext_flash, 0, ext_flash->size, partition_label, ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, &fat_partition));
    return fat_partition;
}

static void example_list_data_partitions(void)
{
    ESP_LOGI(TAG, "Listing data partitions:");
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL; it = esp_partition_next(it)) {
        const esp_partition_t *part = esp_partition_get(it);
        ESP_LOGI(TAG, "- partition '%s', subtype %d, offset 0x%x, size %d kB",
        part->label, part->subtype, part->address, part->size / 1024);
    }

    esp_partition_iterator_release(it);
}

static bool example_mount_fatfs(const char* partition_label)
{
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, partition_label, &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return false;
    }
    return true;
}


static void example_get_fatfs_usage(size_t* out_total_bytes, size_t* out_free_bytes)
{
    FATFS *fs;
    size_t free_clusters;
    int res = f_getfree("0:", &free_clusters, &fs);
    assert(res == FR_OK);
    size_t total_sectors = (fs->n_fatent - 2) * fs->csize;
    size_t free_sectors = free_clusters * fs->csize;

    // assuming the total size is < 4GiB, should be true for SPI Flash
    if (out_total_bytes != NULL) {
        *out_total_bytes = total_sectors * fs->ssize;
    }
    if (out_free_bytes != NULL) {
        *out_free_bytes = free_sectors * fs->ssize;
    }
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
