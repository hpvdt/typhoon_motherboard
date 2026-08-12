#include <Arduino.h>
#include "driver/twai.h"

constexpr gpio_num_t CAN_TX_PIN = GPIO_NUM_43;
constexpr gpio_num_t CAN_RX_PIN = GPIO_NUM_44;

constexpr uint32_t LED_COMMAND_ID = 0x200;
constexpr uint32_t LED_STATUS_ID  = 0x201;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    twai_general_config_t generalConfig =
        TWAI_GENERAL_CONFIG_DEFAULT(
            CAN_TX_PIN,
            CAN_RX_PIN,
            TWAI_MODE_NORMAL
        );

    twai_timing_config_t timingConfig =
        TWAI_TIMING_CONFIG_500KBITS();

    twai_filter_config_t filterConfig =
        TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(
            &generalConfig,
            &timingConfig,
            &filterConfig
        ) != ESP_OK) {

        Serial.println("CAN installation failed");

        while (true)
            delay(1000);
    }

    if (twai_start() != ESP_OK) {
        Serial.println("CAN start failed");

        while (true)
            delay(1000);
    }

    Serial.println("CAN LED controller started");
}

void loop()
{
    static bool ledOn = false;
    static uint8_t sequence = 0;

    ledOn = !ledOn;

    twai_message_t command = {};

    command.identifier = LED_COMMAND_ID;
    command.data_length_code = 2;

    command.data[0] = ledOn ? 1 : 0;
    command.data[1] = sequence;

    if (twai_transmit(
            &command,
            pdMS_TO_TICKS(100)
        ) == ESP_OK) {

        Serial.print("LED command sent: ");
        Serial.println(ledOn ? "ON" : "OFF");
    }
    else {
        Serial.println("LED command failed");
    }

    twai_message_t response = {};

    if (twai_receive(
            &response,
            pdMS_TO_TICKS(500)
        ) == ESP_OK) {

        if (response.identifier == LED_STATUS_ID &&
            response.data_length_code >= 2 &&
            response.data[1] == sequence) {

            Serial.print("Receiver confirmed LED: ");
            Serial.println(response.data[0] ? "ON" : "OFF");
        }
    }
    else {
        Serial.println("No response from receiver");
    }

    sequence++;

    delay(1000);
}