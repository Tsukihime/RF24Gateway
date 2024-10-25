#ifndef __MQTT_H
#define __MQTT_H

#include <Arduino.h>

class MQTT {
public:
    static void init(const char *domain, uint16_t port, const char* login, const char* password);

    static void loop();

    static void publish(const char *name, const char *value);

    static void publish(const char* topic, const uint8_t* payload, unsigned int plength, bool retained = false);

    static void disconnect();

private:
    static void reconnect();

    static void messageArrived(char *p_topic, byte *p_payload, unsigned int p_length);
};

#endif
