#ifndef ESP8266_METEOSTATION_EEPROMSETTINGS_H
#define ESP8266_METEOSTATION_EEPROMSETTINGS_H

#include <PubSubClient.h>

class EEPROMSettings {
private:
    struct EEPROM_SETTINGS {
        uint16_t magic;
        char mqtt_server[255];
        char mqtt_login[255];
        char mqtt_password[255];
        uint16_t mqtt_port;
    } _settings;

public:
    EEPROMSettings();

    void save();

    char *getMQTTServer();

    void setMQTTServer(const char *server);

    uint16_t getMQTTPort();

    void setMQTTPort(uint16_t port);

    char *getMQTTLogin();

    void setMQTTLogin(const char *login);

    char *getMQTTPassword();

    void setMQTTPassword(const char *password);
};

#endif //ESP8266_METEOSTATION_EEPROMSETTINGS_H
