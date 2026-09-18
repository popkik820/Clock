#include "24cxx.h"
#include "memory.h"
#include <stdbool.h>

#include "esp_log.h"
static const char *TAG = "memory";

int MEMORY_INTENSITY = 5;
uint8_t MEMORY_ALARM[3] = {0, 0, 0};
int32_t MEMORY_TIME_OFFSET = 0;
int32_t MEMORY_DATE_OFFSET = 0;

static void memory_write_int32(uint16_t address,int32_t value)
{
    uint32_t raw_value = (uint32_t)value;

    uint8_t data[4] = {
        (uint8_t)(raw_value & 0xFFU),
        (uint8_t)((raw_value >> 8) & 0xFFU),
        (uint8_t)((raw_value >> 16) & 0xFFU),
        (uint8_t)((raw_value >> 24) & 0xFFU)
    };

    at24cxx_write(address,data,sizeof(data));
}

static int32_t memory_read_int32(uint16_t address)
{
    uint8_t data[4];

    at24cxx_read(
        address,
        data,
        sizeof(data)
    );

    uint32_t raw_value =
        ((uint32_t)data[0]) |
        ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);

    return (int32_t)raw_value;
}

esp_err_t HAL_memory_write_intensity(int intensity)
{
    if(intensity > 15 || intensity < 0)
    {
        ESP_LOGE(TAG,"intensity invalid");
        return ESP_ERR_INVALID_STATE;
    }
    at24cxx_write_one_byte(MEMORY_ADDR_INTENSITY,intensity);
    return ESP_OK;
}

int HAL_memory_read_intensity(void)
{
    int data = at24cxx_read_one_byte(MEMORY_ADDR_INTENSITY);
    return data;
}

esp_err_t HAL_memory_write_alarm(uint8_t *alarm)
{
    if(alarm == NULL ||
        alarm[0] > (uint8_t)23 ||
        alarm[1] > (uint8_t)59 ||
        alarm[2] > (uint8_t)59 )
    {
        ESP_LOGE(TAG,"alarm incalid!");
        return ESP_ERR_INVALID_STATE;
    }
    uint8_t data[3] = {alarm[0],alarm[1],alarm[2]}; 
    at24cxx_write(MEMORY_ADDR_ALARM_HOUR,data,3);
    return ESP_OK;
}

void HAL_memory_read_alarm(uint8_t *alarm)
{
    at24cxx_read(MEMORY_ADDR_ALARM_HOUR,alarm,3);
}

esp_err_t HAL_memory_write_time_offset(int32_t time_offset)
{
    if(time_offset > 86400 || time_offset < -86400)
    {
        ESP_LOGE(TAG,"time_offset invalid");
        return ESP_ERR_INVALID_STATE;
    }
    memory_write_int32(MEMORY_ADDR_TIME_OFFSET,time_offset);
    return ESP_OK;
}

int32_t HAL_memory_read_time_offset(void)
{
    int32_t data = memory_read_int32(MEMORY_ADDR_TIME_OFFSET);
    return data;
}
esp_err_t HAL_memory_write_date_offset(int32_t date_offset)
{
    if(date_offset > 86400 || date_offset < -86400)
    {
        ESP_LOGE(TAG,"date_offset invalid");
        return ESP_ERR_INVALID_STATE;
    }
    memory_write_int32(MEMORY_ADDR_DATE_OFFSET,date_offset);
    return ESP_OK;
}

int32_t HAL_memory_read_date_offset(void)
{
    int32_t data = memory_read_int32(MEMORY_ADDR_DATE_OFFSET);
    return data;
}

esp_err_t HAL_memory_init(i2c_obj_t self)
{
    at24cxx_init(self);
    if(at24cxx_check())
    {
        ESP_LOGE(TAG,"memory init failed");
        return ESP_FAIL;
    }
    MEMORY_INTENSITY = HAL_memory_read_intensity();
    MEMORY_TIME_OFFSET = HAL_memory_read_time_offset();
    MEMORY_DATE_OFFSET = HAL_memory_read_date_offset();
    HAL_memory_read_alarm(MEMORY_ALARM);
    return ESP_OK;
}