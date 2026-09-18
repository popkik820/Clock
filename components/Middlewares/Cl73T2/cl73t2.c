#include "cl73t2.h"
#include "uart.h"
#include "key_control.h"

esp_err_t HAL_cl73t2_init(uint32_t baudrate)
{
    uart_init(baudrate);
    return ESP_OK;
}

void HAL_cl73t2_music(uint8_t num)
{
    uart_write_serail_data(UART_NUM,num);
}

void HAL_cl73t2_change_mode(void)
{
    uint8_t data = uart_read_serail_data(UART_NUM);
    if(s_context.state != CLOCK_UI_EDIT){
    switch (data)
    {
        case 0x01:
            s_context.mode = CLOCK_MODE_TIME;
            break;
        case 0x02:
            s_context.mode = CLOCK_MODE_CALENDAR;
            break;            
        case 0x03:
            s_context.mode = CLOCK_MODE_TIMER;
            break;        
        case 0x04:
            s_context.mode = CLOCK_MODE_BRIGHTNESS;
            break;        
        case 0x05:
            s_context.mode = CLOCK_MODE_ALARM;
            break;        
        default:
            break;
    }
}
}