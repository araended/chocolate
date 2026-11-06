#include <Arduino.h>
#include "sensor_handler.h"
Adafruit_ADS1X15 ads;

float depth_constant = 3; //3 cm / mv

void init_sensor(){
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115; check I2C wiring and address.");
    while (true) {
      delay(1000);
    }
  }
}

float read_depth_sensor(){
  int16_t raw = ads.readADC_SingleEnded(ADS1115_PRESSURE_CH);
  float voltage = ads.computeVolts(raw);
  float depth = voltage * depth_constant;
  return voltage;
}
