#ifndef __MEMORY_H
#define __MEMORY_H

#include <stdint.h>
#include "esp_err.h"
#include "iic.h"

#define MEMORY_ADDR_INTENSITY        0x00U
#define MEMORY_ADDR_ALARM_HOUR       0x04U
#define MEMORY_ADDR_ALARM_MINUTE     0x05U
#define MEMORY_ADDR_ALARM_SECOND     0x06U
#define MEMORY_ADDR_TIME_OFFSET      0x08U
#define MEMORY_ADDR_DATE_OFFSET      0x0CU

extern int MEMORY_INTENSITY;
extern uint8_t MEMORY_ALARM[3];
extern int32_t MEMORY_TIME_OFFSET;
extern int32_t MEMORY_DATE_OFFSET;

esp_err_t HAL_memory_write_intensity(int intensity);
int HAL_memory_read_intensity(void);
esp_err_t HAL_memory_write_alarm(uint8_t *alarm);
void HAL_memory_read_alarm(uint8_t *alarm);
esp_err_t HAL_memory_write_time_offset(int32_t time_offset);
int32_t HAL_memory_read_time_offset(void);
esp_err_t HAL_memory_write_date_offset(int32_t date_offset);
int32_t HAL_memory_read_date_offset(void);
esp_err_t HAL_memory_init(i2c_obj_t self);

#endif