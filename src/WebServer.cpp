#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "../include/WebServer.h"
#include "../include/AudioPlayer.h"

// Define hardware pins
#define LED_R 9
#define LED_J 10
#define LED_V 11
#define DRAGON 12
#define NECRO 13

AsyncWebServer server(80);

void setupWebServer() {
  // Init pins
  pinMode(LED_R, OUTPUT);
  pinMode(LED_J, OUTPUT);
  pinMode(LED_V, OUTPUT);
  pinMode(DRAGON, OUTPUT);
  pinMode(NECRO, OUTPUT);

  // Default all off
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_J, LOW);
  digitalWrite(LED_V, LOW);
  digitalWrite(DRAGON, LOW);
  digitalWrite(NECRO, LOW);

  // ✅ React Router fallback
  server.onNotFound([](AsyncWebServerRequest *request) {
    String path = request->url();
    if (!path.startsWith("/api")) {
      // Return index.html for React routes
      request->send(LittleFS, "/index.html", "text/html");
    } else {
      request->send(404, "application/json", R"({"error":"API route not found"})");
    }
  });

  // ✅ REST API endpoints

  // Status endpoint
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *req) {
    StaticJsonDocument<200> json;
    json["status"] = "ok";
    json["uptime_ms"] = millis();

    String out;
    serializeJson(json, out);
    req->send(200, "application/json", out);
  });

  // LED control
  server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!req->hasParam("color") || !req->hasParam("state")) {
      req->send(400, "application/json", R"({"error":"Missing 'color' or 'state' param"})");
      return;
    }

    String color = req->getParam("color")->value();
    String state = req->getParam("state")->value();
    int brightness = req->hasParam("brightness") ? req->getParam("brightness")->value().toInt() : 255;
    brightness = constrain(brightness, 0, 255);

    int pin = -1;
    if (color == "red") pin = LED_R;
    else if (color == "yellow") pin = LED_J;
    else if (color == "green") pin = LED_V;

    if (pin == -1) {
      req->send(400, "application/json", R"({"error":"Unknown color"})");
      return;
    }

    if (state == "on") {
      // Only set the requested LED, do not turn off others
      analogWrite(pin, brightness);
    } else if (state == "off") {
      analogWrite(pin, 0);
    } else {
      req->send(400, "application/json", R"({"error":"Unknown state"})");
      return;
    }

    String resp = "{\"color\":\"" + color + "\",\"state\":\"" + state + "\",\"brightness\":" + String(brightness) + "}";
    req->send(200, "application/json", resp);
  });

  // Smoke control
  server.on("/api/smoke", HTTP_GET, [](AsyncWebServerRequest *req) {
    bool necroState = digitalRead(NECRO);
    bool dragonState = digitalRead(DRAGON);

    if (req->hasParam("necro")) {
      String val = req->getParam("necro")->value();
      necroState = (val == "on");
      digitalWrite(NECRO, necroState);
    }

    if (req->hasParam("dragon")) {
      String val = req->getParam("dragon")->value();
      dragonState = (val == "on");
      digitalWrite(DRAGON, dragonState);
    }

    String resp = "{\"necro\":\"" + String(necroState ? "on" : "off") +
                  "\",\"dragon\":\"" + String(dragonState ? "on" : "off") + "\"}";
    req->send(200, "application/json", resp);
  });

  // === DFPlayer-backed Audio endpoints ===

  // List files on SD - not supported by DFPlayer over UART
  server.on("/api/sd/list", HTTP_GET, [](AsyncWebServerRequest *req) {
    StaticJsonDocument<256> doc;
    doc["error"] = "Listing files is not available when using DFPlayer Mini over UART. Use numeric track indices with /api/sd/play?path=/001.mp3 or /api/sd/play?path=1";
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // Play file (path param required) - DFPlayer expects numeric track index or filename starting with number
  server.on("/api/sd/play", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!req->hasParam("path")) {
      req->send(400, "application/json", R"({"error":"Missing 'path' param"})");
      return;
    }
    String path = req->getParam("path")->value();
    bool ok = playFile(path.c_str());
    if (!ok) {
      req->send(500, "application/json", R"({"error":"Failed to start playback; use numeric index like /api/sd/play?path=/001.mp3 or /api/sd/play?path=1"})");
      return;
    }
    String resp = "{\"playing\":true,\"path\":\"" + path + "\"}";
    req->send(200, "application/json", resp);
  });

  // Stop playback
  server.on("/api/sd/stop", HTTP_GET, [](AsyncWebServerRequest *req) {
    stopPlayback();
    req->send(200, "application/json", R"({"playing":false})");
  });

  // Playback status
  server.on("/api/sd/status", HTTP_GET, [](AsyncWebServerRequest *req) {
    StaticJsonDocument<128> doc;
    doc["playing"] = isPlaying();
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // Volume control: GET to read, pass ?level=N to set (0..30)
  server.on("/api/sd/volume", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (req->hasParam("level")) {
      int level = req->getParam("level")->value().toInt();
      if (level < 0 || level > 30) {
        req->send(400, "application/json", R"({"error":"level must be between 0 and 30"})");
        return;
      }
      bool ok = audioSetVolume(level);
      DynamicJsonDocument doc(128);
      doc["set"] = ok;
      doc["volume"] = audioGetVolume();
      String out;
      serializeJson(doc, out);
      req->send(ok ? 200 : 500, "application/json", out);
    } else {
      DynamicJsonDocument doc(128);
      doc["volume"] = audioGetVolume();
      String out;
      serializeJson(doc, out);
      req->send(200, "application/json", out);
    }
  });

  // Reinitialize DFPlayer (attempt retries) and return success
  server.on("/api/sd/reinit", HTTP_GET, [](AsyncWebServerRequest *req) {
    bool ok = audioReinit();
    DynamicJsonDocument doc(256);
    doc["reinit"] = ok;
    doc["info"] = audioGetInfo();
    String out;
    serializeJson(doc, out);
    req->send(ok ? 200 : 500, "application/json", out);
  });

  // Return DFPlayer diagnostic info (last init messages)
  server.on("/api/sd/info", HTTP_GET, [](AsyncWebServerRequest *req) {
    String info = audioGetInfo();
    if (info.length() == 0) {
      req->send(200, "text/plain", "No DFPlayer diagnostic info available");
    } else {
      req->send(200, "text/plain", info);
    }
  });

  // Mount static files (React build) after API routes so /api/* isn't intercepted
  server.serveStatic("/", LittleFS, "/")
      .setDefaultFile("index.html")
      .setCacheControl("no-cache")
      .setLastModified(time(nullptr))
      .setAuthentication("", ""); // no gzip

  server.begin();
  Serial.println("[OK] AsyncWebServer started on port 80");
}
