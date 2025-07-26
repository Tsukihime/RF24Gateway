#ifndef LOG_H
#define LOG_H

#include <Arduino.h>

const size_t LOG_LENGTH = 7;

class Log {
private:
    struct LogEntry {
        String header;
        String text;
        unsigned long time;
    };

    static LogEntry logs[LOG_LENGTH];
    static unsigned int logCount;
    static unsigned int logIndex;

public:
    static void add(String header, String text);
    static String getLogJson();
};

#endif
