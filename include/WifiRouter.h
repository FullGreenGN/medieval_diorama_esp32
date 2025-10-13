#ifndef WIFIROUTER_H
#define WIFIROUTER_H

#include <Arduino.h>
#include <WiFi.h>

#define WIFI_SSID "marine-l-esp32"
#define WIFI_PASSWORD "password"

void setupWiFi(const char *ssid, const char *password);

void checkWiFiConnection();

String getWiFiStatus();

String getLocalIP();

String getSSID();

#endif // WIFIROUTER_H
