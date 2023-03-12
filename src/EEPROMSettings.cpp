#include "EEPROMSettings.h"
#include <EEPROM.h>

const uint16_t MAGIC = 0xC0DE;

EEPROMSettings::EEPROMSettings() {
    EEPROM.begin(sizeof(_settings));
    EEPROM.get(0, _settings);
    EEPROM.end();
    if(_settings.magic != MAGIC) {
        _settings.magic = MAGIC;
        _settings.mqtt_port = 1883;
        strcpy(_settings.mqtt_server, "127.0.0.1");
        strcpy(_settings.mqtt_login, "");
        strcpy(_settings.mqtt_password, "");
    }
}

void EEPROMSettings::save() {
    EEPROM.begin(sizeof(_settings));
    EEPROM.put(0, _settings);
    EEPROM.commit();
    EEPROM.end();
}

char *EEPROMSettings::getMQTTServer() {
    return &_settings.mqtt_server[0];
}

void EEPROMSettings::setMQTTServer(const char *server) {
    strcpy(_settings.mqtt_server, server);
}

uint16_t EEPROMSettings::getMQTTPort() {
    return _settings.mqtt_port;
}

void EEPROMSettings::setMQTTPort(uint16_t port) {
    _settings.mqtt_port = port;
}

char *EEPROMSettings::getMQTTLogin() {
    return &_settings.mqtt_login[0];
}

void EEPROMSettings::setMQTTLogin(const char *login) {
    strcpy(_settings.mqtt_login, login);
}

char *EEPROMSettings::getMQTTPassword() {
    return &_settings.mqtt_password[0];
}

void EEPROMSettings::setMQTTPassword(const char *password) {
    strcpy(_settings.mqtt_password, password);
}
