#include "key.h"

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_timer.h"

#define KEY_DOUBLE_CLICK_US  (300 * 1000LL)
#define KEY_LONG_PRESS_US    (700 * 1000LL)

/* 原有消抖状态 */
static key_event_t s_candidate_key = KEY_EVENT_NONE;
static key_event_t s_stable_key = KEY_EVENT_NONE;
static int64_t s_candidate_since_us = 0;

/* 中键手势状态 */
static int64_t s_ok_pressed_us = 0;
static int64_t s_ok_released_us = 0;
static bool s_wait_second_click = false;
static bool s_second_click = false;
static bool s_long_sent = false;

static void key_reset_gesture(void)
{
    s_ok_pressed_us = 0;
    s_ok_released_us = 0;
    s_wait_second_click = false;
    s_second_click = false;
    s_long_sent = false;
}

static key_event_t key_read_raw(void)
{
    if (gpio_get_level(UP_INT_GPIO_PIN) == 0) {
        return KEY_EVENT_UP;
    }

    if (gpio_get_level(DOWN_INT_GPIO_PIN) == 0) {
        return KEY_EVENT_DOWN;
    }

    // if (gpio_get_level(LEFT_INT_GPIO_PIN) == 0) {
    //     return KEY_EVENT_LEFT;
    // }

    // if (gpio_get_level(RIGHT_INT_GPIO_PIN) == 0) {
    //     return KEY_EVENT_RIGHT;
    // }

    if (gpio_get_level(PRESS_INT_GPIO_PIN) == 0) {
        return KEY_EVENT_OK;
    }

    return KEY_EVENT_NONE;
}

esp_err_t key_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = KEY_GPIO_MASK,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t ret = gpio_config(&config);

    if (ret != ESP_OK) {
        return ret;
    }

    s_candidate_key = KEY_EVENT_NONE;
    s_stable_key = KEY_EVENT_NONE;
    s_candidate_since_us = esp_timer_get_time();
    key_reset_gesture();

    return ESP_OK;
}

key_event_t key_scan(void)
{
    key_event_t raw_key = key_read_raw();
    int64_t now_us = esp_timer_get_time();

    if (raw_key != s_candidate_key) {
        s_candidate_key = raw_key;
        s_candidate_since_us = now_us;
    }

    if (s_candidate_key != s_stable_key && now_us - s_candidate_since_us >= KEY_DEBOUNCE_TIME_US) {

        key_event_t previous_key = s_stable_key;
        s_stable_key = s_candidate_key;

        if (s_stable_key == KEY_EVENT_OK) {
            /* 中键有效按下：先判断是否为第二次点击 */
            s_second_click = s_wait_second_click && now_us - s_ok_released_us <= KEY_DOUBLE_CLICK_US;

            /* 等待窗口已过，先交付上一次单击；
             * 同时将本次按下作为一轮新手势的起点。
             */
            bool previous_single = s_wait_second_click && !s_second_click;

            s_wait_second_click = false;
            s_ok_pressed_us = now_us;
            s_long_sent = false;

            if (previous_single) {
                return KEY_EVENT_OK;
            }

        } else if (s_stable_key == KEY_EVENT_NONE) {
            if (previous_key == KEY_EVENT_OK) {
                /* 中键有效松开 */
                if (s_long_sent) {
                    /* 长按已经触发，松开不再产生单击 */
                    key_reset_gesture();

                } else if (s_second_click) {
                    /* 两次短按完成，只发送一次左键事件 */
                    key_reset_gesture();
                    return KEY_EVENT_PRESS_TWICE;

                } else {
                    /* 第一次短按结束，等待第二次点击 */
                    s_ok_released_us = now_us;
                    s_wait_second_click = true;
                }
            }

        } else {
            key_reset_gesture();
            return s_stable_key;
        }
    }

    /* 3. 长按：达到门限时发送一次右键事件 */
    if (s_stable_key == KEY_EVENT_OK &&
        raw_key == KEY_EVENT_OK &&
        !s_long_sent &&
        now_us - s_ok_pressed_us >= KEY_LONG_PRESS_US) {

        s_long_sent = true;
        s_wait_second_click = false;
        s_second_click = false;
        return KEY_EVENT_PRESS_LONG;
    }

    /* 4. 第一次短按后，等待窗口结束才确认单击 */
    if (s_wait_second_click &&
        s_stable_key == KEY_EVENT_NONE &&
        now_us - s_ok_released_us >= KEY_DOUBLE_CLICK_US) {

        /* 如果第二次按下已被采样且落在窗口内，
         * 先等待其消抖完成，避免提前发送单击。
         */
        bool second_press_pending =
                s_candidate_key == KEY_EVENT_OK &&
                s_candidate_since_us - s_ok_released_us
                    <= KEY_DOUBLE_CLICK_US;

        if (!second_press_pending) {
            s_wait_second_click = false;
            return KEY_EVENT_OK;
        }
    }

    return KEY_EVENT_NONE;
}