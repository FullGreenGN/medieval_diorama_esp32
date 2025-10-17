#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "Leds.h"
#include "Smoke.h"
#include "WebServer.h"
#include "AudioPlayer.h"
#include "Logger.h"
#include "Pwm.h"
#include "WifiRouter.h"

AsyncWebServer server(80);

void setupWebServer() {
  // ✅ React Router fallback
  server.onNotFound([](AsyncWebServerRequest *request) {
    String path = request->url();
    if (!path.startsWith("/api")) {
      // Return index.html for React routes
      request->send(LittleFS, "/index.html", "text/html");
    } else {
      LOGE("("+request->url() + ") API route not found");
      request->send(404, "application/json", R"({"error":"API route not found"})");
    }
  });

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
    if (color == "red") pin = LED_ONE;
    else if (color == "yellow") pin = LED_TWO;
    else if (color == "green") pin = LED_THREE;

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

  // Mill (PWM) control - read or set mill power
  server.on("/api/mill", HTTP_GET, [](AsyncWebServerRequest *req) {
    StaticJsonDocument<192> doc;

    // If `power` param present, attempt to set PWM. For safety require `pwd` query param.
    if (req->hasParam("power")) {
      if (!req->hasParam("pwd")) {
        doc["error"] = "Missing 'pwd' parameter";
        String out;
        serializeJson(doc, out);
        req->send(401, "application/json", out);
        return;
      }

      String pwd = req->getParam("pwd")->value();
      if (pwd != String(WIFI_PASSWORD)) {
        doc["error"] = "Unauthorized";
        String out;
        serializeJson(doc, out);
        req->send(401, "application/json", out);
        return;
      }

      int power = req->getParam("power")->value().toInt();
      power = constrain(power, 0, 255);
      setPwm(power);
      doc["set"] = true;
      doc["power"] = getPwm();

      String out;
      serializeJson(doc, out);
      req->send(200, "application/json", out);
      return;
    }

    // No power param: return current status
    doc["power"] = getPwm();
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // Boost / fire-effect control endpoint
  server.on("/api/boost", HTTP_GET, [](AsyncWebServerRequest *req) {
    StaticJsonDocument<256> doc;

    // Supported query forms:
    //  - /api/boost?action=start
    //  - /api/boost?action=stop
    //  - /api/boost?start=true
    //  - /api/boost?stop=true
    //  - /api/boost  -> returns status

    bool handled = false;
    if (req->hasParam("action")) {
      String action = req->getParam("action")->value();
      if (action == "start") {
        startFireEffect();
        doc["action"] = "start";
        handled = true;
      } else if (action == "stop") {
        stopFireEffect();
        doc["action"] = "stop";
        handled = true;
      }
    }

    if (!handled) {
      if (req->hasParam("start")) {
        String v = req->getParam("start")->value();
        if (v == "1" || v.equalsIgnoreCase("true")) {
          startFireEffect();
          doc["action"] = "start";
          handled = true;
        }
      }
    }

    if (!handled) {
      if (req->hasParam("stop")) {
        String v = req->getParam("stop")->value();
        if (v == "1" || v.equalsIgnoreCase("true")) {
          stopFireEffect();
          doc["action"] = "stop";
          handled = true;
        }
      }
    }

    // If not a state-changing request, just return current status
    doc["active"] = isFireEffectActive();
    doc["message"] = handled ? "Action applied" : "No action; returning status";

    String out;
    serializeJson(doc, out);
    req->send(handled ? 200 : 200, "application/json", out);
  });

  // Smoke control
  server.on("/api/smoke", HTTP_GET, [](AsyncWebServerRequest *req) {
    StaticJsonDocument<256> doc;

    // If `action` param present, perform a command
    if (req->hasParam("action")) {
      String action = req->getParam("action")->value();

      if (action == "try") {
        trySmoke();
        doc["action"] = "try";
        doc["message"] = "Performed smoke test";

      } else if (action == "on") {
        turnOnSmoke();
        doc["action"] = "on";
        doc["message"] = "Smoke output(s) turned on";

      } else if (action == "off") {
        turnOffSmoke();
        doc["action"] = "off";
        doc["message"] = "Smoke output(s) turned off";

      } else if (action == "set") {
        // set expects led (1|2 or pin) and brightness (0..255)
        if (!req->hasParam("led") || !req->hasParam("brightness")) {
          doc["error"] = "Missing 'led' or 'brightness' parameter for action=set";
          String out;
          serializeJson(doc, out);
          req->send(400, "application/json", out);
          return;
        }

        int ledVal = req->getParam("led")->value().toInt();
        int brightness = req->getParam("brightness")->value().toInt();
        brightness = constrain(brightness, 0, 255);

        int pin = -1;
        if (ledVal == 1) pin = SMOKE_1;
        else if (ledVal == 2) pin = SMOKE_2;
        else pin = ledVal; // allow direct pin numbers if user passes them

        setSmoke(pin, brightness);
        doc["action"] = "set";
        doc["led"] = ledVal;
        doc["brightness"] = brightness;
        doc["message"] = "Smoke output set";

      } else {
        doc["error"] = "Unknown action; allowed: try,on,off,set";
        String out;
        serializeJson(doc, out);
        req->send(400, "application/json", out);
        return;
      }
    }

    // Always include current status of both smoke outputs
    doc["smoke1"] = getSmoke(SMOKE_1);
    doc["smoke2"] = getSmoke(SMOKE_2);

    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
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