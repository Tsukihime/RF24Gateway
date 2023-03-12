#include "radio.h"
#include <RF24.h>
#include "mqtt.h"
#include "Config.h"

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
    radio.enableDynamicPayloads();
    radio.setDataRate(RF24_1MBPS);    // RF24_250KBPS, RF24_1MBPS или RF24_2MBPS
    radio.setCRCLength(RF24_CRC_8);   // размер контрольной суммы 8 bit или 16 bit RF24_CRC_DISABLED RF24_CRC_16
    radio.setChannel(channel);        // установка канала
    radio.setAutoAck(false);          // автоответ
    radio.openReadingPipe(1, address);// открыть трубу на приём
    radio.startListening();           // приём

    MQTT::publish("Radio", "start!!!");
}

bool parsePayload(uint8_t* payload, uint8_t payloadSize) {
    const uint8_t HEADER_SIZE = 2;
    const uint8_t MAX_PAYLOAD_SIZE = 32;
    if(payloadSize < HEADER_SIZE) return false;
    uint8_t tiopicSize = payload[0];
    uint8_t messageSize = payload[1];
    if(tiopicSize == 0 || messageSize == 0) return false;
    if((tiopicSize + messageSize + HEADER_SIZE) > MAX_PAYLOAD_SIZE) return false;

    String topic = Config::getNRF24MQTTGatePrefix();
    for(int i = HEADER_SIZE; i < tiopicSize + HEADER_SIZE; i++) {
        topic += (char)payload[i];
    }

    MQTT::publish(topic.c_str(), &payload[tiopicSize + HEADER_SIZE], messageSize);
    return true;
}

void sendUnparsed(uint8_t* payload, uint8_t payloadSize) {
    String str;
    for(int i = 0; i < payloadSize; i++) {
        String hex = String(payload[i], HEX);
        if(hex.length() == 1) {
            hex = '0' + hex;
        }
        str += hex;
    }
    String topic = Config::getNRF24MQTTGatePrefix() + "unparsed";
    MQTT::publish(topic.c_str(), str.c_str());
}

void Radio::loop() {
    uint8_t data[32];
    if(!radioOk) return;
    if (radio.available()) {
        uint8_t payloadSize = radio.getDynamicPayloadSize();
        radio.read(data, payloadSize);
        if(!parsePayload(data, payloadSize)) {
            sendUnparsed(data, payloadSize);
        }
    }
}
