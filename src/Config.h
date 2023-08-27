#ifndef CONFIG_H
#define CONFIG_H

#include "WString.h"
#include "Esp.h"

namespace Config {

    static String getDeviceName() {
        return "Meteostation";
    }

    static String getDeviceId() {
        return String(ESP.getChipId(), HEX);
    }

    static String getMQTTMeteostationPrefix() {
        return "home/Meteostation/";
    }

    static String getNRF24MQTTGatewayPrefix() {
        return "home/NRF24MQTTGateway/";
    }
};

#endif //CONFIG_H
