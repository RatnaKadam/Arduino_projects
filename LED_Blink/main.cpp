#include <stdio.h>
#include "freertos/FreeRTos.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <Led.h>

#define LED_PIN GPIO_NUM_18
//static const char *TAG = "MAIN";

Led *led1;

extern "C" void app_main(void)
{   
    led1 = new Led(LED_PIN);
   
    for (size_t i = 0; i<10; i++)
    {
        led1->on();
        vTaskDelay(pdMS_TO_TICKS(1000));

        led1->off();
        vTaskDelay(pdMS_TO_TICKS(1000));

    }

    delete led1;
}