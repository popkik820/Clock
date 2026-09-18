#ifndef __DISPALY_
#define __DISPALY_

#include "esp_err.h"

esp_err_t HAL_display_init(void);
void HAL_display_update(void);
esp_err_t HAL_display_set_intensity(int intensenty);

#endif