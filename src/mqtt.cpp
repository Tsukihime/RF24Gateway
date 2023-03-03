#include "mqtt.h"

#include <ESP8266WiFi.h>
#include <cstdio>
#include <PubSubClient.h>

#include "Config.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

String mqtt_domain;

void MQTT::init(const char *domain, uint16_t port) {
    mqtt_domain = domain;
    client.setServer(mqtt_domain.c_str(), port);
    //client.setCallback(MQTT::messageArrived);
    //String topic = (String)MQTT_PARENT_TOPIC + "/ota" ;
    //loop(); // neded before subscribe
    //client.subscribe("home/esp8266_sensor/ota");
}

void MQTT::messageArrived(char *p_topic, byte *p_payload, unsigned int p_length) {

}

void MQTT::publish(const char* topic, const uint8_t* payload, unsigned int plength) {
    String topic_str = Config::getMQTTTopicPrefix() + topic;
    Serial.println("MQTT Publish binary data: " + Config::getMQTTTopicPrefix() + topic_str);
    client.publish(topic_str.c_str(), payload, plength);
}

void MQTT::publish(const char *name, const char *value) {
    String topic = Config::getMQTTTopicPrefix() + name;
    Serial.println("MQTT Publish: " + topic + " => " + value);
    client.publish(topic.c_str(), value);
}

void MQTT::disconnect() {
    Serial.println("INFO: Closing the MQTT connection");
    client.disconnect();
}

void MQTT::reconnect() {
    while (!client.connected()) {
        Serial.println("INFO: Attempting MQTT connection...");
        if (client.connect(Config::getDeviceName().c_str(), "", "")) {
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
