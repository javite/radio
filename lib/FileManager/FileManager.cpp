 #include <FileManager.h>
 
 FileManager::FileManager(){}

JsonDocument FileManager::ReadRadioFile(String path) {
    JsonDocument doc;
    if (!SPIFFS.exists(path)) {
        Serial.print(path);
        Serial.println(F(" no existe. Creando archivo..."));
        if(path == RADIO_PATH){
            doc["tune_type"] = 2;
            doc["actual_freq"] = DEFAULT_FRECUENCY;
            doc["actual_preset"] = 0;
        } else {
            doc["stations"] = JsonArray();
            doc["stations"].add(DEFAULT_FRECUENCY);
        }
        // String initialJson;
        // serializeJson(doc, initialJson);
    } else {
        File radio_file = SPIFFS.open(path);
        DeserializationError error = deserializeJson(doc, radio_file);
        radio_file.close();
        if (error) {
            Serial.print(F("Error de deserialización JSON: "));
            Serial.println(error.c_str());
        }
    }
    return doc;
}

bool FileManager::saveOnDisk(String content, String path){
    bool res = false;
    Serial.print(F("Saving file on disk: "));
    Serial.println(content);
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
