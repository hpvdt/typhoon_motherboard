#include <Arduino.h>

#define STATUS_LED_PIN 5 
void setup() {
  pinMode(STATUS_LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(STATUS_LED_PIN, HIGH);  // Turn LED on
  delay(500);                          // Wait 500ms
  digitalWrite(STATUS_LED_PIN, LOW);   // Turn LED off
  delay(500);                          // Wait 500ms
}