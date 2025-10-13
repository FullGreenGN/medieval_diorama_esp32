#ifndef SMOKE_H
#define SMOKE_H

#include <Arduino.h>

#define SMOKE_1 13
#define SMOKE_2 5

void setupSmoke();
void trySmoke();
void setSmoke(int ledPin, int brightness);
bool getSmoke(int ledPin);
void turnOnSmoke();
void turnOffSmoke();

#endif // SMOKE_H
