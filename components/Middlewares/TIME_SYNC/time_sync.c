#include "time_sync.h"

#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"

#include "ds3231.h"
#include "memory.h"

#define NTP_SERVER                  "ntp.aliyun.com"

#define NTP_SYNC_INTERVAL_MS        (6UL * 60UL * 60UL * 1000UL)

#define SNTP_SYNCED_BIT         BIT0 //获取服务器时间
#define DS3231_UPDATED_BIT       BIT1 //写入DS3231时间

static const char *TAG = "time_sync";

static EventGroupHandle_t s_time_event_group = NULL;
static TaskHandle_t s_time_sync_task_handle = NULL;
static bool s_time_sync_started = false;

static esp_err_t time_sync_write_time(void)
{
    time_t now = time(NULL); //获取ESP32现在时间

    int64_t adjusted_seconds = (int64_t)now +(int64_t)MEMORY_TIME_OFFSET +(int64_t)MEMORY_DATE_OFFSET * 86400LL;
    time_t adjusted_time = (time_t)adjusted_seconds;

    if ((int64_t)adjusted_time != adjusted_seconds) {
        ESP_LOGE(TAG, "Adjusted time exceeds time_t range");
        return ESP_ERR_INVALID_STATE;
    }

    struct tm local_time = {0};

    if (localtime_r(&now, &local_time) == NULL) {
        ESP_LOGE(TAG, "localtime_r failed");
        return ESP_FAIL;
    }

    localtime_r(&adjusted_time,&local_time);

    int full_year = local_time.tm_year + 1900;

    if (full_year < 2000 || full_year > 2099) {
        ESP_LOGE(
            TAG,
            "Invalid system year: %d",
            full_year
        );

        return ESP_ERR_INVALID_STATE;
    }

    ds3231_time_t rtc_time= {
        .seconds = (uint8_t)local_time.tm_sec,
        .minutes = (uint8_t)local_time.tm_min,
        .hours   = (uint8_t)local_time.tm_hour, 
        .weekday = (uint8_t)(local_time.tm_wday + 1),
        .date = (uint8_t)local_time.tm_mday,
        .month = (uint8_t)(local_time.tm_mon + 1),
        .year = (uint8_t)(full_year - 2000),
    };
    
    esp_err_t ret = ds3231_set_time(&rtc_time);

    if (ret != ESP_OK) {
        ESP_LOGE(
            TAG,
            "Write DS3231 failed: %s",
            esp_err_to_name(ret)
        );

        return ret;}

        ESP_LOGI(
        TAG,
        "DS3231 updated: %04d-%02d-%02d %02d:%02d:%02d",
        full_year,
        local_time.tm_mon + 1,
        local_time.tm_mday,
        local_time.tm_hour,
        local_time.tm_min,
        local_time.tm_sec);

        return ESP_OK;
    }

static void sntp_sync_notification_cb(struct timeval *tv)
{
    (void)tv;

    if (s_time_event_group != NULL) {
        xEventGroupSetBits(
            s_time_event_group,
            SNTP_SYNCED_BIT
        );
    }
}

static void time_sync_task(void *arg)
{
    (void)arg;

    while (1) {
        //等待SNTP同步成功
        xEventGroupWaitBits(
            s_time_event_group,
            SNTP_SYNCED_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        for (int retry = 0; retry < 3; ++retry) {

            esp_err_t ret = time_sync_write_time();
            
            if (ret == ESP_OK) {
                xEventGroupSetBits(
                    s_time_event_group,
                    DS3231_UPDATED_BIT
                );

                break;
            }

            ESP_LOGW(
                TAG,
                "Retry DS3231 update: %d/3",
                retry + 1
            );

            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

esp_err_t HAL_time_sync_start(void)
{
    if(s_time_sync_started){
        ESP_LOGW(TAG, "SNTP has already been started");
        return ESP_OK;
    }

    setenv("TZ", "CST-8", 1); //设置时区
    tzset();

    s_time_event_group = xEventGroupCreate();

    if (s_time_event_group == NULL) {
        ESP_LOGE(
            TAG,
            "Create time Event Group failed"
        );

        return ESP_ERR_NO_MEM;
    }

    //创建任务
    BaseType_t task_ret = xTaskCreate(
        time_sync_task,
        "time_sync_task",
        4096,
        NULL,
        5,
        &s_time_sync_task_handle
    );

    if (task_ret != pdPASS) {
        vEventGroupDelete(s_time_event_group);
        s_time_event_group = NULL;

        return ESP_ERR_NO_MEM;
    }

    //设置同步周期
    esp_sntp_set_sync_interval(NTP_SYNC_INTERVAL_MS);

    //创建SNTP配置
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTP_SERVER);
    config.sync_cb = sntp_sync_notification_cb; //回调函数
    config.smooth_sync = false; //收到同步时间后立即同步

    esp_err_t ret = esp_netif_sntp_init(&config);

    if (ret != ESP_OK) {
        ESP_LOGE(
            TAG,
            "SNTP initialization failed: %s",
            esp_err_to_name(ret)
        );

        vTaskDelete(s_time_sync_task_handle);
        s_time_sync_task_handle = NULL;

        vEventGroupDelete(s_time_event_group);
        s_time_event_group = NULL;

        return ret;
    }

    s_time_sync_started = true;

    ESP_LOGI(
        TAG,
        "SNTP started, server=%s, interval=6 hours",
        NTP_SERVER
    );

    return ESP_OK;
}


esp_err_t HAL_time_sync_first_update(uint32_t timeout_ms)
{
    if (!s_time_sync_started || s_time_event_group == NULL) {

        return ESP_ERR_INVALID_STATE;
    }

    EventBits_t bits = xEventGroupWaitBits(
        s_time_event_group,
        DS3231_UPDATED_BIT,

        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms)
    );

    if ((bits & DS3231_UPDATED_BIT) != 0) {
        return ESP_OK;
    }

    return ESP_ERR_TIMEOUT;
}