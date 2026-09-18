#include "alarm.h"

#include "cl73t2.h"
#include "ds3231.h"
#include "memory.h"

static alarm_t s_alarm;
static bool s_triggered = false;

static bool alarm_time_valid(uint8_t hour,uint8_t minute,uint8_t second)
{
    return hour <= 23 && minute <= 59 && second <= 59;
}

esp_err_t HAL_alarm_init(void)
{
    uint8_t data[3];

    HAL_memory_read_alarm(data);

    s_triggered = false;

    if (!alarm_time_valid(data[0], data[1], data[2])) {
        s_alarm.hour = 0;
        s_alarm.minute = 0;
        s_alarm.second = 0;

        return ESP_OK;
    }

    s_alarm.hour = data[0];
    s_alarm.minute = data[1];
    s_alarm.second = data[2];

    return ESP_OK;
}

esp_err_t HAL_alarm_set(uint8_t hour,uint8_t minute,uint8_t second)
{
    if (!alarm_time_valid(hour, minute, second)) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[3] = {hour,minute,second};
    esp_err_t ret = HAL_memory_write_alarm(data);

    if (ret != ESP_OK) 
        return ret;

    s_alarm.hour = hour;
    s_alarm.minute = minute;
    s_alarm.second = second;
    s_triggered = false;

    return ESP_OK;
}

esp_err_t HAL_alarm_check(void)
{
    ds3231_time_t now;
    esp_err_t ret = ds3231_get_time(&now);

    if (ret != ESP_OK) 
        return ret;

    bool time_matched =
        now.hours == s_alarm.hour &&
        now.minutes == s_alarm.minute &&
        now.seconds == s_alarm.second;

    if (time_matched && !s_triggered) {
        HAL_cl73t2_music(music_id);
        s_triggered = true;
    }

    if (!time_matched) {
        s_triggered = false;
    }

    return ESP_OK;
}

void HAL_alarm_get(uint8_t *data)
{
    data[0] = s_alarm.hour;
    data[1] = s_alarm.minute;
    data[2] = s_alarm.second;
}
