//
// Created by FullGreen on 13/10/2025.
//
#include "WifiRouter.h"

void setupWiFi(const char *ssid, const char *password) {
    try {
        WiFi.mode(WIFI_MODE_AP);
        WiFi.softAP(ssid, password);
        Serial.print("Access Point IP: ");
        Serial.println(WiFi.softAPIP());
    } catch (const std::exception &err) {
        Serial.print("[ERROR] Exception during setup: ");
        Serial.println(err.what());
    }
}

void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, attempting to reconnect...");
        WiFi.reconnect();
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(100);
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Reconnected to WiFi!");
        } else {
            Serial.println("Failed to reconnect to WiFi.");
        }
    }
}

String getWiFiStatus() {
    switch (WiFi.status()) {
        case WL_NO_SHIELD: return "No WiFi shield";
        case WL_IDLE_STATUS: return "Idle";
        case WL_NO_SSID_AVAIL: return "No SSID available";
        case WL_SCAN_COMPLETED: return "Scan completed";
        case WL_CONNECTED: return "Connected";
        case WL_CONNECT_FAILED: return "Connection failed";
        case WL_CONNECTION_LOST: return "Connection lost";
        case WL_DISCONNECTED: return "Disconnected";
        default: return "Unknown status";
    }
}

String getLocalIP() {
    return WiFi.localIP().toString();
}

String getSSID() {
    return WiFi.SSID();
}

