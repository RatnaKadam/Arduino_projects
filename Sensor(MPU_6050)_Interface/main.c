#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "mpu6050.h"
#include "esp_log.h"


#define SDA_PIN GPIO_NUM_13
#define SCL_PIN GPIO_NUM_14
#define I2C_FREQ_HZ 400000

#define I2C_PORT_NUM I2C_NUM_0
#define MPU6050_ADDRESS  MPU6050_I2C_ADDRESS

const char *TAG = "MAIN";
mpu6050_handle_t mpuHandel;
mpu6050_raw_acce_value_t accelerationValues;
uint8_t deviceid;

void app_main(void)
{
    
    i2c_config_t i2cConfig;
    i2cConfig.mode = I2C_MODE_MASTER;
    i2cConfig.scl_io_num = SCL_PIN;
    i2cConfig.sda_io_num = SDA_PIN;
    i2cConfig.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2cConfig.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2cConfig.master.clk_speed = I2C_FREQ_HZ;

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT_NUM,&i2cConfig));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0,i2cConfig.mode,0,0,0));

    mpuHandel = mpu6050_create(I2C_PORT_NUM,MPU6050_ADDRESS);

    ESP_ERROR_CHECK(mpu6050_config(mpuHandel,ACCE_FS_2G,GYRO_FS_250DPS));
    ESP_ERROR_CHECK(mpu6050_wake_up(mpuHandel));


    for (;;)
    {
        ESP_ERROR_CHECK(mpu6050_get_deviceid(mpuHandel,&deviceid));
        ESP_ERROR_CHECK_WITHOUT_ABORT(mpu6050_get_raw_acce(mpuHandel,&accelerationValues));
        ESP_LOGI(TAG,"device Id: %d, Acceleration: xyz[]",deviceid,accelerationValues.raw_acce_x,accelerationValues.raw_acce_y,accelerationValues.raw_acce_z);
        vTaskDelay(2000);
    }
    

}