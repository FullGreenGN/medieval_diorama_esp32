#include "Files.h"
#include "WebServer.h"
#include "AudioPlayer.h"
#include "WifiRouter.h"
#include "Leds.h"
#include "Smoke.h"

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println("[BOOT] ESP32 starting up...");
  setupLeds();
  setupSmoke();
  setupAudioSystem();
  setupWiFi(WIFI_SSID, WIFI_PASSWORD);

  setupFileSystem();
  setupWebServer();
}

void loop() {
  // Run non-blocking fire animation for LEDs
  fireEffect();
  // keep loop cooperative; fireEffect handles its own frame timing
  delay(1);
}
