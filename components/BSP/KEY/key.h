#ifndef __KEY_H_
#define __KEY_H_

#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_err.h"

typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_UP,
    KEY_EVENT_DOWN,
    KEY_EVENT_PRESS_TWICE,
    KEY_EVENT_PRESS_LONG,
    KEY_EVENT_OK
} key_event_t;

#define UP_INT_GPIO_PIN  GPIO_NUM_6
#define DOWN_INT_GPIO_PIN  GPIO_NUM_5
#define RIGHT_INT_GPIO_PIN  GPIO_NUM_18
#define LEFT_INT_GPIO_PIN  GPIO_NUM_4
#define PRESS_INT_GPIO_PIN  GPIO_NUM_7


#define KEY_GPIO_MASK (                         \
    (1ULL << UP_INT_GPIO_PIN)    |              \
    (1ULL << DOWN_INT_GPIO_PIN)  |              \
    (1ULL << RIGHT_INT_GPIO_PIN) |              \
    (1ULL << LEFT_INT_GPIO_PIN)  |              \
    (1ULL << PRESS_INT_GPIO_PIN))

#define KEY_DEBOUNCE_TIME_US  (30 * 1000)

esp_err_t key_init(void);
key_event_t key_scan(void);

#endif