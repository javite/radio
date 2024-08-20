#include "RadioFile.h"

RadioFile::RadioFile(FileManager& fm) : fileManager(fm) {
    JsonDocument doc = fileManager.ReadRadioFile(RADIO_PATH);
    this->tuneType  = doc["tune_type"];
    this->actualPreset = doc["actual_preset"];
    this->actualFrequency = doc["actual_freq"];
    String initialJson;
    serializeJson(doc, initialJson);
    fileManager.saveOnDisk(initialJson, RADIO_PATH);

    JsonDocument doc2 = fileManager.ReadRadioFile(STATIONS_PATH);
    if (doc2.containsKey("stations") && doc2["stations"].is<JsonArray>() && doc2["stations"].as<JsonArray>().size() > 0) {
        JsonArray stationsArray = doc2["stations"].as<JsonArray>();
        this->stations.clear();
        for (JsonVariant station : stationsArray) {
            if (station.is<int>()) {
                this->stations.push_back(station.as<int>());
            }
        }
    } else {
        this->stations.push_back(DEFAULT_FRECUENCY);
    }
    serializeJson(doc2, initialJson);
    fileManager.saveOnDisk(initialJson, STATIONS_PATH);
    this->stationsCount = this->stations.size();
    this->saveFile = false;
    this->saveStations = false;
}

bool RadioFile::saveRadioFile(){
    if(this->saveFile){
        JsonDocument doc = fileManager.ReadRadioFile(RADIO_PATH);
        if (doc.isNull()) {
            Serial.println(F("Error: opening radio.json"));
            return false;
        }
        doc["tune_type"] = this->tuneType;
        doc["actual_freq"] = this->actualFrequency;
        doc["actual_preset"] = this->actualPreset;
        String updatedJson;
        serializeJson(doc, updatedJson);
        if(fileManager.saveOnDisk(updatedJson, RADIO_PATH)){
            this->saveFile = false;
            return true;
        }
        return false;
    }
    if(this->saveStations){
        JsonDocument doc2 = fileManager.ReadRadioFile(STATIONS_PATH);
        if (doc2.isNull()) {
            Serial.println(F("Error: opening stations.json"));
            return false;
        }
        doc2["stations"].clear();
        for (int station : this->stations) {
            doc2["stations"].add(station);
        }
        String updatedJson;
        serializeJson(doc2, updatedJson);
        if(fileManager.saveOnDisk(updatedJson, STATIONS_PATH)){
            this->saveStations = false;
            return true;
        }
        return false;
    }
    return true;
}

bool RadioFile::saveTuneType(int tuneType) {
    this->tuneType = tuneType;
    this->saveFile = true;
    return true;
}

TuneType RadioFile::getTuneType() {
    switch (this->tuneType) {
        case 0: return MANUAL;
        case 1: return SEARCH;
        case 2: return PRESET;
        default: return PRESET;
    }
}

bool RadioFile::saveActualFrequency(long frequency) {
    this->actualFrequency = frequency;
    this->saveFile = true;
    return true;
}

void RadioFile::actualPresetUp(){
    this->actualPreset++;
    if(this->actualPreset > this->stationsCount - 1){
        this->actualPreset = 0;
    }
    this->actualFrequency = this->stations[this->actualPreset];
    this->saveFile = true;
}

void RadioFile::actualPresetDown(){
    this->actualPreset--;
    if(this->actualPreset < 0){
        this->actualPreset = this->stationsCount - 1;
    }
    this->actualFrequency = this->stations[this->actualPreset];
    this->saveFile = true;
}

bool RadioFile::saveActualPreset() {
    this->saveFile = true;
    return true;
}

bool RadioFile::saveStation(int station) {
    if (station > 11000) {
        Serial.println(F("Error: Valor de estación superior a 11000"));
        return false;
    }
    if (std::find( this->stations.begin(),  this->stations.end(), station) !=  this->stations.end()) {
        Serial.println(F("La estación ya existe, no se guardará de nuevo"));
        return true;
    }
    this->stations.push_back(station);
    this->stationsCount = this->stations.size();
    std::sort( this->stations.begin(),  this->stations.end());
    this->saveStations = true;
    return true;
}

bool RadioFile::deleteStation(int station) {
    auto it = std::find(this->stations.begin(), this->stations.end(), station);
    if (it == this->stations.end()) {
        Serial.println(F("La estación no existe en la lista"));
        return false;
    }
    this->stations.erase(it);
    this->stationsCount = this->stations.size();
    std::sort( this->stations.begin(),  this->stations.end());
    this->saveStations = true;
    return true;
}

bool RadioFile::shouldSave(){
    return this->saveFile || this->saveStations;
}