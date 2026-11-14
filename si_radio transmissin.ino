#include <Wire.h>
#include <SI470X.h>

SI470X radio;

#define RESET_PIN 2
#define SDA_PIN A4

void setup() {
  Wire.begin();

  radio.setup(RESET_PIN, SDA_PIN);

  radio.setVolume(15);
  radio.setFrequency(9830);   // 104.0 MHz
}

void loop() {
}
