#ifndef __ALARM_H
#define __ALARM_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#define music_id 0xAA

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool enabled;
} alarm_t;


esp_err_t HAL_alarm_init(void);
esp_err_t HAL_alarm_set(uint8_t hour,uint8_t minute,uint8_t second);
esp_err_t HAL_alarm_check(void);
void HAL_alarm_get(uint8_t *data);


#endif