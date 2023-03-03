#include "sensors.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "mqtt.h"

const uint8_t BMP280_I2C_ADDRESS = 0x76;
Adafruit_BMP280 bme;
const uint8_t ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometerAddress;

void Sensors::init() {
    if (!bme.begin(BMP280_I2C_ADDRESS)) {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    }

    sensors.begin();
    if (sensors.getAddress(thermometerAddress, 0)) {
        if (sensors.getResolution() != 12) {
            sensors.setResolution(thermometerAddress, 12);
        }
        sensors.requestTemperatures();
    } else {
        Serial.println("Unable to find address for Device 0 [ds18b20]");
    }
}

bool Sensors::getOutdoorTemperature(float &temperature) {
    temperature = sensors.getTempC(thermometerAddress);
    bool correctTemp =  (temperature >= -55 && temperature <= 125);  // -55 ... +125 °C.
    if(!correctTemp) {
        Serial.println("ERROR: Failed to read from ds18b20 sensor!");
        init();
        return false;
    }

    sensors.requestTemperatures();
    return true;
}

bool Sensors::getTemperature(float &temperature) {
    temperature = bme.readTemperature();
    return temperature >= -40 && temperature <= 85; // -40 ... +85 °C
}

bool Sensors::getPressure(float &pressure) {
    pressure = bme.readPressure() / 100.0F; // hPa;
    return pressure >= 300 && pressure <= 1100; // 300 ... 1100 hPa
}
