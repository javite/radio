#pragma once

#ifndef DialScreen_h
#include <DialScreen.h>
#endif

#include <ArduinoJson.h>
#include <vector>
#include "FileManager.h"

#define RADIO_FILE_SIZE 1024

class RadioFile {
    private:
        FileManager& fileManager;
        int stationsCount;
        int actualPreset;
        int actualFrequency;
        int tuneType;
        std::vector<int> stations;
        bool saveFile;
        bool saveStations;

    public:
        RadioFile(FileManager& fm);
        bool saveTuneType(int tuneType);
        bool saveActualFrequency(long frequency);
        void actualPresetUp();
        void actualPresetDown();
        bool saveActualPreset();
        bool saveStation(int station);
        bool deleteStation(int station);
        TuneType getTuneType();
        bool shouldSave();
        bool saveRadioFile();
        long getActualFrequency() const {
            return actualFrequency;
        }
        void setActualFrequency(long freq) {
            this->actualFrequency = freq;
        }
        int getActualPreset() const {
            return actualPreset;
        }
        
        const std::vector<int>& getStations() const {
            return stations;
        }
        size_t getStationsCount() const {
            return stationsCount;
        }
};