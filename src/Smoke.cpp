#include "Smoke.h"

void setupSmoke() {
    pinMode(SMOKE_1, OUTPUT);
    pinMode(SMOKE_2, OUTPUT);
    trySmoke();
    turnOffSmoke();
}

void setSmoke(int smokePin, int state) {
    digitalWrite(smokePin, state);
}

bool getSmoke(int ledPin) {
    return digitalRead(ledPin);
}


void turnOnSmoke() {
    digitalWrite(SMOKE_1, HIGH);
    digitalWrite(SMOKE_2, HIGH);
}

void turnOffSmoke() {
    digitalWrite(SMOKE_1, LOW);
    digitalWrite(SMOKE_2, LOW);
}

void trySmoke() {
    turnOnSmoke();
    delay(1000);
}