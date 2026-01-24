/* CAN Prototype - Custom Typhoon Motherboard
 * CAN bus communication with LED status indicators
 */

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/twai.h" // For CAN
#include "esp_log.h"
#include "sdkconfig.h"

/* -------------------------------------------------------------------------- */
/*                               Configurations                               */
/* -------------------------------------------------------------------------- */
#define CAN_LOOPBACK_TEST

/* -------------------------------------------------------------------------- */
/*                              Global Variables                              */
/* -------------------------------------------------------------------------- */

static const char *TAG = "CAN_PROTOTYPE";
static uint8_t s_led_state = 0;
/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

/* ----------------------------- GPIO DEFINITION ---------------------------- */
#define LED_GPIO_5  5
#define LED_GPIO_6  6
#define LED_GPIO_7  7
#define LED_GPIO_8  8

#define BLINK_PERIOD CONFIG_BLINK_PERIOD

// CAN Pins
#define CAN_TX_PIN GPIO_NUM_37
#define CAN_RX_PIN GPIO_NUM_36

/* --------------------------- CAN ID Definitions --------------------------- */
#define CAN_ID_MOTHERBOARD  0x100
#define CAN_ID_ENV_SENSOR   0x200
#define CAN_ID_SPEED_CADENCE 0x300

/* ----------------------- CAN Message Type Definition ---------------------- */
#define MSG_HEARTBEAT       0x00
#define MSG_SENSOR_DATA     0x01
#define MSG_COMMAND         0x02

/* -------------------------------------------------------------------------- */
/*                            Function Definitions                            */
/* -------------------------------------------------------------------------- */ 


/**
 * @brief configures the LEDs
 * 
 */
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

/**
 * @brief Toggles the state of all 4 LEDs -> used to check programmability
 * 
 */
static void toggle_leds(void)
{
    /* Set all GPIO levels to the same state (all on or all off) */
    gpio_set_level(LED_GPIO_5, s_led_state);
    gpio_set_level(LED_GPIO_6, s_led_state);
    gpio_set_level(LED_GPIO_7, s_led_state);
    gpio_set_level(LED_GPIO_8, s_led_state);
}

/**
 * @brief initialize CAN stuff
 * 
 */
void can_init(void){
    // Configure timing for 500 kbps
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

    // Configure filter to accept all messages
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    #ifdef CAN_LOOPBACK_TEST
    // Configure general settings - Loopback Mode
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        CAN_TX_PIN,
        CAN_RX_PIN,
        TWAI_MODE_NO_ACK // loopback mode for testing
    );
    #endif

    // Set queue lengths
    g_config.tx_queue_len = 5;
    g_config.rx_queue_len = 5;

    // Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(TAG, "TWAI driver installed");

    // Start TWAI driver
    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(TAG, "TWAI driver started - LOOPBACK MODE");
}

/**
 * @brief Sending Message
 * 
 * @param id 
 * @param data 
 * @param len 
 * @return esp_err_t 
 */
esp_err_t can_send_message(uint32_t id, uint8_t *data, uint8_t len){
    twai_message_t message;
    message.identifier = id;
    message.data_length_code = len;
    message.extd = 0; // Standard 11-bit ID
    message.rtr = 0; // Data frame, not remote request

    // Copy data to message 
    for(int i = 0; i < len && i < 8; i ++){
        message.data[i] = data[i];
    }

    // Transmit Message
    esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(1000));
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "Message sent - ID: 0x%03" PRIX32 ", Len: %d", id, len);
    } else {
        ESP_LOGW(TAG, "Failed to send message: %s", esp_err_to_name(result));
    }
    return result;
}

void can_receive_task(void *arg){
    twai_message_t message;

    while (1) {
        // Wait for message (blocking with timeout)
        if (twai_receive(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
            ESP_LOGI(TAG, "Received - ID: 0x%03" PRIX32 ", DLC: %d, Data: ",
                     message.identifier, message.data_length_code);
            
            // Print data bytes
            for (int i = 0; i < message.data_length_code; i++) {
                printf("%02X ", message.data[i]);
            }
            printf("\n");
            
            // Handle different message types
            switch(message.identifier) {
                case CAN_ID_MOTHERBOARD:
                    ESP_LOGI(TAG, "Message from Motherboard");
                    break;
                case CAN_ID_ENV_SENSOR:
                    ESP_LOGI(TAG, "Message from ENV sensor");
                    break;
                case CAN_ID_SPEED_CADENCE:
                    ESP_LOGI(TAG, "Message from Speed/Cadence sensor");
                    break;
                default:
                    ESP_LOGI(TAG, "Message from unknown device");
            }
        }
    }
}

void can_heartbeat_task(void *arg){
    uint8_t counter = 0;
    uint8_t data[2];

    while(1){
        // Prepare heartbeat message 
        data[0] = MSG_HEARTBEAT;
        data[1] = counter++;

        // Send heartbeat
        can_send_message(CAN_ID_MOTHERBOARD, data, 2);

        // Wait
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/* -------------------------------------------------------------------------- */
/*                                Main Function                               */
/* -------------------------------------------------------------------------- */
void app_main(void)
{
    ESP_LOGI(TAG, "=== Typhoon Motherboard CAN system ===");
    ESP_LOGI(TAG, "Starting loopback test...");
    
    // Initiate CAN
    can_init();

    // Create receive task
    xTaskCreate(can_receive_task, "can_rx", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "CAN receive task created");
    
    // Create heartbeat task
    xTaskCreate(can_heartbeat_task, "can_hb", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "CAN heartbeat task created");
    
    ESP_LOGI(TAG, "All CAN tasks started - monitoring...");

    // Configure LEDs
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
