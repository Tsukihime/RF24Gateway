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

    char* oldMqttServer = settings.getMQTTServer();
    uint16_t oldMqttPort = settings.getMQTTPort();
    char* oldMqttLogin = settings.getMQTTLogin();
    char* oldMqttPassword = settings.getMQTTPassword();

    WiFiManager wifiManager;
    WiFiManagerParameter mqttServerParameter("Server", "mqtt server", oldMqttServer, 255);
    WiFiManagerParameter mqttPortParameter("Port", "mqtt port", String(oldMqttPort).c_str(), 5);
    WiFiManagerParameter mqttLoginParameter("Login", "mqtt login", oldMqttLogin, 255);
    WiFiManagerParameter mqttPasswordParameter("Password", "mqtt password", oldMqttPassword, 255);

    wifiManager.addParameter(&mqttServerParameter);
    wifiManager.addParameter(&mqttPortParameter);
    wifiManager.addParameter(&mqttLoginParameter);
    wifiManager.addParameter(&mqttPasswordParameter);

    wifiManager.setConfigPortalTimeout(180);
    if (!wifiManager.autoConnect(Config::getDeviceName().c_str())) {
        Serial.println("RESTART!");
        ESP.restart();
    }

    const char *newMqttServer = mqttServerParameter.getValue();
    uint16_t newMqttPort = String(mqttPortParameter.getValue()).toInt();
    const char *newMqttLogin = mqttLoginParameter.getValue();
    const char *newMqttPassword = mqttPasswordParameter.getValue();

    bool mqttServerChanged = strcmp(newMqttServer, oldMqttServer) != 0;
    bool mqttPortChanged = newMqttPort != oldMqttPort;
    bool mqttLoginChanged = strcmp(newMqttLogin, oldMqttLogin) != 0;
    bool mqttPasswordChanged = strcmp(newMqttPassword, oldMqttPassword) != 0;
    bool settingsChanged = mqttServerChanged ||
                           mqttPortChanged ||
                           mqttLoginChanged ||
                           mqttPasswordChanged;

    if (settingsChanged) {
        settings.setMQTTPort(newMqttPort);
        settings.setMQTTServer(newMqttServer);
        settings.setMQTTLogin(newMqttLogin);
        settings.setMQTTPassword(newMqttPassword);
        settings.save();
        Serial.println("MQTT settings updated");
    }

    Sensors::init();
    MQTT::init(oldMqttServer, oldMqttPort, oldMqttLogin, oldMqttPassword);
    ArduinoOTA.begin();
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
    float indoortemp;
    float pressure;
    bool t_ok = Sensors::getTemperature(indoortemp);
    bool p_ok = Sensors::getPressure(pressure);

    String temperature_topic = Config::getMQTTMeteostationPrefix() + "temperature";
    String pressure_topic = Config::getMQTTMeteostationPrefix() + "pressure";

    if (t_ok && p_ok) {
        MQTT::publish(temperature_topic.c_str(), String(indoortemp).c_str());
        MQTT::publish(pressure_topic.c_str(), String(pressure).c_str());
    } else {
        Serial.println("ERROR: Failed to read from BMP280 sensor!");
        MQTT::publish("ERROR", "Failed to read from BMP280 sensor!");
        Sensors::init();
    }
}

void loop() {
    ArduinoOTA.handle();
    MQTT::loop();
    Radio::loop();
}
