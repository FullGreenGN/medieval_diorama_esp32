//
// Created by FullGreen on 13/10/2025.
//

#ifndef PWM_H
#define PWM_H

#include <Arduino.h>

#define MOULIN_PWM_PIN 12

void setupPwm();
void setPwm(int brightness);
int getPwm();
void turnOffPwm();
void turnOnPwm();
void tryPwm();

#endif // PWM_H
