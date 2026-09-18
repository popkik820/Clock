#ifndef __TIME_SYNC_H
#define __TIME_SYNC_H

#include <stdint.h>
#include "esp_err.h"

esp_err_t HAL_time_sync_start(void);
esp_err_t HAL_time_sync_first_update(uint32_t timeout_ms);

#endif