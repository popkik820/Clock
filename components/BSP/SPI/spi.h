#ifndef __SPI_H
#define __SPI_H

#include <string.h>
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"


/* 引脚定义 */
#define SPI_MOSI_GPIO_PIN   GPIO_NUM_11        /* SPI2_MOSI */
#define SPI_CLK_GPIO_PIN    GPIO_NUM_8         /* SPI2_CLK */
#define SPI_MISO_GPIO_PIN   GPIO_NUM_12         /* SPI2_MISO */

/* 函数声明 */
void spi2_init(void);                                                               /* 初始化SPI2 */
void spi2_write_cmd(spi_device_handle_t handle, uint8_t cmd);                       /* SPI发送命令 */
void spi2_write_data(spi_device_handle_t handle, const uint8_t *data, int len);     /* SPI发送数据 */
uint8_t spi2_transfer_byte(spi_device_handle_t handle, uint8_t byte);               /* SPI处理数据 */
#endif
