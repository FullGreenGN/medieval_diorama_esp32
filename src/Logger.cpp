// File: `src/Logger.cpp`
#include "Logger.h"

Logger* Logger::s_instance = nullptr;

void Logger::init(const String &appName, Level minLevel) {
    if (s_instance == nullptr) {
        s_instance = new Logger(appName, minLevel);
    } else {
        s_instance->setLevel(minLevel);
    }
}

Logger& Logger::instance() {
    if (s_instance == nullptr) {
        // Defensive: if not initialized, create a default instance
        s_instance = new Logger("App", INFO);
    }
    return *s_instance;
}

void Logger::deinit() {
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

Logger::Logger(const String &appName, Level minLevel)
    : appName(appName), minLevel(minLevel)
{
    mutex = xSemaphoreCreateMutex();
}

Logger::~Logger()
{
    if (mutex) {
        vSemaphoreDelete(mutex);
        mutex = nullptr;
    }
}

void Logger::setLevel(Level level) { minLevel = level; }
Logger::Level Logger::getLevel() const { return minLevel; }

void Logger::log(const String &message) {
    log(INFO, message, millis(), 0);
}
void Logger::log(const String &message, unsigned long timestamp) {
    log(INFO, message, timestamp, 0);
}
void Logger::log(const String &message, unsigned long timestamp, unsigned long threadTime) {
    log(INFO, message, timestamp, threadTime);
}

void Logger::log(Level lvl, const String &message, unsigned long timestamp, unsigned long threadTime) {
    if (lvl < minLevel) return;
    output(lvl, message, timestamp == 0 ? millis() : timestamp, threadTime);
}

const char* Logger::levelToString(Level lvl) const {
    switch (lvl) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        default:    return "UNK";
    }
}

void Logger::output(Level lvl, const String &message, unsigned long timestamp, unsigned long threadTime) {
    if (mutex && xSemaphoreTake(mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        Serial.print('[');
        Serial.print(appName);
        Serial.print("][");
        Serial.print(levelToString(lvl));
        Serial.print("] ");

        Serial.print('(');
        Serial.print(timestamp);
        Serial.print("ms)");
        if (threadTime != 0) {
            Serial.print(" [t:");
            Serial.print(threadTime);
            Serial.print("ms]");
        }

        Serial.print(" ");
        Serial.println(message);

        xSemaphoreGive(mutex);
    } else {
        // Fallback if mutex not available
        Serial.print('[');
        Serial.print(appName);
        Serial.print("][");
        Serial.print(levelToString(lvl));
        Serial.print("] ");
        Serial.print('(');
        Serial.print(timestamp);
        Serial.print("ms)");
        if (threadTime != 0) {
            Serial.print(" [t:");
            Serial.print(threadTime);
            Serial.print("ms]");
        }
        Serial.print(" ");
        Serial.println(message);
    }
}