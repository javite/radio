#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef _SPIFFS_H_
#include "SPIFFS.h"
#endif

#include <ArduinoJson.h>

#define RADIO_PATH "/radio.json"
#define STATIONS_PATH "/stations.json"
#define DEFAULT_FRECUENCY 9590

class FileManager {
    private:

    public:
        FileManager();
        JsonDocument ReadRadioFile(String path);
        bool saveOnDisk(String content, String path);
};

#endif