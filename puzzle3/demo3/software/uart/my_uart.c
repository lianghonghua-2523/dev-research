#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "my_uart.h"
#define UART1_BAUD_RATE   115200

int g_UART1_BUF_SIZE=1024;


void uart1_init(int baud_rate)
{
        /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;
    ESP_ERROR_CHECK(uart_driver_install(MY_UART_NUM, g_UART1_BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(MY_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(MY_UART_NUM, UART1_TX, UART1_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

}

int uart1_send(const char *data,int len)
{
    return uart_write_bytes(MY_UART_NUM, (const char *) data, len);
}
