#include "mqtt.h"

#include <ESP8266WiFi.h>
#include <cstdio>
#include <PubSubClient.h>

#include "Config.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

String mqttDomain;
String mqttLogin;
String mqttPassword;

void MQTT::init(const char* domain, uint16_t port, const char* login, const char* password) {
    mqttDomain = domain;
    mqttLogin = login;
    mqttPassword = password;
    client.setServer(mqttDomain.c_str(), port);
    //client.setCallback(MQTT::messageArrived);
    //String topic = (String)MQTT_PARENT_TOPIC + "/ota" ;
    //loop(); // neded before subscribe
    //client.subscribe("home/esp8266_sensor/ota");
}

void MQTT::messageArrived(char *p_topic, byte *p_payload, unsigned int p_length) {

}

void MQTT::publish(const char* topic, const uint8_t* payload, unsigned int plength) {
    Serial.println("MQTT Publish binary data: " + String(topic));
    client.publish(topic, payload, plength);
}

void MQTT::publish(const char *name, const char *value) {
    Serial.println("MQTT Publish: " + String(name) + " => " + value);
    client.publish(name, value);
}

void MQTT::disconnect() {
    Serial.println("INFO: Closing the MQTT connection");
    client.disconnect();
}

void MQTT::reconnect() {
    while (!client.connected()) {
        Serial.println("INFO: Attempting MQTT connection...");
        String mqttClientId = Config::getDeviceName() + "-" + Config::getDeviceId();
        if (client.connect(mqttClientId.c_str(), mqttLogin.c_str(), mqttPassword.c_str())) {
            Serial.println("INFO: connected");
        } else {
            Serial.print("ERROR: failed, rc=");
            Serial.print(client.state());
            Serial.println("DEBUG: try again in 3 seconds");
            delay(3000);
        }
    }
}

void MQTT::loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}
