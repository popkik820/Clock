# include "uart.h"

#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "bsp_uart";
static bool s_uart_initialized = false;

static size_t buffered_len = 0;

void uart_init(uint32_t baudrate)
{
    uart_config_t uart_config = {0};

    uart_config.baud_rate = baudrate;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 122;
    uart_config.source_clk = UART_SCLK_APB;
    uart_param_config(UART_NUM, &uart_config);

    uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
}

uint8_t uart_read_serail_data(uart_port_t uart_num)
{
    uart_get_buffered_data_len(uart_num,(size_t*) &buffered_len);
    if(buffered_len > 0)
    {
        uint8_t data[UART_BUF_SIZE] = {0};
        memset(data, 0, sizeof(data));
        uart_read_bytes(UART_NUM, data, buffered_len, 100);
        if(data[0] == 0xAA && data[1] == 0x55 && data[3] == 0x55 && data[4] == 0xAA)
            return data[2];    
        else
            return -1;
    }
    return 0xFF;
}

void uart_write_serail_data(uart_port_t uart_num,uint8_t num)
{
    uint8_t data[5] = {0xAA,0x55,num,0x55,0xAA};
    uart_write_bytes(uart_num,data,5);
}