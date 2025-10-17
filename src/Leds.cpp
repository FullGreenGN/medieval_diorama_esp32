#include "Leds.h"

#include "Logger.h"

void setupLeds() {
    LOGD("Init leds");
    pinMode(LED_ONE, OUTPUT);
    pinMode(LED_TWO, OUTPUT);
    pinMode(LED_THREE, OUTPUT);
    // seed PRNG for pseudo-random fire effect
    randomSeed(micros());
    tryLeds();
    turnOffLeds();
}

void setLed(int ledPin, int brightness) {
    analogWrite(ledPin, brightness);
}

bool getLed(int ledPin) {
    return analogRead(ledPin);
}

void turnOffLeds() {
    analogWrite(LED_ONE, 0);
    analogWrite(LED_TWO, 0);
    analogWrite(LED_THREE, 0);
}

void turnOnLeds() {
    analogWrite(LED_ONE, 255);
    analogWrite(LED_TWO, 255);
    analogWrite(LED_THREE, 255);
}

void tryLeds() {
    LOGD("Testing Leds");
    turnOnLeds();
    delay(1000);
    LOGD("Leds OK");
}

// Internal state for the fire effect
static volatile bool g_fireActive = false;

// Move heat and timing to file-scope so we can reset them when starting
static uint8_t g_heat[3] = {0, 0, 0};
static unsigned long g_lastFrame = 0;

// Provide external control for the fire effect
void startFireEffect() {
    // reset internal heat array by restarting the effect
    g_heat[0] = g_heat[1] = g_heat[2] = 0;
    g_lastFrame = millis();
    g_fireActive = true;
    LOGI("Fire effect started");
}

void stopFireEffect() {
    g_fireActive = false;
    // ensure LEDs are turned off when stopping the effect
    turnOffLeds();
    LOGI("Fire effect stopped");
}

bool isFireEffectActive() {
    return g_fireActive;
}

// Non-blocking fire effect inspired by simple heat-simulation.
// Call fireEffect() frequently from loop() to animate when the
// effect is active.
void fireEffect() {
    if (!g_fireActive) return; // no-op when effect is not active

    constexpr uint8_t sparking = 120; // chance of new spark (0..255)
    constexpr uint16_t frameDelay = 50; // ms between updates
    // use file-scope g_heat[] and g_lastFrame

    unsigned long now = millis();
    if (now - g_lastFrame < frameDelay) return;
    g_lastFrame = now;

    // Step 1. Cool down every cell a little
    for (int i = 0; i < 3; i++) {
        constexpr uint8_t cooling = 55;
        int cooldown = random(0, ((cooling * 10) / 3) + 2);
        if (cooldown >= g_heat[i]) g_heat[i] = 0;
        else g_heat[i] = g_heat[i] - cooldown;
    }

    // Step 2. Heat diffusion upward (towards higher indices)
    // We'll mix the lower cells into the ones above to simulate rising heat.
    uint8_t newHeat2 = static_cast<uint8_t>((g_heat[1] + g_heat[0] + g_heat[0]) / 3);
    uint8_t newHeat1 = static_cast<uint8_t>((g_heat[0] + g_heat[0] + g_heat[1]) / 3);
    g_heat[2] = newHeat2;
    g_heat[1] = newHeat1;
    // g_heat[0] will get sparks below

    // Step 3. Randomly ignite new 'sparks' near the bottom
    if (random(255) < sparking) {
        int y = 0; // create spark at bottom
        int added = random(160, 255);
        int v = g_heat[y] + added;
        g_heat[y] = (v > 255) ? 255 : v;
    }

    // Step 4. Map heat to LED brightness (non-linear for nicer look)
    for (int i = 0; i < 3; i++) {
        // scale heat (0..255) to brightness 0..255 with a quadratic curve
        int h = g_heat[i];
        int brightness = (h * h) / 255; // simple gamma-like curve
        if (brightness > 255) brightness = 255;

        int pin = LED_ONE;
        if (i == 0) pin = LED_ONE;
        else if (i == 1) pin = LED_TWO;
        else if (i == 2) pin = LED_THREE;

        analogWrite(pin, brightness);
    }
}
