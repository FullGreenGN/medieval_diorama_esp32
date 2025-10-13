#include <WiFi.h>
#include <LittleFS.h>
#include "WebServer.h"
#include "AudioPlayer.h"

const char *ssid = "jean-marie-l-esp32";
const char *password = "password";

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("[BOOT] ESP32 starting up...");

  try {
    WiFi.mode(WIFI_MODE_AP);
    Serial.print("Station IP: ");
    Serial.println(WiFi.localIP());
    WiFi.softAP(ssid, password);

    Serial.println("Wi-Fi Access Point started!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  } catch (const std::exception &err) {
    Serial.print("[ERROR] Exception during setup: ");
    Serial.println(err.what());
    return;
  }

  // Mount filesystem (React build)
  if (!LittleFS.begin(true)) {
    Serial.println("[ERROR] LittleFS mount failed!");
    return;
  }
  Serial.println("[OK] LittleFS mounted successfully");

  // Initialize audio subsystem
  if (audioInit()) {
    Serial.println("[OK] Audio subsystem initialized");
  } else {
    Serial.println("[WARN] Audio subsystem failed to initialize");
  }

  // Initialize Async Web Server
  setupWebServer();
}

void loop() {
  // AsyncWebServer runs in background — nothing needed here
}
