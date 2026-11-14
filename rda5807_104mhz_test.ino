/*
  MINIMAL RDA5807 TEST — fixed at 104.0 MHz
  -------------------------------------------
  Wire ONLY the RDA5807 to the Nano (no transmitter connected):
    RDA5807 VCC -> 5V (or 3.3V if module needs it)
    RDA5807 GND -> GND
    RDA5807 SDA -> A4
    RDA5807 SCL -> A5

  Upload, open Serial Monitor at 9600 baud.
  If it finds the chip, it'll lock to 104.0 MHz and print live RSSI.
*/

#include <Wire.h>
#include <RDA5807.h>

RDA5807 radio;
uint16_t FREQ = 10400; // 104.00 MHz

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println(F("=== Minimal RDA5807 test @ 104.0 MHz ==="));

  Wire.begin();
  Wire.beginTransmission(0x10);
  byte err = Wire.endTransmission();
  Serial.print(F("I2C check @0x10: "));
  Serial.println(err == 0 ? F("ACK OK") : F("NO RESPONSE"));

  radio.setup();
  radio.setVolume(15);
  radio.setMute(false);
  radio.setMono(false);
  radio.setFrequency(FREQ);

  Serial.println(F("Tuned to 104.0 MHz"));
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    Serial.print(F("Freq: "));
    Serial.print(radio.getFrequency() / 100.0, 2);
    Serial.print(F(" MHz | RSSI: "));
    Serial.println(radio.getRssi());
  }
}
