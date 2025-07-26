#include "Log.h"

#include <ArduinoJson.h>

Log::LogEntry Log::logs[LOG_LENGTH];
unsigned int Log::logCount = 0;
unsigned int Log::logIndex = 0;

void Log::add(String header, String text) {
    logs[logIndex].header = header;
    logs[logIndex].text = text;
    logs[logIndex].time = millis();

    logIndex = (logIndex + 1) % LOG_LENGTH;
    if (logCount < LOG_LENGTH) {
        logCount++;
    }
}

String Log::getLogJson() {
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();

    int startIndex = logCount < LOG_LENGTH ? 0 : logIndex;
    for (unsigned int i = 0; i < logCount; i++) {
        int index = (startIndex + i) % LOG_LENGTH;
        String key = String(logs[index].time);
        root[key] = logs[index].header + " => " + logs[index].text;
    }

    String json;
    serializeJson(root, json);
    return json;
}