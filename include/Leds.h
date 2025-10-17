#ifndef LED_H
#define LED_H

#include <Arduino.h>

#define LED_ONE 15
#define LED_TWO 7
#define LED_THREE 6

void setupLeds();
void tryLeds();
void setLed(int ledPin, int brightness);
bool getLed(int ledPin);
void turnOffLeds();
void turnOnLeds();

// Start/update a non-blocking fire effect. Call fireEffect() frequently from loop()
// when the effect is active. Use startFireEffect()/stopFireEffect() to control it from
// other parts of the program (for example an API call).
void fireEffect();

// Control the fire effect from external code (e.g. web API)
void startFireEffect();
void stopFireEffect();
bool isFireEffectActive();

#endif // LED_H
