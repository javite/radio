#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef _SPIFFS_H_
#include "SPIFFS.h"
#endif

#ifndef DialScreen_h
#include <DialScreen.h>
#endif

#include <ArduinoJson.h>

#define RADIO_PATH "/radio.json"
#define RADIO_FILE_SIZE 1024


class FileManager {
    private:

    public:
        FileManager();
        ~FileManager();
        JsonDocument ReadRadioFile();
        bool saveOnDisk(String content, String path);
        bool saveTuneType(int tuneType);
        TuneType readTuneType();
        bool saveActualFrequency(long frequency);
        long readActualFrequency();
        bool saveActualPreset(int preset);
        int readActualPreset();
};

#endif