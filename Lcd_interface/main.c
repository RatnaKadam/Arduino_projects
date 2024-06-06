#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO           18    /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           19    /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_1 /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          100000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0 /*!< I2C master doesn't need buffer */
#define I2C_LCD_ADDR                0x27 /*!< I2C address of the LCD */

// Commands
#define LCD_CMD_CLEAR_DISPLAY       0x01
#define LCD_CMD_RETURN_HOME         0x02
#define LCD_CMD_ENTRY_MODE_SET      0x04
#define LCD_CMD_DISPLAY_CONTROL     0x08
#define LCD_CMD_CURSOR_SHIFT        0x10
#define LCD_CMD_FUNCTION_SET        0x20
#define LCD_CMD_SET_CGRAM_ADDR      0x40
#define LCD_CMD_SET_DDRAM_ADDR      0x80

#define LCD_FLAG_DISPLAY_ON         0x04
#define LCD_FLAG_DISPLAY_OFF        0x00
#define LCD_FLAG_CURSOR_ON          0x02
#define LCD_FLAG_CURSOR_OFF         0x00
#define LCD_FLAG_BLINK_ON           0x01
#define LCD_FLAG_BLINK_OFF          0x00

#define LCD_FLAG_8BIT_MODE          0x10
#define LCD_FLAG_4BIT_MODE          0x00
#define LCD_FLAG_2LINE              0x08
#define LCD_FLAG_1LINE              0x00
#define LCD_FLAG_5x10DOTS           0x04
#define LCD_FLAG_5x8DOTS            0x00

static const char *TAG = "lcd";

// Function prototypes
static esp_err_t i2c_master_init(void);
static esp_err_t lcd_send_cmd(uint8_t cmd);
static esp_err_t lcd_send_data(uint8_t data);
static void lcd_init(void);
static void lcd_print(const char *str);

void app_main(void) {
    ESP_ERROR_CHECK(i2c_master_init());
    lcd_init();
    lcd_print("Hello, Rushmay!");

    while (1) {
       
       vTaskDelay(pdMS_TO_TICKS(1000));
      
    }
}

static esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                                       I2C_MASTER_RX_BUF_DISABLE,
                                       I2C_MASTER_TX_BUF_DISABLE, 0));
    return ESP_OK;
}

static esp_err_t lcd_send_cmd(uint8_t cmd) {
    uint8_t data[4];
    data[0] = (cmd & 0xF0) | 0x0C;
    data[1] = (cmd & 0xF0) | 0x08;
    data[2] = ((cmd << 4) & 0xF0) | 0x0C;
    data[3] = ((cmd << 4) & 0xF0) | 0x08;
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (I2C_LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, data, sizeof(data), true);
    i2c_master_stop(handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(handle);
    return ret;
}

static esp_err_t lcd_send_data(uint8_t data) {
    uint8_t data_arr[4];
    data_arr[0] = (data & 0xF0) | 0x0D;
    data_arr[1] = (data & 0xF0) | 0x09;
    data_arr[2] = ((data << 4) & 0xF0) | 0x0D;
    data_arr[3] = ((data << 4) & 0xF0) | 0x09;
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (I2C_LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, data_arr, sizeof(data_arr), true);
    i2c_master_stop(handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(handle);
    return ret;
}

static void lcd_init(void) {
    vTaskDelay(pdMS_TO_TICKS(50)); // wait for >40ms
    lcd_send_cmd(0x30);
    vTaskDelay(pdMS_TO_TICKS(5));  // wait for >4.1ms
    lcd_send_cmd(0x30);
    vTaskDelay(pdMS_TO_TICKS(1));  // wait for >100us
    lcd_send_cmd(0x30);
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_send_cmd(0x20);  // 4-bit mode
    vTaskDelay(pdMS_TO_TICKS(10));

    lcd_send_cmd(LCD_CMD_FUNCTION_SET | LCD_FLAG_2LINE | LCD_FLAG_5x8DOTS);
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_send_cmd(LCD_CMD_DISPLAY_CONTROL | LCD_FLAG_DISPLAY_ON | LCD_FLAG_CURSOR_OFF | LCD_FLAG_BLINK_OFF);
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_send_cmd(LCD_CMD_CLEAR_DISPLAY);
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_send_cmd(LCD_CMD_ENTRY_MODE_SET | 0x02);
    vTaskDelay(pdMS_TO_TICKS(10));
}

static void lcd_print(const char *str) {
    while (*str) {
        lcd_send_data((uint8_t)(*str));
        str++;
    }
}
