#include "radio.h"
#include <RF24.h>
#include "MQTT.h"
#include "Config.h"
#include "lib/MQTTGateway.hpp"

MQTTGatewayReceiver<1024> receiver(
    [](const char* topic, uint8_t* payload, uint16_t payload_length, bool retained) {
        MQTT::publish(topic, payload, payload_length, retained);        
    }
);

RF24 radio(16, 15);
const uint8_t address[6] = { "NrfMQ" };
const uint8_t channel = 0x6f;
bool radioOk = false;

void Radio::init() {
    if (!radio.begin()) {
        Serial.println(F("NRF24 radio hardware is not responding!!"));
        return;
    }
    radioOk = true;
    Serial.println(F("NRF24 radio initialized!"));
 // ------------------------------------------
    radio.setPALevel(RF24_PA_HIGH, true); // уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
    radio.enableDynamicPayloads();
    radio.setDataRate(RF24_250KBPS);  // RF24_250KBPS, RF24_1MBPS или RF24_2MBPS
    radio.setCRCLength(RF24_CRC_16);  // размер контрольной суммы 8 bit или 16 bit RF24_CRC_DISABLED RF24_CRC_16
    radio.setChannel(channel);        // установка канала
    radio.setAutoAck(true);           // автоответ
    radio.openReadingPipe(1, address);// открыть трубу на приём
    radio.startListening();           // приём
}

void sendUnparsedPacket(const uint8_t* packet, uint8_t payloadSize) {
    String topic = Config::getMqttPrefix() + "unparsed";
    MQTT::publish(topic.c_str(), packet, payloadSize);
}

void Radio::loop() {
    uint8_t data[32];
    if (!radioOk) return;

    while (radio.available()) {
        uint8_t payloadSize = radio.getDynamicPayloadSize();
        radio.read(data, payloadSize);

        if (!receiver.parsePacket(data, payloadSize)) {
            sendUnparsedPacket(data, payloadSize);
        }
    }
}
