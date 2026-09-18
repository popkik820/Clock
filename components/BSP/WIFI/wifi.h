#ifndef __WIFI_H
#define __WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_wifi.h"

#define WIFI_SSID            "popkik"
#define WIFI_PASSWORD        "123456789"

#define WIFI_CONNECTED_BIT  BIT0

extern EventGroupHandle_t wifi_event;

#define WIFICONFIG()   {                            \
    .sta = {                                        \
        .ssid = WIFI_SSID,                          \
        .password = WIFI_PASSWORD,                  \
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,   \
    },                                              \
}

void wifi_sta_init(void);

#endif
