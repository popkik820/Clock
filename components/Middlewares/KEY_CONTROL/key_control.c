#include "key_control.h"
#include "esp_log.h"
#include "stopwatch.h"

static const char *TAG = "key_control";

clock_ui_context_t s_context = {0};

static uint8_t increase_wrap(uint8_t value, uint8_t maximum)
{
    if (value >= maximum) {
        return 0;
    }

    return value + 1;
}

static uint8_t decrease_wrap(uint8_t value, uint8_t maximum)
{
    if (value == 0) {
        return maximum;
    }

    return value - 1;
}

static void time_field_increase(void)
{
    switch (s_context.selected_field) {
        case TIME_FIELD_HOUR_HIGH:
            s_context.edit_tmp[0] =
                increase_wrap(s_context.edit_tmp[0], 2);
            break;
        case TIME_FIELD_HOUR_LOW:
        if(s_context.edit_tmp[0] == 2){
            s_context.edit_tmp[1] =
                increase_wrap(s_context.edit_tmp[1], 3);}
        else{
            s_context.edit_tmp[1] =
                increase_wrap(s_context.edit_tmp[1], 9);}
            break;
        case TIME_FIELD_MINUTE_HIGH:
            s_context.edit_tmp[2] =
                increase_wrap(s_context.edit_tmp[2], 5);
            break;
        case TIME_FIELD_MINUTE_LOW:
            s_context.edit_tmp[3] =
                increase_wrap(s_context.edit_tmp[3], 9);
            break;
        case TIME_FIELD_SECOND_HIGH:
            s_context.edit_tmp[4] =
                increase_wrap(s_context.edit_tmp[4], 5);
            break;
        case TIME_FIELD_SECOND_LOW:
            s_context.edit_tmp[5] =
                increase_wrap(s_context.edit_tmp[5], 9);
            break;
        default:
            break;
    }
}

static void time_field_decrease(void)
{
    switch (s_context.selected_field) {
        case TIME_FIELD_HOUR_HIGH:
            s_context.edit_tmp[0] =
                decrease_wrap(s_context.edit_tmp[0], 2);
            break;
        case TIME_FIELD_HOUR_LOW:
        if(s_context.edit_tmp[0] == 1){
            s_context.edit_tmp[1] =
                decrease_wrap(s_context.edit_tmp[1], 9);}
        else{
            s_context.edit_tmp[1] =
                decrease_wrap(s_context.edit_tmp[1], 3);}
            break;
        case TIME_FIELD_MINUTE_HIGH:
            s_context.edit_tmp[2] =
                decrease_wrap(s_context.edit_tmp[2], 5);
            break;
        case TIME_FIELD_MINUTE_LOW:
            s_context.edit_tmp[3] =
                decrease_wrap(s_context.edit_tmp[3], 9);
            break;
        case TIME_FIELD_SECOND_HIGH:
            s_context.edit_tmp[4] =
                decrease_wrap(s_context.edit_tmp[4], 5);
            break;
        case TIME_FIELD_SECOND_LOW:
            s_context.edit_tmp[5] =
                decrease_wrap(s_context.edit_tmp[5], 9);
            break;
        default:
            break;
    }
}

static clock_ui_action_t enter_time_edit(void)
{
    ds3231_time_t data;
    esp_err_t ret = ds3231_get_time(&data);
    s_context.edit_tmp[0] = data.hours / 10;
    s_context.edit_tmp[1] = data.hours % 10;
    s_context.edit_tmp[2] = data.minutes / 10;
    s_context.edit_tmp[3] = data.minutes % 10;
    s_context.edit_tmp[4] = data.seconds / 10;
    s_context.edit_tmp[5] = data.seconds % 10;

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    s_context.selected_field = TIME_FIELD_HOUR_HIGH;
    s_context.state = CLOCK_UI_EDIT;
    s_context.edit_mode = EDIT_TIME;
    s_context.last_error = ESP_OK;

    return CLOCK_UI_ACTION_EDIT_STARTED;
}

static clock_ui_action_t save_time_edit(void)
{
    ds3231_time_t current_time;

    esp_err_t ret = ds3231_get_time(&current_time);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    int32_t offset = 3600 * (s_context.edit_tmp[0] * 10 + s_context.edit_tmp[1] - current_time.hours)
                    + 60 * (s_context.edit_tmp[2] * 10 + s_context.edit_tmp[3] - current_time.minutes)
                    + (s_context.edit_tmp[4] * 10 + s_context.edit_tmp[5] - current_time.seconds);

    ret = HAL_memory_write_time_offset(HAL_memory_read_time_offset() + offset);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    current_time.hours = s_context.edit_tmp[0] * 10 + s_context.edit_tmp[1];
    current_time.minutes = s_context.edit_tmp[2] * 10 + s_context.edit_tmp[3];
    current_time.seconds = s_context.edit_tmp[4] * 10 + s_context.edit_tmp[5];

    ret = ds3231_set_time(&current_time);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }
    
    MEMORY_TIME_OFFSET = HAL_memory_read_time_offset();
    s_context.state = CLOCK_UI_VIEW;
    s_context.edit_mode = EDIT_NULL;
    s_context.last_error = ESP_OK;

    return CLOCK_UI_ACTION_TIME_SAVED;
}

static void alarm_field_increase(void)
{
    switch (s_context.selected_field) {
        case TIME_FIELD_HOUR_HIGH:
            s_context.edit_tmp[0] =
                increase_wrap(s_context.edit_tmp[0], 2);
            break;
        case TIME_FIELD_HOUR_LOW:
            s_context.edit_tmp[1] =
                increase_wrap(s_context.edit_tmp[1], 9);
            break;
        case TIME_FIELD_MINUTE_HIGH:
            s_context.edit_tmp[2] =
                increase_wrap(s_context.edit_tmp[2], 5);
            break;
        case TIME_FIELD_MINUTE_LOW:
            s_context.edit_tmp[3] =
                increase_wrap(s_context.edit_tmp[3], 9);
            break;
        case TIME_FIELD_SECOND_HIGH:
            s_context.edit_tmp[4] =
                increase_wrap(s_context.edit_tmp[4], 5);
            break;
        case TIME_FIELD_SECOND_LOW:
            s_context.edit_tmp[5] =
                increase_wrap(s_context.edit_tmp[5], 9);
            break;
        default:
            break;
    }
}

static void alarm_field_decrease(void)
{
    switch (s_context.selected_field) {
        case TIME_FIELD_HOUR_HIGH:
            s_context.edit_tmp[0] =
                decrease_wrap(s_context.edit_tmp[0], 2);
            break;
        case TIME_FIELD_HOUR_LOW:
            s_context.edit_tmp[1] =
                decrease_wrap(s_context.edit_tmp[1], 9);
            break;
        case TIME_FIELD_MINUTE_HIGH:
            s_context.edit_tmp[2] =
                decrease_wrap(s_context.edit_tmp[2], 5);
            break;
        case TIME_FIELD_MINUTE_LOW:
            s_context.edit_tmp[3] =
                decrease_wrap(s_context.edit_tmp[3], 9);
            break;
        case TIME_FIELD_SECOND_HIGH:
            s_context.edit_tmp[4] =
                decrease_wrap(s_context.edit_tmp[4], 5);
            break;
        case TIME_FIELD_SECOND_LOW:
            s_context.edit_tmp[5] =
                decrease_wrap(s_context.edit_tmp[5], 9);
            break;
        default:
            break;
    }
}

static clock_ui_action_t enter_alarm_edit(void)
{
    uint8_t data[3];
    HAL_alarm_get(data);
    s_context.edit_tmp[0] = data[0] / 10;
    s_context.edit_tmp[1] = data[0] % 10;
    s_context.edit_tmp[2] = data[1] / 10;
    s_context.edit_tmp[3] = data[1] % 10;
    s_context.edit_tmp[4] = data[2] / 10;
    s_context.edit_tmp[5] = data[2] % 10;

    s_context.selected_field = TIME_FIELD_HOUR_HIGH;
    s_context.state = CLOCK_UI_EDIT;
    s_context.edit_mode = EDIT_ALARM;
    s_context.last_error = ESP_OK;

    return CLOCK_UI_ACTION_EDIT_STARTED;
}

static clock_ui_action_t save_alarm_edit(void)
{
    uint8_t data[3];
    data[0] = s_context.edit_tmp[0] * 10 + s_context.edit_tmp[1];
    data[1] = s_context.edit_tmp[2] * 10 + s_context.edit_tmp[3];
    data[2] = s_context.edit_tmp[4] * 10 + s_context.edit_tmp[5];

    esp_err_t ret = HAL_alarm_set(data[0],data[1],data[2]);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    ret = HAL_alarm_set(data[0],data[1],data[2]);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    s_context.state = CLOCK_UI_VIEW;
    s_context.edit_mode = EDIT_NULL;
    s_context.last_error = ESP_OK;

    return CLOCK_UI_ACTION_ALARM_SAVED;
}

static void calendar_field_increase(void)
{
    switch (s_context.selected_field) {
        case TIME_FIELD_HOUR_HIGH:
            s_context.edit_tmp[0] =
                increase_wrap(s_context.edit_tmp[0], 9);
            break;
        case TIME_FIELD_HOUR_LOW:
            s_context.edit_tmp[1] =
                increase_wrap(s_context.edit_tmp[1], 9);
            break;
        case TIME_FIELD_MINUTE_HIGH:
            s_context.edit_tmp[2] =
                increase_wrap(s_context.edit_tmp[2], 12);
            break;
        case TIME_FIELD_MINUTE_LOW:
            s_context.edit_tmp[3] =
                increase_wrap(s_context.edit_tmp[3], 31);
            break;
        default:
            break;
    }
}

static void calendar_field_decrease(void)
{
    switch (s_context.selected_field) {
        case TIME_FIELD_HOUR_HIGH:
            s_context.edit_tmp[0] =
                decrease_wrap(s_context.edit_tmp[0], 9);
            break;
        case TIME_FIELD_HOUR_LOW:
            s_context.edit_tmp[1] =
                decrease_wrap(s_context.edit_tmp[1], 9);
            break;
        case TIME_FIELD_MINUTE_HIGH:
            s_context.edit_tmp[2] =
                decrease_wrap(s_context.edit_tmp[2], 12);
            break;
        case TIME_FIELD_MINUTE_LOW:
        if(s_context.edit_tmp[2] == 1 || s_context.edit_tmp[2] == 3 || s_context.edit_tmp[2] == 5 ||
        s_context.edit_tmp[2] == 7 || s_context.edit_tmp[2] == 8 || s_context.edit_tmp[2] == 10 || s_context.edit_tmp[2] == 12)
            {s_context.edit_tmp[3] = decrease_wrap(s_context.edit_tmp[3], 31);}
        if(s_context.edit_tmp[2] == 4 || s_context.edit_tmp[2] == 6 || s_context.edit_tmp[2] == 9 ||s_context.edit_tmp[2] == 11)
            {s_context.edit_tmp[3] = decrease_wrap(s_context.edit_tmp[3], 30);}
        if(s_context.edit_tmp[2] == 2)
            {s_context.edit_tmp[3] = decrease_wrap(s_context.edit_tmp[3], 28);}
            break;
        default:
            break;
    }
}

static clock_ui_action_t enter_calendar_edit(void)
{
    ds3231_time_t data;
    esp_err_t ret = ds3231_get_time(&data);
    s_context.edit_tmp[0] = data.year / 10;
    s_context.edit_tmp[1] = data.year % 10;
    s_context.edit_tmp[2] = data.month;
    s_context.edit_tmp[3] = data.date;
    s_context.edit_tmp[4] = 0;
    s_context.edit_tmp[5] = 0;

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    s_context.selected_field = TIME_FIELD_HOUR_HIGH;
    s_context.state = CLOCK_UI_EDIT;
    s_context.edit_mode = EDIT_CALENDAR;
    s_context.last_error = ESP_OK;

    return CLOCK_UI_ACTION_EDIT_STARTED;
}

static int32_t calendar_days(int year, int month, int day)
{
    static const uint16_t days_before_month[12] = {
        0, 31, 59, 90, 120, 151,
        181, 212, 243, 273, 304, 334
    };

    static const uint8_t month_days[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (year < 0 || year > 99 || month < 1 || month > 12) {
        return -1;
    }

    int leap = (year % 4 == 0);
    int maximum = month_days[month - 1];

    if (month == 2) {
        maximum += leap;
    }

    if (day < 1 || day > maximum) {
        return -1;
    }

    return 365 * year
         + (year + 3) / 4
         + days_before_month[month - 1]
         + ((month > 2) ? leap : 0)
         + day - 1;
}

static clock_ui_action_t save_calendar_edit(void)
{
    ds3231_time_t current_time;

    esp_err_t ret = ds3231_get_time(&current_time);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    int32_t old_days = calendar_days(
    current_time.year,
    current_time.month,
    current_time.date);

    int32_t new_days = calendar_days(
    s_context.edit_tmp[0] * 10 + s_context.edit_tmp[1],
    s_context.edit_tmp[2],
    s_context.edit_tmp[3]);

    ret = HAL_memory_write_date_offset(HAL_memory_read_date_offset() + new_days - old_days);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    current_time.year = s_context.edit_tmp[0] * 10 + s_context.edit_tmp[1];
    current_time.month = s_context.edit_tmp[2];
    current_time.date = s_context.edit_tmp[3];
    current_time.weekday = (uint8_t)((new_days + 6) % 7 + 1);

    ret = ds3231_set_time(&current_time);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    MEMORY_DATE_OFFSET = HAL_memory_read_date_offset();
    s_context.state = CLOCK_UI_VIEW;
    s_context.edit_mode = EDIT_NULL;
    s_context.last_error = ESP_OK;

    return CLOCK_UI_ACTION_CALENDAR_SAVED;
}

static void brightness_field_increase(void)
{
    switch (s_context.selected_field) {
        case TIME_FIELD_HOUR_HIGH:
            s_context.edit_tmp[0] =
                increase_wrap(s_context.edit_tmp[0], 15);
            break;
        default:
            break;
    }
}

static void brightness_field_decrease(void)
{
    switch (s_context.selected_field) {
        case TIME_FIELD_HOUR_HIGH:
            s_context.edit_tmp[0] =
                decrease_wrap(s_context.edit_tmp[0], 15);
            break;
        default:
            break;
    }
}

static clock_ui_action_t enter_brightness_edit(void)
{
    int data = HAL_memory_read_intensity();
    
    s_context.edit_tmp[0] = data;

    s_context.selected_field = TIME_FIELD_HOUR_HIGH;
    s_context.state = CLOCK_UI_EDIT;
    s_context.edit_mode = EDIT_BRIGHTNESS;
    s_context.last_error = ESP_OK;

    return CLOCK_UI_ACTION_EDIT_STARTED;
}

static clock_ui_action_t save_brightness_edit(void)
{
    int data = s_context.edit_tmp[0];

    esp_err_t ret = HAL_display_set_intensity(data);

    if (ret != ESP_OK) {
        s_context.last_error = ret;
        return CLOCK_UI_ACTION_ERROR;
    }

    s_context.state = CLOCK_UI_VIEW;
    s_context.edit_mode = EDIT_NULL;
    s_context.last_error = ESP_OK;

    return CLOCK_UI_ACTION_BRIGHTNESS_SAVED;
}

static void edit_increase(void)
{
    switch (s_context.mode)
    {
        case CLOCK_MODE_TIME:
            time_field_increase();
            break;
        case CLOCK_MODE_ALARM:
            alarm_field_increase();
            break;
        case CLOCK_MODE_CALENDAR:
            calendar_field_increase();
            break;          
        case CLOCK_MODE_BRIGHTNESS:
            brightness_field_increase();
            break;
        default:
    }
}

static void edit_decrease(void)
{
    switch (s_context.mode)
    {
        case CLOCK_MODE_TIME:
            time_field_decrease();
            break;
        case CLOCK_MODE_ALARM:
            alarm_field_decrease();
            break;
        case CLOCK_MODE_CALENDAR:
            calendar_field_decrease();
            break;          
        case CLOCK_MODE_BRIGHTNESS:
            brightness_field_decrease();
            break;
        default:
    }
}

static clock_ui_action_t save_edit(void)
{
    switch (s_context.mode)
    {
        case CLOCK_MODE_TIME:
            return save_time_edit();
        case CLOCK_MODE_ALARM:
            return save_alarm_edit();
        case CLOCK_MODE_CALENDAR:
            return save_calendar_edit();      
        case CLOCK_MODE_BRIGHTNESS:
            return save_brightness_edit();
        default:
            return CLOCK_UI_ACTION_ERROR;
    }
}

static clock_ui_action_t enter_edit(void)
{
    ESP_LOGI(TAG,"state:%d",s_context.mode);
    switch (s_context.mode)
    {
        case CLOCK_MODE_TIME:
            return enter_time_edit();
        case CLOCK_MODE_ALARM:
            return enter_alarm_edit();
        case CLOCK_MODE_CALENDAR:
            return enter_calendar_edit();       
        case CLOCK_MODE_BRIGHTNESS:
            return enter_brightness_edit();
        default:
            return CLOCK_UI_ACTION_ERROR;
    }
}

static bool TIMER_MODE_BIT;

static clock_ui_action_t handle_view_event(key_event_t event)
{
    switch (event) {
        case KEY_EVENT_UP:
            if (s_context.mode == CLOCK_MODE_TIME) 
                s_context.mode = CLOCK_MODE_BRIGHTNESS;
            else 
                s_context.mode--;
            ESP_LOGI(TAG,"mode:%d",s_context.mode);

            return CLOCK_UI_ACTION_MODE_CHANGED;

        case KEY_EVENT_DOWN:
            s_context.mode =
                (clock_mode_t)((s_context.mode + 1) % CLOCK_MODE_COUNT);

            ESP_LOGI(TAG,"mode:%d",s_context.mode);
            return CLOCK_UI_ACTION_MODE_CHANGED;

        case KEY_EVENT_PRESS_LONG:
            if(s_context.mode != CLOCK_MODE_TIMER)
                return enter_edit();
            else{
                HAL_stopwatch_reset();
                TIMER_MODE_BIT = 0;
                ESP_LOGI(TAG, "timer reset");
            }return CLOCK_UI_ACTION_NONE;
        case KEY_EVENT_PRESS_TWICE:
            if(s_context.mode == CLOCK_MODE_TIMER){
            if(TIMER_MODE_BIT){
                    HAL_stopwatch_pause();
                    TIMER_MODE_BIT = 0;
                    ESP_LOGI(TAG, "timer paused");
                }

                else{
                    HAL_stopwatch_start();
                    TIMER_MODE_BIT = 1;
                     ESP_LOGI(TAG, "timer started");
                }
                return CLOCK_UI_ACTION_NONE;
            }
             return CLOCK_UI_ACTION_NONE;   
        case KEY_EVENT_NONE:
        default:
            return CLOCK_UI_ACTION_NONE;
    }
}

static clock_ui_action_t handle_edit_event(key_event_t event)
{
    switch (event) {
        case KEY_EVENT_PRESS_TWICE:
            if (s_context.selected_field == TIME_FIELD_SECOND_LOW) 
                s_context.selected_field = TIME_FIELD_HOUR_HIGH;
            else 
                s_context.selected_field++;
    
            return CLOCK_UI_ACTION_FIELD_CHANGED;

        case KEY_EVENT_UP:
            edit_increase();
            return CLOCK_UI_ACTION_VALUE_CHANGED;

        case KEY_EVENT_DOWN:
            edit_decrease();
            return CLOCK_UI_ACTION_VALUE_CHANGED;

        case KEY_EVENT_PRESS_LONG:
            return save_edit();

        case KEY_EVENT_NONE:
        default:
            return CLOCK_UI_ACTION_NONE;
    }
}

esp_err_t HAL_key_control_init(void)
{
    esp_err_t ret = key_init();

    if(ret != ESP_OK)
        return ret;

    s_context.mode = CLOCK_MODE_TIME;
    s_context.state = CLOCK_UI_VIEW;
    s_context.edit_mode = EDIT_NULL;
    TIMER_MODE_BIT = 0;
    s_context.selected_field = TIME_FIELD_HOUR_HIGH;
    for(int i = 0 ; i < 6 ; i++)
    {s_context.edit_tmp[i] = 0;}
    s_context.last_error = ESP_OK;

    return ESP_OK;
}

clock_ui_action_t HAL_key_control_handle(key_event_t event)
{
    if (event == KEY_EVENT_NONE) {
        return CLOCK_UI_ACTION_NONE;
    }

    if (s_context.state == CLOCK_UI_EDIT) {
        return handle_edit_event(event);
    }

    return handle_view_event(event);
}

const clock_ui_context_t *HAL_key_control_get_context(void)
{
    return &s_context;
}