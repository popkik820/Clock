#include "stopwatch.h"
#include "timer.h"

typedef struct{
    bool mode;
    int64_t start_time ;
    int64_t accumulated_time;
} stopwatch_t;

static stopwatch_t s_stopwatch;

esp_err_t HAL_stopwatch_init(void)
{
    s_stopwatch.mode = false;
    s_stopwatch.start_time = 0;
    s_stopwatch.accumulated_time = 0;
    return ESP_OK;
}

void HAL_stopwatch_start(void)
{
    if (s_stopwatch.mode) {
        return;
    }

    s_stopwatch.start_time = BSP_timer_get_time_us();
    s_stopwatch.mode = true;
}

void HAL_stopwatch_pause(void)
{
    if (!s_stopwatch.mode) {
        return;
    }

    s_stopwatch.accumulated_time +=
        BSP_timer_get_time_us() - s_stopwatch.start_time;

    s_stopwatch.mode = false;
}

void HAL_stopwatch_reset(void)
{
    s_stopwatch.mode = false;
    s_stopwatch.start_time = 0;
    s_stopwatch.accumulated_time = 0;
}

uint64_t HAL_stopwatch_get_time(void)
{
    int64_t elapsed_us = s_stopwatch.accumulated_time;

    if (s_stopwatch.mode) {
        elapsed_us +=
            BSP_timer_get_time_us() -
            s_stopwatch.start_time;
    }

    return (uint64_t)(elapsed_us / 1000);
}

bool HAL_stopwatch_mode(void)
{
    return s_stopwatch.mode;
}