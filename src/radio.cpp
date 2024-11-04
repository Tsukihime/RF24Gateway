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
    radio.setPALevel(RF24_PA_MAX);    // уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
    radio.enableDynamicPayloads();
    radio.setDataRate(RF24_1MBPS);    // RF24_250KBPS, RF24_1MBPS или RF24_2MBPS
    radio.setCRCLength(RF24_CRC_8);   // размер контрольной суммы 8 bit или 16 bit RF24_CRC_DISABLED RF24_CRC_16
    radio.setChannel(channel);        // установка канала
    radio.setAutoAck(true);           // автоответ
    radio.openReadingPipe(1, address);// открыть трубу на приём
    radio.startListening();           // приём

    MQTT::publish("Radio", "start!!!");
}

namespace PACKET {
    const uint8_t SMALL_HEADER_SIZE = 2;

    const uint8_t START = 255;
    const uint8_t NEXT = 254;
    const uint8_t STOP = 253;

    union BigPacket {
        struct {
            uint16_t topic_length;
            uint16_t payload_length;
            uint8_t retained;
            uint8_t data[1019];
        };
        uint8_t raw[1024];
    };

    union SmallPacket {
        struct {
            uint8_t topic_length;
            uint8_t payload_length;
            uint8_t data[30];
        };
        uint8_t raw[32];
    };
}

bool parseSmallPayload(uint8_t *payload, uint8_t payloadSize) {
    auto pk = (PACKET::SmallPacket *) payload;
    if (payloadSize < PACKET::SMALL_HEADER_SIZE) return false;
    if (pk->topic_length == 0 || pk->payload_length == 0) return false;
    if ((pk->topic_length + pk->payload_length) > sizeof(PACKET::SmallPacket::data)) return false;

    String topic = Config::getRF24GatewayPrefix();
    for (int i = 0; i < pk->topic_length; i++) {
        topic += (char) pk->data[i];
    }

    MQTT::publish(topic.c_str(), &pk->data[pk->topic_length], pk->payload_length);
    return true;
}

bool parseBigPayload(uint8_t *payload, uint8_t payloadSize) {
    static PACKET::BigPacket pk;
    static uint16_t pos = 0;
    uint8_t packet_id = payload[0];
    uint8_t *block_data = &payload[1];
    uint8_t block_sz = payloadSize - 1;

    if (packet_id != PACKET::START
        && packet_id != PACKET::NEXT
        && packet_id != PACKET::STOP) {
        return false;
    }

    if (packet_id == PACKET::START) {
        pos = 0;
    }

    if ((pos + block_sz) > sizeof(PACKET::BigPacket)) {
        Serial.println("Buffer overflow!");
        return false;
    }

    memcpy(&pk.raw[pos], block_data, block_sz);
    pos += block_sz;

    if (packet_id == PACKET::STOP) {
        uint16_t length = pos;
        pos = 0;
        if ((pk.payload_length + pk.topic_length + 5) != length) {
            Serial.println("Parse error!");
            return false;
        }

        String topic;
        topic.reserve(pk.topic_length + 1);
        for (int i = 0; i < pk.topic_length; i++) {
            topic += (char)pk.data[i];
        }

        MQTT::publish(topic.c_str(), &pk.data[pk.topic_length], pk.payload_length, pk.retained);
    }
    return true;
}

void sendUnparsedPayload(uint8_t* payload, uint8_t payloadSize) {
    String topic = Config::getRF24GatewayPrefix() + "unparsed";
    MQTT::publish(topic.c_str(), payload, payloadSize);
}

void Radio::loop() {
    uint8_t data[32];
    if(!radioOk) return;
    while(radio.available()) {
        uint8_t payloadSize = radio.getDynamicPayloadSize();
        radio.read(data, payloadSize);
        if (!parseSmallPayload(data, payloadSize)) {
            if (!parseBigPayload(data, payloadSize)) {
                sendUnparsedPayload(data, payloadSize);
            }
        }
    }
}
