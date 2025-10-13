//
// Created by FullGreen on 13/10/2025.
//

#include "Files.h"

void setupFileSystem() {
    if (!LittleFS.begin(true)) {
        Serial.println("[ERROR] LittleFS mount failed!");
        return;
    }
    Serial.println("[OK] LittleFS mounted successfully");
}
