#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <Ticker.h>

#include "sensors.h"
#include "mqtt.h"
#include "radio.h"
#include "EEPROMSettings.h"
#include "Config.h"

Ticker ticker;

void updateSensors();
void firstRun();

void setup() {
    Serial.begin(115200);
    Serial.println();

    WiFi.hostname(Config::getDeviceName());

    EEPROMSettings settings;

    char *old_mqtt_server = settings.getMQTTserver();
    uint16_t old_mqtt_port = settings.getMQTTPort();

    WiFiManager wifiManager;
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", old_mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", String(old_mqtt_port).c_str(), 5);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.setConfigPortalTimeout(180);
    if (!wifiManager.autoConnect(Config::getDeviceName().c_str())) {
        Serial.println("RESTART!");
        ESP.restart();
    }

    const char *new_mqtt_server = custom_mqtt_server.getValue();
    uint16_t new_mqtt_port = String(custom_mqtt_port.getValue()).toInt();

    bool mqtt_server_changed = strcmp(new_mqtt_server, old_mqtt_server) != 0;
    bool mqtt_port_changed = new_mqtt_port != old_mqtt_port;

    if (mqtt_server_changed || mqtt_port_changed) {
        settings.setMQTTPort(new_mqtt_port);
        settings.setMQTTServer(new_mqtt_server);
        settings.save();
        Serial.println("MQTT settings updated");
    }

    Sensors::init();
    MQTT::init(old_mqtt_server, old_mqtt_port);
    ArduinoOTA.begin();

    Sensors::init();
    Radio::init();

    ticker.once_scheduled(1, []() { // first run
        firstRun();
    });
}

void firstRun() {
    updateSensors();
    ticker.attach_scheduled(5 * 60, []() { // every 5 min
        updateSensors();
    });
}

void updateSensors() {
    float ds_temperature;
    if (Sensors::getOutdoorTemperature(ds_temperature)) {
        MQTT::publish("temperature_outdoor", String(ds_temperature).c_str());
    }

    float indoortemp;
    float pressure;
    bool t_ok = Sensors::getTemperature(indoortemp);
    bool p_ok = Sensors::getPressure(pressure);

    if (t_ok && p_ok) {
        MQTT::publish("temperature", String(indoortemp).c_str());
        MQTT::publish("pressure", String(pressure).c_str());
    } else {
        Serial.println("ERROR: Failed to read from BMP280 sensor!");
        Sensors::init();
    }
}

void loop() {
    ArduinoOTA.handle();
    MQTT::loop();
    Radio::loop();
}
