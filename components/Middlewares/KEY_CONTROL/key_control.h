#ifndef __KEY_CONTROL_H
#define __KEY_CONTROL_H

#include "esp_err.h"

#include "key.h"
#include "ds3231.h"
#include "alarm.h"
#include "memory.h"
#include "display.h"

typedef enum {
    CLOCK_MODE_TIME = 0,
    CLOCK_MODE_ALARM,
    CLOCK_MODE_CALENDAR,
    CLOCK_MODE_TIMER,
    CLOCK_MODE_BRIGHTNESS,
    CLOCK_MODE_COUNT
} clock_mode_t;

typedef enum {
    CLOCK_UI_VIEW = 0,
    CLOCK_UI_EDIT
} clock_ui_state_t;

typedef enum {
    EDIT_TIME = 0,
    EDIT_ALARM ,
    EDIT_CALENDAR ,
    EDIT_TIMER ,
    EDIT_BRIGHTNESS ,
    EDIT_NULL,
}  clock_edit_mode_t;

typedef enum {
    TIME_FIELD_HOUR_HIGH = 0,
    TIME_FIELD_HOUR_LOW,
    TIME_FIELD_MINUTE_HIGH,
    TIME_FIELD_MINUTE_LOW,
    TIME_FIELD_SECOND_HIGH,
    TIME_FIELD_SECOND_LOW,  
    TIME_FIELD_COUNT
} time_field_t;

typedef enum {
    CLOCK_UI_ACTION_NONE = 0,
    CLOCK_UI_ACTION_MODE_CHANGED,
    CLOCK_UI_ACTION_EDIT_STARTED,
    CLOCK_UI_ACTION_FIELD_CHANGED,
    CLOCK_UI_ACTION_VALUE_CHANGED,
    CLOCK_UI_ACTION_TIME_SAVED,
    CLOCK_UI_ACTION_ALARM_SAVED,
    CLOCK_UI_ACTION_CALENDAR_SAVED,
    CLOCK_UI_ACTION_BRIGHTNESS_SAVED,
    CLOCK_UI_ACTION_ERROR
} clock_ui_action_t;

typedef struct {
    clock_mode_t mode;
    clock_ui_state_t state;
    clock_edit_mode_t edit_mode;
    time_field_t selected_field;
    int edit_tmp[6];
    esp_err_t last_error;
} clock_ui_context_t;

extern clock_ui_context_t s_context;

esp_err_t HAL_key_control_init(void);

clock_ui_action_t HAL_key_control_handle(key_event_t event);

const clock_ui_context_t *HAL_key_control_get_context(void);

#endif