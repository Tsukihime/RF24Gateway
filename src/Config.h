#ifndef CONFIG_H
#define CONFIG_H

#include "WString.h"
#include "Esp.h"

namespace Config {

    static String getDeviceName() {
        return "RF24Gateway";
    }

    static String getDeviceId() {
        return String(ESP.getChipId(), HEX);
    }

    static String getRF24GatewayPrefix() {
        return "home/RF24Gateway/";
    }
};

#endif //CONFIG_H
