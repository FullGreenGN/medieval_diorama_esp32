#include "Files.h"
#include "WebServer.h"
#include "AudioPlayer.h"
#include "WifiRouter.h"
#include "Leds.h"
#include "Logger.h"
#include "Smoke.h"
#include "Pwm.h"

void setup() {
  Serial.begin(115200);
  Logger::init("DIORAMA", Logger::DEBUG);
  delay(3000);
  LOGI("ESP 32 is booting");

  setupLeds();
  setupPwm();
  setupSmoke();
  setupAudioSystem();
  setupWiFi(WIFI_SSID, WIFI_PASSWORD);

  setupFileSystem();
  setupWebServer();
}

void loop() {
  // Run non-blocking fire animation for LEDs only when requested
  if (isFireEffectActive()) {
    fireEffect();
  }
  // keep loop cooperative; fireEffect handles its own frame timing
  delay(1);
}
