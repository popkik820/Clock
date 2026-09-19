#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "iic.h"
#include "spi.h"
#include "wifi.h"
#include "uart.h"
#include "24cxx.h"
#include "ds3231.h"
#include "max7219.h"

#include "alarm.h"
#include "cl73t2.h"
#include "key_control.h"
#include "memory.h"
#include "stopwatch.h"
#include "time_sync.h"
#include "display.h"

static const char *TAG = "main";

static void nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
}

void app_main(void)
{
    i2c_obj_t i2c0_master =iic_init(I2C_NUM_0);

    //时钟模块初始化
    ESP_ERROR_CHECK(ds3231_init(i2c0_master));

    //MEMORY功能初始化
    ESP_ERROR_CHECK(HAL_memory_init(i2c0_master));

    //LED展示功能初始化
    ESP_ERROR_CHECK(HAL_display_init());

    //语音交互功能初始化
    ESP_ERROR_CHECK(HAL_cl73t2_init(19200));

    //闹钟功能初始化
    ESP_ERROR_CHECK(HAL_alarm_init());

    //秒表功能初始化
    ESP_ERROR_CHECK(HAL_stopwatch_init());

    //按键状态机初始化
    ESP_ERROR_CHECK(HAL_key_control_init());

    //WIFI初始化
    nvs_init();
    wifi_sta_init();

    //启动 NTP 对时
    ESP_ERROR_CHECK(HAL_time_sync_start());

    esp_err_t ret = HAL_time_sync_first_update(30000);

    if(ret == ESP_OK){
        ESP_LOGI(TAG, "First NTP calibration completed");
    }
    else{
        ESP_LOGW(TAG, "First NTP calibration timed out");
    }

    while(1)
    {
        //扫描按键状态机
        HAL_key_control_handle(key_scan());

        //刷新LED
        HAL_display_update();

        //闹钟检测
        HAL_alarm_check();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
