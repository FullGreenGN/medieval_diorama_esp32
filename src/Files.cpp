//
// Created by FullGreen on 13/10/2025.
//

#include "Files.h"

#include "Logger.h"

void setupFileSystem() {
    if (!LittleFS.begin(true)) {
        LOGE("LittleFS mount failed!");
        return;
    }
    LOGI("LittleFS mounted successfully");
}
