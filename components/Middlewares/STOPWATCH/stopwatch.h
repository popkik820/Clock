#ifndef __STOPWATCH_H
#define __STOPWATCH_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

esp_err_t HAL_stopwatch_init(void);
void HAL_stopwatch_start(void);
void HAL_stopwatch_pause(void);
void HAL_stopwatch_reset(void);
uint64_t HAL_stopwatch_get_time(void);
bool HAL_stopwatch_mode(void);

#endif