#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-S3 LED Test Starting...");

  // Test multiple pins
  pinMode(7, OUTPUT);  // Built-in LED (if available)
  pinMode(5, OUTPUT);  // Your original pin
  pinMode(6, OUTPUT);  // Additional test pin

  Serial.println("Setup complete");
}

void loop() {
  // Test all pins
  digitalWrite(7, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  Serial.println("All LEDs ON");
  delay(1000);

  digitalWrite(7, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  Serial.println("All LEDs OFF");
  delay(1000);
}