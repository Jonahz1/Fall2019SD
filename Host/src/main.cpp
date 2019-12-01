#include <Arduino.h>
#include <LoRa.h>

void setup() {
  Serial.begin(9600);
  delay(100);
}

void loop() {
  Serial.println("Hello World");
  delay(1000);
}