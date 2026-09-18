#include "max7219.h"
#include "spi.h"
#include "esp_log.h"

spi_device_handle_t max7219_handle;
static int num = 9;

static void max7219_write_data(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};

    spi2_write_data(max7219_handle, data, sizeof(data));
}

void max7219_init(void)
{
    esp_err_t ret;

    spi_device_interface_config_t max7219_device_config = {
        .clock_speed_hz = 1*1000*1000,
        .mode = 0,
        .spics_io_num = MAX7219_CS_PIN,
        .queue_size = 7,
    };

    ret = spi_bus_add_device(SPI2_HOST, &max7219_device_config, &max7219_handle);
    
    uint8_t data[num * 2];
    for(int j = 0; j < sizeof(max7219_init_data) / sizeof(max7219_init_data[0]); j++)
    {
        for(int i = 0; i < num * 2; i += 2)
        {
            data[i] = max7219_init_data[j][0];
            data[i + 1] = max7219_init_data[j][1];
        }
        spi2_write_data(max7219_handle, data, sizeof(data));
   
    if (ret != ESP_OK) {
        ESP_LOGE("MAX7219", "Failed to add MAX7219 device to SPI bus");
        return;
    }
}}


void max7219_write_number(int *number)
{
    uint8_t data[2 * num];
    for (int i = 0; i < 8; i++) {
        for(int j = 0; j < num; j++)
    switch (number[j]) {
        case 0:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[0][i];
            break;
        case 1:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[1][i];
            break;
        case 2:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[2][i];
            break;
        case 3:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[3][i];
            break;
        case 4:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[4][i];
            break;
        case 5:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[5][i];
            break;
        case 6:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[6][i];
            break;
        case 7:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[7][i];
            break;
        case 8:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[8][i];
            break;
        case 9:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[9][i];
            break;
        case 10:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[10][i];
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            data[j*2] = 0x01 + i;
            data[j*2+1] = max7219_digit_font[number[j]][i];
            break;
        default:
            ESP_LOGW("MAX7219", "Unsupported number: %d", number[j]);
            break;
    }
            spi2_write_data(max7219_handle, data, sizeof(data));
}
}   


void max7219_intensity_change(int number)
{
    uint8_t data[2 * num];
    if (number < 0 || number > 15) {
        ESP_LOGW("MAX7219", "Intensity value out of range: %d", number);
        return;
    }
    for(int i = 0; i < num * 2; i += 2) {
        data[i] = MAX7219_REG_INTENSITY;
        data[i + 1] = number;
    }
    spi2_write_data(max7219_handle, data, sizeof(data));
}