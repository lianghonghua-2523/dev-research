#ifndef __MY_UART_H
#define __MY_UART_H



#define MY_UART_NUM UART_NUM_1
#define UART1_TX GPIO_NUM_0
#define UART1_RX GPIO_NUM_1

extern int g_UART1_BUF_SIZE;
void uart1_init(int baud_rate);
int uart1_send(const char *data,int len);






#endif

