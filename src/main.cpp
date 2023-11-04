#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <Ticker.h>

#include "mqtt.h"
#include "radio.h"
#include "EEPROMSettings.h"
#include "Config.h"

Ticker ticker;

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

    MQTT::init(oldMqttServer, oldMqttPort, oldMqttLogin, oldMqttPassword);
    ArduinoOTA.begin();
    Radio::init();

    ticker.once_scheduled(1, []() {
        String topic = Config::getRF24GatewayPrefix() + "log";
        MQTT::publish(topic.c_str(), "Start!");
    });
}

void loop() {
    ArduinoOTA.handle();
    MQTT::loop();
    Radio::loop();
}
