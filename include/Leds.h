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

// Start/update a non-blocking fire effect. Call frequently from loop().
void fireEffect();

#endif // LED_H
