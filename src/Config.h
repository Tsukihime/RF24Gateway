#ifndef CONFIG_H
#define CONFIG_H

#include "WString.h"
#include "Esp.h"

namespace Config {
    static const String DEVICE_NAME = "Meteostation";
    static const String MQTT_PARENT_TOPIC = "home";
    static const String NRF24_PREFIX = "nrf24_gate";

    static String getDeviceName() {
        return DEVICE_NAME; // + "_" +  String(ESP.getChipId(), HEX);
    }

    static String getMQTTTopicPrefix() {
        return MQTT_PARENT_TOPIC + "/" + getDeviceName() + "/";
    }

    static String getNRF24MQTTGatePrefix() {
        return NRF24_PREFIX + "/";
    }
};


#endif //CONFIG_H
