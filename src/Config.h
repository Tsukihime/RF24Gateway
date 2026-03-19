#ifndef CONFIG_H
#define CONFIG_H

namespace Config {

inline uint32_t getChipId() {
    return ESP.getChipId();
}

inline const char* getBaseName() {
    return "MQTTRFGateway";
}

inline String getDeviceId() {
    char buf[9];
    snprintf(buf, sizeof(buf), "%06x", getChipId());
    return String(buf);
}

inline String getDeviceName() {
    return String(getBaseName()) + "_" + getDeviceId();
}

inline String getMqttPrefix() {
    static String prefix;
    if (prefix.isEmpty()) {
        prefix.reserve(48);
        prefix = "home/";
        prefix += getDeviceName();
        prefix += "/";
    }
    return prefix;
}

};

#endif //CONFIG_H
