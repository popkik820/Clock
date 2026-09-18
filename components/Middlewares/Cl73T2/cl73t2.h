#ifndef __CL73T2_H
#define __CL73T2_H

#include <stdint.h>
#include "esp_err.h"
esp_err_t HAL_cl73t2_init(uint32_t baudrate);
void HAL_cl73t2_music(uint8_t num);
void HAL_cl73t2_change_mode(void);

#endif