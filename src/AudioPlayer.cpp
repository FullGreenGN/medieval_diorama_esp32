#include "AudioPlayer.h"

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#ifndef DFPLAYER_RX_PIN
#define DFPLAYER_RX_PIN 16
#endif
#ifndef DFPLAYER_TX_PIN
#define DFPLAYER_TX_PIN 17
#endif
#ifndef DFPLAYER_BAUD
#define DFPLAYER_BAUD 9600
#endif

static DFRobotDFPlayerMini dfplayer;
static bool audioInitialized = false;
static volatile bool audioIsPlaying = false;
static Stream *dfSerial = nullptr;
static String lastInfo = "";

// add volume state
static int currentVolume = 20; // DFPlayer volume range 0..30

static void appendInfo(const char *msg) {
  lastInfo += msg;
  lastInfo += '\n';
}

static bool tryBegin(Stream &s) {
  bool ok = dfplayer.begin(s);
  if (ok) appendInfo("dfplayer.begin() succeeded on provided Stream");
  else appendInfo("dfplayer.begin() failed on provided Stream");
  return ok;
}

void setupAudioSystem() {
  // Initialize audio subsystem
  if (audioInit()) {
    Serial.println("[OK] Audio subsystem initialized");
  } else {
    Serial.println("[WARN] Audio subsystem failed to initialize");
  }
}


bool audioInit() {
  lastInfo = "";
  appendInfo("[INFO] Initializing DFPlayer...");

  // First try the configured Serial2 pins (RX then TX)
  Serial2.begin(DFPLAYER_BAUD, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  delay(200);
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "[INFO] Trying DFPlayer on Serial2 (RX=%d, TX=%d)", DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
    appendInfo(buf);
  }
  if (tryBegin(Serial2)) {
    appendInfo("[OK] DFPlayer Mini detected on Serial2");
    dfSerial = &Serial2;
    audioInitialized = true;
    dfplayer.volume(20);
    return true;
  }

  // Retry a few times
  for (int i = 0; i < 2 && !audioInitialized; ++i) {
    delay(200);
    appendInfo("[WARN] DFPlayer not responding; retrying Serial2...");
    if (tryBegin(Serial2)) {
      appendInfo("[OK] DFPlayer Mini detected on Serial2 after retry");
      dfSerial = &Serial2;
      audioInitialized = true;
      dfplayer.volume(20);
      return true;
    }
  }

  // Try swapped wiring (user may have RX/TX reversed)
  Serial2.end();
  Serial2.begin(DFPLAYER_BAUD, SERIAL_8N1, DFPLAYER_TX_PIN, DFPLAYER_RX_PIN);
  delay(200);
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "[INFO] Trying DFPlayer on Serial2 (swapped) (RX=%d, TX=%d)", DFPLAYER_TX_PIN, DFPLAYER_RX_PIN);
    appendInfo(buf);
  }
  if (tryBegin(Serial2)) {
    appendInfo("[OK] DFPlayer Mini detected on Serial2 (swapped pins)");
    dfSerial = &Serial2;
    audioInitialized = true;
    dfplayer.volume(20);
    return true;
  }

  // Try Serial1 using configured pins in case wiring uses UART1
  Serial2.end();
  Serial1.begin(DFPLAYER_BAUD, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  delay(200);
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "[INFO] Trying DFPlayer on Serial1 (RX=%d, TX=%d)", DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
    appendInfo(buf);
  }
  if (tryBegin(Serial1)) {
    appendInfo("[OK] DFPlayer Mini detected on Serial1");
    dfSerial = &Serial1;
    audioInitialized = true;
    dfplayer.volume(20);
    return true;
  }

  // Try swapped on Serial1
  Serial1.end();
  Serial1.begin(DFPLAYER_BAUD, SERIAL_8N1, DFPLAYER_TX_PIN, DFPLAYER_RX_PIN);
  delay(200);
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "[INFO] Trying DFPlayer on Serial1 (swapped) (RX=%d, TX=%d)", DFPLAYER_TX_PIN, DFPLAYER_RX_PIN);
    appendInfo(buf);
  }
  if (tryBegin(Serial1)) {
    appendInfo("[OK] DFPlayer Mini detected on Serial1 (swapped pins)");
    dfSerial = &Serial1;
    audioInitialized = true;
    dfplayer.volume(20);
    return true;
  }

  // As a last resort, try the USB Serial (only works if DFPlayer physically connected to those pins)
  Serial1.end();
  Serial2.end();
  appendInfo("[WARN] DFPlayer not detected on Serial2/Serial1; attempting USB Serial (Serial) fallback");
  delay(200);
  if (tryBegin(Serial)) {
    appendInfo("[OK] DFPlayer Mini detected on USB Serial");
    dfSerial = &Serial;
    audioInitialized = true;
    dfplayer.volume(20);
    return true;
  }

  appendInfo("[ERROR] DFPlayer Mini not found on Serial2/Serial1/Serial. Check wiring: DFPlayer TX->ESP RX, DFPlayer RX->ESP TX, common GND, correct voltage (5V for DFPlayer VCC). Also ensure pins used are available on your ESP32-S3 board.");
  audioInitialized = false;
  return false;
}

bool audioReinit() {
  // Try to reinitialize; clear previous state and call audioInit
  audioInitialized = false;
  audioIsPlaying = false;
  // End serials to ensure a clean start
  Serial1.end();
  Serial2.end();
  delay(50);
  bool ok = audioInit();
  if (ok) appendInfo("[INFO] audioReinit succeeded");
  else appendInfo("[ERROR] audioReinit failed");
  return ok;
}

String audioGetInfo() {
  return lastInfo;
}

// Accepts paths like "/001.mp3" or numeric strings "1"; maps to DFPlayer track index
static int parseIndexFromPath(const char *path) {
  if (!path) return -1;
  // Skip leading '/'
  const char *p = path;
  if (*p == '/') p++;

  // Extract leading digits
  int idx = 0;
  bool found = false;
  while (*p) {
    if (*p >= '0' && *p <= '9') {
      found = true;
      idx = idx * 10 + (*p - '0');
      p++;
    } else {
      // stop at first non-digit
      break;
    }
  }
  return found ? idx : -1;
}

bool playFile(const char *path) {
  if (!audioInitialized) {
    appendInfo("[WARN] audioInit() not called or failed - trying to init now");
    audioInit();
  }
  if (!audioInitialized) return false;

  int index = parseIndexFromPath(path);
  if (index > 0) {
    char buf[64];
    snprintf(buf, sizeof(buf), "[OK] DFPlayer play index %d", index);
    appendInfo(buf);
    dfplayer.play(index);
    audioIsPlaying = true;
    return true;
  }

  // If path not numeric, attempt to parse last numeric sequence
  // Not supported reliably by DFPlayer; return false
  appendInfo("[ERROR] DFPlayer playFile: could not parse track index from path");
  return false;
}

void stopPlayback() {
  if (!audioInitialized) return;
  dfplayer.stop();
  audioIsPlaying = false;
  appendInfo("[INFO] stopPlayback called");
}

bool isPlaying() {
  return audioIsPlaying;
}

bool audioSetVolume(int vol) {
  vol = constrain(vol, 0, 30);
  currentVolume = vol;

  if (!audioInitialized) {
    appendInfo("[WARN] audioSetVolume: DFPlayer not initialized, attempting init");
    if (!audioInit()) {
      appendInfo("[ERROR] audioSetVolume: init failed");
      return false;
    }
  }

  dfplayer.volume((uint8_t)vol);
  char buf[64];
  snprintf(buf, sizeof(buf), "[INFO] audioSetVolume: set to %d", vol);
  appendInfo(buf);
  return true;
}

int audioGetVolume() {
  return currentVolume;
}
