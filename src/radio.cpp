#include "radio.h"
#include <RF24.h>
#include "MQTT.h"
#include "RF24MQTTProto.h"
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
    radio.setPALevel(RF24_PA_HIGH, true); // уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
    radio.enableDynamicPayloads();
    radio.setDataRate(RF24_250KBPS);  // RF24_250KBPS, RF24_1MBPS или RF24_2MBPS
    radio.setCRCLength(RF24_CRC_16);  // размер контрольной суммы 8 bit или 16 bit RF24_CRC_DISABLED RF24_CRC_16
    radio.setChannel(channel);        // установка канала
    radio.setAutoAck(true);           // автоответ
    radio.openReadingPipe(1, address);// открыть трубу на приём
    radio.startListening();           // приём
}

bool parsePacket(const Packet& pk, uint8_t payloadSize) {
    static uint16_t topic_length;
    static uint16_t payload_length;
    static bool retained;
    static uint8_t buffer[1024];
    static uint16_t pos = 0;

    if (payloadSize == 0) {
        return false;
    }

    const uint8_t *data;
    uint8_t block_sz;

    if (pk.header.marker == Marker::START || pk.header.marker == Marker::START_STOP) {
        pos = 0;
        topic_length = pk.first.topic_length;
        payload_length = pk.first.payload_length;
        retained = pk.header.retained;
        data = pk.first.data;
        block_sz = payloadSize - offsetof(Packet, first.data);
    } else {
        data = pk.next.data;
        block_sz = payloadSize - offsetof(Packet, next.data);
    }

    if ((pos + block_sz) > sizeof(buffer)) {
        Serial.println("Buffer overflow!");
        return false;
    }

    memcpy(&buffer[pos], data, block_sz);
    pos += block_sz;

    if (pk.header.marker == Marker::STOP || pk.header.marker == Marker::START_STOP) {
        uint16_t length = pos;
        pos = 0;

        if ((payload_length + topic_length) != length) {
            Serial.println("Parse error!");
            return false;
        }

        String topic;
        topic.reserve(topic_length + 1);
        for (int i = 0; i < topic_length; i++) {
            topic += (char)buffer[i];
        }

        MQTT::publish(topic.c_str(), &buffer[topic_length], payload_length, retained);
    }

    return true;
}

void sendUnparsedPacket(const Packet& pk, uint8_t payloadSize) {
    String topic = Config::getRF24GatewayPrefix() + "unparsed";
    MQTT::publish(topic.c_str(), reinterpret_cast<const uint8_t*>(&pk), payloadSize);
}

void Radio::loop() {
    Packet packet;
    if (!radioOk) return;

    while (radio.available()) {
        uint8_t payloadSize = radio.getDynamicPayloadSize();
        radio.read(&packet, payloadSize);

        if (!parsePacket(packet, payloadSize)) {
            sendUnparsedPacket(packet, payloadSize);
        }
    }
}
