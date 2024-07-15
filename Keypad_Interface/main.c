#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN1 GPIO_NUM_4   // Column 4
#define PIN2 GPIO_NUM_5   // Column 3
#define PIN3 GPIO_NUM_18  // Column 2
#define PIN4 GPIO_NUM_19  // Column 1
#define PIN5 GPIO_NUM_21  // Row 4
#define PIN6 GPIO_NUM_22  // Row 3
#define PIN7 GPIO_NUM_23  // Row 2
#define PIN8 GPIO_NUM_25  // Row 1

#define DEBOUNCE_DELAY 50  // milliseconds

static const char *TAG = "keypad";

uint32_t Button_pressed = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    Button_pressed = gpio_num;
}

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_INFO);

   // ESP_LOGI(TAG, "Starting app_main");

    gpio_config_t col_config = {
        .pin_bit_mask = (1ULL << PIN1) | (1ULL << PIN2) | (1ULL << PIN3) | (1ULL << PIN4),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&col_config);
   // ESP_LOGI(TAG, "Configured column pins as outputs");

    gpio_config_t row_config = {
        .pin_bit_mask = (1ULL << PIN5) | (1ULL << PIN6) | (1ULL << PIN7) | (1ULL << PIN8),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&row_config);
   // ESP_LOGI(TAG, "Configured row pins as inputs with pull-up resistors");

    gpio_install_isr_service(0);
    //ESP_LOGI(TAG, "Installed ISR service");

    gpio_isr_handler_add(PIN5, gpio_isr_handler, (void*) PIN5);
    gpio_isr_handler_add(PIN6, gpio_isr_handler, (void*) PIN6);
    gpio_isr_handler_add(PIN7, gpio_isr_handler, (void*) PIN7);
    gpio_isr_handler_add(PIN8, gpio_isr_handler, (void*) PIN8);
  //  ESP_LOGI(TAG, "Attached ISR handler to row pins");

    const int col_pins[] = { PIN4, PIN3, PIN2, PIN1 };
    const char key_map[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    while (1) {
        if (Button_pressed != 0) {
            uint32_t pressed_row = Button_pressed;
            int row = -1;

            if (pressed_row == PIN8) {
                row = 0;
            } else if (pressed_row == PIN7) {
                row = 1;
            } else if (pressed_row == PIN6) {
                row = 2;
            } else if (pressed_row == PIN5) {
                row = 3;
            }

            if (row != -1) {
                for (int col = 0; col < 4; col++) {
                    // Set all columns to low
                    gpio_set_level(PIN1, 0);
                    gpio_set_level(PIN2, 0);
                    gpio_set_level(PIN3, 0);
                    gpio_set_level(PIN4, 0);

                    // Set the corresponding column pin high
                    gpio_set_level(col_pins[col], 1);
                    vTaskDelay(DEBOUNCE_DELAY / portTICK_PERIOD_MS);

                    // Check if the current row pin is high
                    if (gpio_get_level(pressed_row) == 1) {
                        ESP_LOGI(TAG, "Button %c pressed", key_map[row][col]);
                        // Debounce the button press
                        vTaskDelay(DEBOUNCE_DELAY / portTICK_PERIOD_MS);
                        break;  // Exit the loop once the pressed key is found
                    }

                    // Reset the column pin to low
                    gpio_set_level(col_pins[col], 0);
                }
            }
            Button_pressed = 0;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
