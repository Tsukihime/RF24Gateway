#include <ArduinoOTA.h>
#include <Ticker.h>

#include "MQTT.h"
#include "radio.h"
#include "Config.h"
#include "Log.h"
#include <ApSettingsManager.h>

Ticker ticker;
Ticker ticker_log;
ApSettingsManager apManager;

void setup() {
    Serial.begin(115200);
    Serial.println();

    String customParameters = R"([{
        "title": "Настройки MQTT",
        "rows": [
            {"name": "mqttServer", "type": "text", "title": "Сервер:", "value": "192.168.0.1"},
            {"name": "mqttPort", "type": "number", "title": "Порт:", "value": "1883"},
            {"name": "mqttLogin", "type": "text", "title": "Логин:", "value": ""},
            {"name": "mqttPassword", "type": "password", "title": "Пароль:", "value": ""}
        ]
    }])";

    apManager.begin(Config::getDeviceName());
    apManager.setCustomParameters(customParameters);

    MQTT::init(
        apManager.getParameter("mqttServer").c_str(), 
        apManager.getParameter("mqttPort").toInt(), 
        apManager.getParameter("mqttLogin").c_str(), 
        apManager.getParameter("mqttPassword").c_str()
    );

    ArduinoOTA.begin();
    Radio::init();

    ticker.once_scheduled(1, []() {
        String topic = Config::getRF24GatewayPrefix() + "log";
        MQTT::publish(topic.c_str(), "Start!");
    });

    ticker_log.attach(5, []() {
        String log = Log::getLogJson();
        apManager.setLogJson(log);
    });
}

void loop() {
    apManager.handle();
    ArduinoOTA.handle();
    MQTT::loop();
    Radio::loop();
}
