#include "ds3231.h"

i2c_obj_t ds3231_i2c_master;

static esp_err_t ds3231_read_byte(uint8_t reg, uint8_t *data, size_t len)
{

    i2c_buf_t bufs[2] = {
        {.len = 1, .buf = &reg},
        {.len = len, .buf = data},
    };

    return i2c_transfer(&ds3231_i2c_master, DS3231_ADDR, 2, bufs, I2C_FLAG_WRITE | I2C_FLAG_READ | I2C_FLAG_STOP);
}

static esp_err_t ds3231_write_byte(uint8_t reg, uint8_t *data, size_t len)
{

    i2c_buf_t bufs[2] = {
        {.len = 1, .buf = &reg},
        {.len = len, .buf = data},
    };

    return i2c_transfer(&ds3231_i2c_master, DS3231_ADDR, 2, bufs, I2C_FLAG_STOP);
}

static uint8_t dec_to_bcd(uint8_t value)
{
    return ((value / 10) << 4) |
           (value % 10);
}

static uint8_t bcd_to_dec(uint8_t value)
{
    return ((value >> 4) * 10) +
           (value & 0x0F);
}

esp_err_t ds3231_init(i2c_obj_t self)
{
    if (self.init_flag != ESP_OK) {
        return ESP_ERR_INVALID_STATE;
    }

    ds3231_i2c_master = self;

    uint8_t status;
    return ds3231_read_byte(
        DS3231_STATUS_REG,
        &status,
        1
    );
}

esp_err_t ds3231_get_time(ds3231_time_t *time)
{
    uint8_t data[7];
    esp_err_t ret = ds3231_read_byte(
        DS3231_SECONDS_REG,
        data,
        sizeof(data)
    );

    if (ret != ESP_OK) {
        return ret;
    }

    time->seconds = bcd_to_dec(data[0]);
    time->minutes = bcd_to_dec(data[1]);
    time->hours = bcd_to_dec(data[2] & 0x1F);
    int mode = data[2] & 0x40; // Check if 12-hour mode is set
    if(mode){
        uint8_t pm = (data[2] & 0x20);
        if (pm) {
            time->hours += 12;
        }
    }
    else
    {
        time->hours = bcd_to_dec(data[2] & 0x3F); // 24-hour mode
    }
    time->weekday = bcd_to_dec(data[3]);
    time->date = bcd_to_dec(data[4]);
    time->month = bcd_to_dec(data[5] & 0x1F); // Mask century bit
    time->year = bcd_to_dec(data[6]);

    return ESP_OK;
}

esp_err_t ds3231_set_time(ds3231_time_t *time)
{
    uint8_t data[7];

    data[0] = dec_to_bcd(time->seconds);
    data[1] = dec_to_bcd(time->minutes);
    data[2] = dec_to_bcd(time->hours);
    data[3] = dec_to_bcd(time->weekday);
    data[4] = dec_to_bcd(time->date);
    data[5] = dec_to_bcd(time->month);
    data[6] = dec_to_bcd(time->year);

    return ds3231_write_byte(
        DS3231_SECONDS_REG,
        data,
        sizeof(data)
    );
}

esp_err_t ds3231_change_time_mode(void)
{
    uint8_t data;
    esp_err_t ret = ds3231_read_byte(
        DS3231_HOURS_REG,
        &data,
        1
    );

    int mode = data & 0x40;
    if(mode) {
        data &= ~0x40; // Clear 12-hour mode bit
    } else {
        data |= 0x40; // Set 12-hour mode bit
    }
    return ds3231_write_byte(DS3231_HOURS_REG, &data, 1);
}

