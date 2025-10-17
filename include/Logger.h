// File: `include/Logger.h`
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class Logger {
public:
    enum Level { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };

    // Initialize the singleton (call once after Serial.begin)
    static void init(const String &appName = "App", Level minLevel = INFO);

    // Access the singleton (init must be called first)
    static Logger& instance();

    // Optional deinit (rarely needed)
    static void deinit();

    void setLevel(Level minLevel);
    Level getLevel() const;

    void log(const String &message);
    void log(const String &message, unsigned long timestamp);
    void log(const String &message, unsigned long timestamp, unsigned long threadTime);
    void log(Level lvl, const String &message, unsigned long timestamp = 0, unsigned long threadTime = 0);

private:
    Logger(const String &appName, Level minLevel);
    ~Logger();

    String appName;
    Level minLevel;
    SemaphoreHandle_t mutex;

    void output(Level lvl, const String &message, unsigned long timestamp, unsigned long threadTime);
    const char* levelToString(Level lvl) const;

    static Logger* s_instance;
};

#define LOGD(msg) Logger::instance().log(Logger::DEBUG, String(msg))
#define LOGI(msg) Logger::instance().log(Logger::INFO,  String(msg))
#define LOGW(msg) Logger::instance().log(Logger::WARN,  String(msg))
#define LOGE(msg) Logger::instance().log(Logger::ERROR, String(msg))

#endif // LOGGER_H