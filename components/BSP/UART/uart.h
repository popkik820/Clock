# ifndef __UART_H__
# define __UART_H__

# include <string.h>
# include "freertos/FreeRTOS.h"
# include "freertos/task.h"
# include "driver/uart.h"
# include "driver/uart_select.h"
# include "driver/gpio.h"

# define UART_NUM UART_NUM_1
# define UART_TX_PIN GPIO_NUM_16
# define UART_RX_PIN GPIO_NUM_17

# define UART_BUF_SIZE 1024

void uart_init(uint32_t baudrate);
uint8_t uart_read_serail_data(uart_port_t uart_num);
void uart_write_serail_data(uart_port_t uart_num,uint8_t num);

# endif