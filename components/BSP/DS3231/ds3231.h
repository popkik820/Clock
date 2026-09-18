#ifndef __DS3231_H
#define __DS3231_H

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "iic.h"

#define DS3231_ADDR                 0x68
#define DS3231_STATUS_REG           0x0F                            
#define DS3231_SECONDS_REG          0x00
#define DS3231_MINUTES_REG          0x01
#define DS3231_HOURS_REG            0x02
#define DS3231_DATE_REG             0x04
#define DS3231_MONTH_REG            0x05
#define DS3231_YEAR_REG             0x06

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} ds3231_time_t;

esp_err_t ds3231_init(i2c_obj_t self);
esp_err_t ds3231_get_time(ds3231_time_t *time);
esp_err_t ds3231_set_time(ds3231_time_t *time);
esp_err_t ds3231_change_time_mode(void);

#endif
