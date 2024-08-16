 #include <FileManager.h>
 
 FileManager::FileManager(){

 }

 FileManager::~FileManager(){

 }

JsonDocument FileManager::ReadRadioFile() {
    JsonDocument doc;
    if (!SPIFFS.exists(RADIO_PATH)) {
        Serial.println(F("radio.json no existe."));
        return doc;
    }
    File radio_file = SPIFFS.open(RADIO_PATH);
    DeserializationError error = deserializeJson(doc, radio_file);
    radio_file.close();
    if (error) {
        Serial.print(F("Error de deserialización JSON: "));
        Serial.println(error.c_str());
    }
    return doc;
}

bool FileManager::saveOnDisk(String content, String path){
    bool res = false;
    Serial.print(F("Saving file on disk: "));
    Serial.println(path);
    if(SPIFFS.exists(path)){
        SPIFFS.remove(path);
    }
    File file_name = SPIFFS.open(path, FILE_WRITE);
    if(file_name.print(content)){
        Serial.print(F("File written: "));
        Serial.println(path);
        res = true;
    } else {
        Serial.print(F("Fail to write file: "));
        Serial.println(path);
        res = false;
    }
    file_name.close();
    return res;
}

bool FileManager::saveTuneType(int tuneType) {
    JsonDocument doc = this->ReadRadioFile();
    if (doc.isNull()) {
        Serial.println(F("Error: No se pudo leer el archivo radio.json"));
        return false;
    }
    doc["tuneType"] = tuneType;
    String updatedJson;
    if (serializeJson(doc, updatedJson) == 0) {
        Serial.println(F("Error: Fallo al serializar el JSON"));
        return false;
    }
    if (!this->saveOnDisk(updatedJson, RADIO_PATH)) {
        Serial.println(F("Error: No se pudo guardar el archivo actualizado"));
        return false;
    }
    return true;
}

TuneType FileManager::readTuneType() {
    JsonDocument doc = this->ReadRadioFile();
    if (doc.isNull() || !doc.containsKey("tuneType")) {
        Serial.println(F("Error: No se pudo leer tuneType del archivo radio.json"));
        return PRESET;
    }
    int tuneTypeValue = doc["tuneType"].as<int>();
    switch (tuneTypeValue) {
        case 0:
            return MANUAL;
        case 1:
            return SEARCH;
        case 2:
            return PRESET;
        default:
            Serial.println(F("Error: Valor de tuneType no reconocido"));
            return PRESET;
    }
}

bool FileManager::saveActualFrequency(long frequency) {
    JsonDocument doc = this->ReadRadioFile();
    if (doc.isNull()) {
        Serial.println(F("Error: No se pudo leer el archivo radio.json"));
        return false;
    }
    doc["actual_freq"] = frequency;
    String updatedJson;
    if (serializeJson(doc, updatedJson) == 0) {
        Serial.println(F("Error: Fallo al serializar el JSON"));
        return false;
    }
    if (!this->saveOnDisk(updatedJson, RADIO_PATH)) {
        Serial.println(F("Error: No se pudo guardar el archivo actualizado"));
        return false;
    }
    return true;
}

long FileManager::readActualFrequency() {
    JsonDocument doc = this->ReadRadioFile();
    if (doc.isNull() || !doc.containsKey("actual_freq")) {
        Serial.println(F("Error: No se pudo leer actual_freq del archivo radio.json"));
        return 0; // Valor por defecto si hay un error
    }
    long frequency = doc["actual_freq"].as<long>();
    if (frequency < 0) {
        Serial.println(F("Error: Frecuencia inválida leída del archivo"));
        return 0;
    }
    return frequency;
}

bool FileManager::saveActualPreset(int preset) {
    JsonDocument doc = this->ReadRadioFile();
    if (doc.isNull()) {
        Serial.println(F("Error: No se pudo leer el archivo radio.json"));
        return false;
    }
    doc["actual_preset"] = preset;
    String updatedJson;
    if (serializeJson(doc, updatedJson) == 0) {
        Serial.println(F("Error: Fallo al serializar el JSON"));
        return false;
    }
    if (!this->saveOnDisk(updatedJson, RADIO_PATH)) {
        Serial.println(F("Error: No se pudo guardar el archivo actualizado"));
        return false;
    }
    Serial.println(F("Preset actual actualizado y guardado con éxito"));
    return true;
}

int FileManager::readActualPreset() {
    JsonDocument doc = this->ReadRadioFile();
    if (doc.isNull() || !doc.containsKey("actual_preset")) {
        Serial.println(F("Error: No se pudo leer actual_preset del archivo radio.json"));
        return 0;
    }
    int preset = doc["actual_preset"].as<int>();
    if (preset < 0) {
        Serial.println(F("Error: Preset inválido leído del archivo"));
        return 0;
    }
    return preset;
}