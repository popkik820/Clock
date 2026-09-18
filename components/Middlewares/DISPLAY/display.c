#include "display.h"
#include "ds3231.h"
#include "max7219.h"
#include "spi.h"
#include "esp_log.h"
#include "memory.h"
#include "stopwatch.h"
#include "alarm.h"
#include <stdint.h>
#include <time.h>
#include "key_control.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "display";

ds3231_time_t now;

static void time_preproduct(int *data)
{
    ds3231_get_time(&now);
    data[0] = 11;
    data[1] = now.hours / 10;
    data[2] = now.hours %10;
    data[3] = 10;
    data[4] = now.minutes / 10;
    data[5] = now.minutes %10;
    data[6] = 10;
    data[7] = now.seconds / 10;
    data[8] = now.seconds % 10;
    if(s_context.state == CLOCK_UI_EDIT)
    {
        switch(s_context.selected_field){
            case TIME_FIELD_HOUR_HIGH:
                data[1] = s_context.edit_tmp[0];
                break;
            case TIME_FIELD_HOUR_LOW:
                data[2] = s_context.edit_tmp[1];
                break;
            case TIME_FIELD_MINUTE_HIGH:
                data[4] = s_context.edit_tmp[2];
                break;            
            case TIME_FIELD_MINUTE_LOW:
                data[5] = s_context.edit_tmp[3];
                break;
            case TIME_FIELD_SECOND_HIGH:
                data[7] = s_context.edit_tmp[4];
                break;
            case TIME_FIELD_SECOND_LOW:
                data[8] = s_context.edit_tmp[5];
                break;
            default:
                break;
        }
    }
}

static void date_preproduct(int *data)
{
    ds3231_get_time(&now);
    
    data[0] = 13;
    data[1] = 2;
    data[2] = 0;
    data[3] = now.year / 10;
    data[4] = now.year % 10;
    data[5] = now.month / 10;
    data[6] = now.month % 10;
    data[7] = now.date / 10;
    data[8] = now.date % 10;

    if(s_context.state == CLOCK_UI_EDIT){
        switch(s_context.selected_field){
            case TIME_FIELD_HOUR_HIGH:
                data[3] = s_context.edit_tmp[0];
                break;
            case TIME_FIELD_HOUR_LOW:
                data[4] = s_context.edit_tmp[1];
                break;
            case TIME_FIELD_MINUTE_HIGH:
                data[5] = s_context.edit_tmp[2] / 10;
                data[6] = s_context.edit_tmp[2] % 10;
                break;            
            case TIME_FIELD_MINUTE_LOW:
                data[7] = s_context.edit_tmp[3] / 10;
                data[8] = s_context.edit_tmp[3] % 10;
                break;
            default:
                break;
        }
    }
}

static void timer_preproduct(int *data)
{
    int timer_data[3]= {0};
    uint64_t per_timer_data = HAL_stopwatch_get_time();
    timer_data[0] = per_timer_data % 1000;
    per_timer_data -= timer_data[0];
    timer_data[1] = per_timer_data % 60000 / 1000;
    per_timer_data -= timer_data[1] * 1000;
    timer_data[2] = per_timer_data / 60000;
    if(timer_data[2] > 10){
        HAL_stopwatch_reset();
        HAL_stopwatch_start();}
    data[0] = 14;
    data[1] = timer_data[2];
    data[2] = 10;
    data[3] = timer_data[1] / 10;
    data[4] = timer_data[1] % 10;
    data[5] = 10;
    data[6] = timer_data[0] / 100;
    data[7] = (timer_data[0] - data[6] * 100) /10;
    data[8] = (timer_data[0] % 100) % 10;

}

static void alarm_preproduct(int *data)
{
    uint8_t alarm_data[3]= {0};
    HAL_alarm_get(alarm_data);

    data[0] = 12;
    data[1] = alarm_data[0] / 10;
    data[2] = alarm_data[0] %10;
    data[3] = 10;
    data[4] = alarm_data[1] / 10;
    data[5] = alarm_data[1] %10;
    data[6] = 10;
    data[7] = alarm_data[2] / 10;
    data[8] = alarm_data[2] % 10;

    if(s_context.state == CLOCK_UI_EDIT)
    {
        switch(s_context.selected_field){
            case TIME_FIELD_HOUR_HIGH:
                data[1] = s_context.edit_tmp[0];
                break;
            case TIME_FIELD_HOUR_LOW:
                data[2] = s_context.edit_tmp[1];
                break;
            case TIME_FIELD_MINUTE_HIGH:
                data[4] = s_context.edit_tmp[2];
                break;            
            case TIME_FIELD_MINUTE_LOW:
                data[5] = s_context.edit_tmp[3];
                break;
            case TIME_FIELD_SECOND_HIGH:
                data[7] = s_context.edit_tmp[4];
                break;
            case TIME_FIELD_SECOND_LOW:
                data[8] = s_context.edit_tmp[5];
                break;
            default:
                break;
        }
    }
}

static void intensity_preproduct(int *data)
{
    int intensity_data = HAL_memory_read_intensity();

    data[0] = 15;
    data[1] = 16;
    data[2] = 16;
    data[3] = 16;
    data[4] = intensity_data / 10;
    data[5] = intensity_data % 10;
    data[6] = 16;
    data[7] = 16;
    data[8] = 16;

    if(s_context.state == CLOCK_UI_EDIT)
    {
        switch(s_context.selected_field){
            case TIME_FIELD_HOUR_HIGH:
                data[4] = s_context.edit_tmp[0] / 10;
                data[5] = s_context.edit_tmp[0] % 10;
            default:
                break;
        }
    }
}

static esp_err_t display_disassemble(int *data)
{
    switch (s_context.mode)
    {
        case CLOCK_MODE_TIME:
            time_preproduct(data);
            return ESP_OK;
        case CLOCK_MODE_CALENDAR:
            date_preproduct(data);
            return ESP_OK;
        case CLOCK_MODE_TIMER:
            timer_preproduct(data);
            return ESP_OK;
        case CLOCK_MODE_ALARM:
            alarm_preproduct(data);
            return ESP_OK;
        case CLOCK_MODE_BRIGHTNESS:
            intensity_preproduct(data);
            return ESP_OK;
        default:
            ESP_LOGI(TAG, "Input Wrong Mode");
            return ESP_FAIL;
    }
}

static void dispaly_mask(int *data)
{
    if ((esp_timer_get_time() / 500000LL) % 2 == 0) 
        return;

    if(s_context.state != CLOCK_UI_VIEW){
        int position = -1;
    switch(s_context.mode){
        static const uint8_t positions_time[6] = {1, 2, 4, 5, 7, 8};
        static const uint8_t positions_calendar[6] = {3 , 4 , 6 , 8 , -1 , -1};
        case CLOCK_MODE_TIME:
                position = positions_time[s_context.selected_field];
                break;
        case CLOCK_MODE_ALARM:
                position = positions_time[s_context.selected_field];
                break;
        case CLOCK_MODE_CALENDAR:
            position = positions_calendar[s_context.selected_field];
            break;
        case CLOCK_MODE_BRIGHTNESS:
            position = 5;
            break;
        default:
            break;
    }
    
    if(position != 0)
        data[position] = 16;
    else 
        return;
    }
    else
        return;
}
esp_err_t HAL_display_init(void)
{
    spi2_init();
    max7219_init();
    max7219_intensity_change(MEMORY_INTENSITY);
    
    return ESP_OK;
}

void HAL_display_update(void)
{
    int data[9] = {0};
    int try = 0;
    while(1)
    {
        esp_err_t ret = display_disassemble(data);
        if(ret != ESP_OK)
        {
        ESP_LOGI(TAG,"Loading again...");
        try++;
        }
        else
            break;
        if(try == 3)
        {
            ESP_LOGI(TAG,"Loading data failed!");
            return;
        }    
    }
    dispaly_mask(data);
    max7219_write_number(data);
}

esp_err_t HAL_display_set_intensity(int intensenty)
{
    esp_err_t ret = HAL_memory_write_intensity(intensenty);
    if(ret != ESP_OK)
        return ret;
    MEMORY_INTENSITY = HAL_memory_read_intensity();
    max7219_intensity_change(MEMORY_INTENSITY);
    return ESP_OK;
}
