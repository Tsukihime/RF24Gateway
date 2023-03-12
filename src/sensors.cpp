#include "sensors.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

const uint8_t BMP280_I2C_ADDRESS = 0x76;
Adafruit_BMP280 bme;

void Sensors::init() {
    if (!bme.begin(BMP280_I2C_ADDRESS)) {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    }
}

bool Sensors::getTemperature(float &temperature) {
    temperature = bme.readTemperature();
    return temperature >= -40 && temperature <= 85; // -40 ... +85 °C
}

bool Sensors::getPressure(float &pressure) {
    pressure = bme.readPressure() / 100.0F; // hPa;
    return pressure >= 300 && pressure <= 1100; // 300 ... 1100 hPa
}
