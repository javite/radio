
#include <Wire.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <OneButton.h>
#include "TEA5767.h"
#include "DialScreen.h"

#include <FileManager.h>
#include <RadioFile.h>

#define PIN_IN1 15
#define PIN_IN2 13
#define PIN_INPUT 2
#define PIN_SDA 21
#define PIN_SCL 22
#define NUM_LEDS 8
#define DATA_PIN 32

#define color1 0xC638
#define color2 0xC638
#define MIN_FREQ 8750
#define MAX_FREQ 10810
#define STEP_FREQ 20

TFT_eSPI tft = TFT_eSPI();
DialScreen dialScreen(&tft);
CRGB leds[NUM_LEDS];
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);
OneButton menu_button(PIN_INPUT, true);
OneButton mute_button(0, true);
TEA5767 radio;
RADIO_INFO radio_info;
FileManager filemanager;
RadioFile* radioManager;
long encoder_count;
int strength = 0;
int index_leds = 0;
int index_colors = 0;
bool muted = false;
bool encoder_changed =  false;
bool enable_save = false;
bool leds_changed = false;
bool button_pressed = false;
unsigned long pressStartTime = 0;
unsigned long lastChangeTime = 0;
const unsigned long TIME_BEFORE_SAVING = 2000;

enum Direction {
  INIT = -1,
  BACKWARD = 0,
  FORDWARD = 1
};

enum Direction direction = INIT;

CRGB colors[8] = { CRGB::Aqua, CRGB::Bisque, CRGB::YellowGreen, CRGB::Violet, CRGB::Red, CRGB::Cyan, CRGB::Green, CRGB::HotPink };

void updateScreen(){
  dialScreen.setTuneType(radioManager->getTuneType());
  dialScreen.update(radioManager->getActualFrequency(), &radio_info);
}

void updateFrecuency() {
  long freq;
  long prev_freq = radioManager->getActualFrequency();
  const int minimum_rssi = 10;
  uint8_t rssi = 0;
  if (!muted){
    switch (radioManager->getTuneType()){
      case MANUAL:
       radioManager->saveActualFrequency(encoder_count);
        break;
      case SEARCH:
        do {
          switch (direction) {
            case INIT:
              radio.setFrequency(encoder_count);
              break;
            case FORDWARD:
              radio.seekUp();
              break;
            case BACKWARD:
              radio.seekDown();
              break;
            default:
              Serial.println(F("Dirección no reconocida"));
              break;
          }
          radio.getRadioInfo(&radio_info);
          rssi = radio_info.rssi;
          freq = radio.getFrequency();
          radioManager->setActualFrequency(freq);
          updateScreen();
        } while ( rssi < minimum_rssi && freq != prev_freq );
       radioManager->saveActualFrequency(freq);
        break;
      case PRESET:
       radioManager->saveActualPreset();
        break;
      default:
        break;
    }
  }
  radio.setFrequency(radioManager->getActualFrequency());
  radio.getRadioInfo(&radio_info);
  encoder_count = radioManager->getActualFrequency();
  updateScreen();
}

void updatedLEDS(){
  if(direction == FORDWARD){
    index_leds++;
  } else {
    index_leds--;
  }
  if(index_leds > (NUM_LEDS - 1)){
    index_leds = 0;
  } else if(index_leds < 0){
    index_leds = NUM_LEDS - 1;
  }
  for (size_t i = 0; i < NUM_LEDS; i++){
    if(i == index_leds){
      leds[i] = colors[index_colors];
    } else {
      leds[i] = CRGB::Black;
    }
    index_colors++;
    if(index_colors > 7){
      index_colors = 0;
    } else if(index_colors < 0){
      index_colors = 7;
    }
  }
  FastLED.show();
}

void readEncoder() {
  static int pos = 0;
  int newPos = encoder.getPosition();
  if (pos != newPos) {
    if (newPos > pos) {
      direction = BACKWARD;
    } else {
      direction = FORDWARD;
    } 
    switch (radioManager->getTuneType()){
      case MANUAL:
        if(direction == BACKWARD){
          encoder_count = encoder_count - STEP_FREQ;
          if(encoder_count < MIN_FREQ){
            encoder_count = MAX_FREQ;
          }
        } else {
          encoder_count = encoder_count + STEP_FREQ;
          if(encoder_count > MAX_FREQ){
            encoder_count = MIN_FREQ;
          }
        }
        break;
      case SEARCH:
        break;
      case PRESET:
        if(direction == FORDWARD){
         radioManager->actualPresetUp();
        } else {
         radioManager->actualPresetDown();
        }
        break;
      default:
        break;
    }
    pos = newPos;
    encoder_changed = true;
    leds_changed = true;
  }
}

void IRAM_ATTR checkPosition() {
  encoder.tick();
  readEncoder();
}

void mute() {
    muted = !muted;
    radio.setMute(muted);
    dialScreen.setMute(muted);
    updateScreen();
}

void singleClick() {
  TuneType nextTuneType;
  TuneType prevTuneType =radioManager->getTuneType();
  switch (radioManager->getTuneType()){
    case MANUAL:
      nextTuneType = SEARCH;
      break;
    case SEARCH:
      nextTuneType = PRESET;
      break;  
    case PRESET:
      nextTuneType = MANUAL;
      break; 
    default:
      nextTuneType = PRESET;
      break;
  }
  if (nextTuneType != prevTuneType) {
    if(radioManager->saveTuneType(static_cast<int>(nextTuneType))){
      lastChangeTime = millis() - TIME_BEFORE_SAVING;
      enable_save = true;
    };
  }
  updateScreen();
} // singleClick

void doubleClick() {
  Serial.println("doubleClick() detected.");
} 

void multiClick() {
  Serial.print("multiClick(");
  Serial.print(menu_button.getNumberClicks());
  Serial.println(") detected.");
} 

void pressStart() {
  pressStartTime = millis();
}

void pressStop() {
  if ((millis() - pressStartTime) > 200){
    if(radioManager->getTuneType() == PRESET){
     radioManager->deleteStation(radioManager->getActualFrequency());
    } else {
      radioManager->saveStation(radioManager->getActualFrequency());
    }
    lastChangeTime = millis() - TIME_BEFORE_SAVING;
    enable_save = true;
  }
}

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
  }

  radioManager = new RadioFile(filemanager); 
  
  encoder_count = radioManager->getActualFrequency();

  if (!radioManager->getStations().empty()) {
    Serial.println("Estaciones leídas:");
    for (int station :radioManager->getStations()) {
        Serial.println(station);
    }
  } else {
    Serial.println("else");
  }

  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);
  
  menu_button.attachClick(singleClick);
  menu_button.attachDoubleClick(doubleClick);
  menu_button.attachMultiClick(multiClick);
  menu_button.setPressTicks(400);
  menu_button.attachLongPressStart(pressStart);
  menu_button.attachLongPressStop(pressStop);
  menu_button.setDebounceTicks(20);

  mute_button.attachClick(mute);

  tft.begin();
  tft.writecommand(0x11);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  Wire.begin(PIN_SDA, PIN_SCL);

  radio.init();
  radio.debugEnable(false);
  radio.setBand(RADIO_BAND_FM);
  radio.setMute(muted);
  radio.setFreqLow(MIN_FREQ);
  radio.setFreqHigh(MAX_FREQ);
  radio.setFreqSteps(STEP_FREQ);
  
  updatedLEDS();
  updateFrecuency();
}

void loop() {
  if(encoder_changed){
    updateFrecuency();
    encoder_changed = false;
    lastChangeTime = millis();
    enable_save = true;
  }
  if(enable_save){
    if((millis() - lastChangeTime > TIME_BEFORE_SAVING) && radioManager->shouldSave()){
      detachInterrupt(digitalPinToInterrupt(PIN_IN1));
      detachInterrupt(digitalPinToInterrupt(PIN_IN2));
      if(!radioManager->saveRadioFile()){
        Serial.println("Error on saving file");
      };
      
      attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
      attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);
      enable_save = false;
    }
  }
  if(leds_changed){
    updatedLEDS();
    leds_changed = false;
  }
  menu_button.tick();
  mute_button.tick();
}
