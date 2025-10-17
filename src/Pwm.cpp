// filepath: /Users/fullgreen/Documents/cours/stein/untitled/src/Pwm.cpp
#include "Pwm.h"
#include "Logger.h"

// Use LEDC on ESP32 for PWM control
static int g_pwmValue = 0;
static const int PWM_CHANNEL = 0;
static const int PWM_FREQ = 5000; // 5kHz
static const int PWM_RESOLUTION = 8; // 8-bit -> 0..255

void setupPwm() {
    LOGD("Init PWM");
    // configure LEDC channel
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOULIN_PWM_PIN, PWM_CHANNEL);
    // default off
    turnOffPwm();
}

void setPwm(int brightness) {
    brightness = constrain(brightness, 0, 255);
    g_pwmValue = brightness;
    ledcWrite(PWM_CHANNEL, brightness);
}

int getPwm() {
    return g_pwmValue;
}

void turnOffPwm() {
    setPwm(0);
}

void turnOnPwm() {
    setPwm(255);
}

void tryPwm() {
    // Simple test pulse
    setPwm(255);
    delay(300);
    setPwm(0);
}

