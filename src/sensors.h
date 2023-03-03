#ifndef __SENSORS_H
#define __SENSORS_H

class Sensors {
public:
    static void init();
    static bool getOutdoorTemperature(float &temperature);
    static bool getTemperature(float &temperature);
    static bool getPressure(float &pressure);
};

#endif
