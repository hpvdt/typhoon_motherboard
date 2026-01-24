/* Blink Example - Custom Typhoon Motherboard
 * Toggles 4 LEDs on GPIO pins 5, 6, 7, and 8
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "typhoon_blink";

/* LED GPIO pins for custom Typhoon motherboard */
#define LED_GPIO_5  5
#define LED_GPIO_6  6
#define LED_GPIO_7  7
#define LED_GPIO_8  8

/* Blink period from config (default 1000ms) */
#define BLINK_PERIOD CONFIG_BLINK_PERIOD

static uint8_t s_led_state = 0;

static void configure_leds(void)
{
    ESP_LOGI(TAG, "Configuring GPIO LEDs on pins 5, 6, 7, 8");

    /* Reset and configure all 4 LED pins as outputs */
    gpio_reset_pin(LED_GPIO_5);
    gpio_reset_pin(LED_GPIO_6);
    gpio_reset_pin(LED_GPIO_7);
    gpio_reset_pin(LED_GPIO_8);

    gpio_set_direction(LED_GPIO_5, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_6, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_7, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_8, GPIO_MODE_OUTPUT);
}

static void toggle_leds(void)
{
    /* Set all GPIO levels to the same state (all on or all off) */
    gpio_set_level(LED_GPIO_5, s_led_state);
    vTaskDelay((BLINK_PERIOD / portTICK_PERIOD_MS) / 4);
    gpio_set_level(LED_GPIO_6, s_led_state);
    vTaskDelay((BLINK_PERIOD / portTICK_PERIOD_MS) / 4);
    gpio_set_level(LED_GPIO_7, s_led_state);
    vTaskDelay((BLINK_PERIOD / portTICK_PERIOD_MS) / 4);
    gpio_set_level(LED_GPIO_8, s_led_state);
}

void app_main(void)
{
    configure_leds();

    while (1) {
        ESP_LOGI(TAG, "LEDs %s", s_led_state ? "ON" : "OFF");
        toggle_leds();

        /* Toggle state for next iteration */
        s_led_state = !s_led_state;

        /* Wait for configured period */
        vTaskDelay(BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
