
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
#define STATIONS_AMOUNT 10

TFT_eSPI tft = TFT_eSPI();
DialScreen dialScreen(&tft);
CRGB leds[NUM_LEDS];
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);
OneButton menu_button(PIN_INPUT, true);
OneButton mute_button(0, true);
TEA5767 radio;
RADIO_INFO radio_info;
FileManager filemanager;
long freq, encoder_count;
int strength = 0;
int index_station = 0;
int index_leds = 0;
int index_colors = 0;
bool muted = false;
bool encoder_changed =  false;
bool leds_changed = false;
bool button_pressed = false;
unsigned long pressStartTime;
long station[STATIONS_AMOUNT] = { 8990, 9510, 9590, 9710, 9830, 10070, 10150, 10230, 10310, 10430 };

enum Direction {
  INIT = -1,
  BACKWARD = 0,
  FORDWARD = 1
};

enum Direction direction = INIT;
enum TuneType tuneType = PRESET;

CRGB colors[8] = { CRGB::Aqua, CRGB::Bisque, CRGB::YellowGreen, CRGB::Violet, CRGB::Red, CRGB::Cyan, CRGB::Green, CRGB::HotPink };

void updateScreen(){
  dialScreen.setTuneType(tuneType);
  dialScreen.update(freq, &radio_info);
}

void updateFrecuency() {
  long prev_freq = freq;
  const int minimum_rssi = 10;
  uint8_t rssi = 0;
  if (!muted){
    switch (tuneType){
      case MANUAL:
        freq = encoder_count;
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
          updateScreen();
        } while ( rssi < minimum_rssi && freq != prev_freq );
        break;
      case PRESET:
        freq = station[index_station];
        filemanager.saveActualPreset(index_station);
        break;
      default:
        break;
    }
  }
  radio.setFrequency(freq);
  radio.getRadioInfo(&radio_info);
  filemanager.saveActualFrequency(freq);
  updateScreen();
  encoder_count = freq ;
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
    switch (tuneType){
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
          index_station++;
          if(index_station > STATIONS_AMOUNT - 1){
            index_station = 0;
          }
        } else {
          index_station--;
          if(index_station < 0){
            index_station = STATIONS_AMOUNT - 1;
          }
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
  TuneType prevTuneType = tuneType;
  switch (tuneType){
    case MANUAL:
      tuneType = SEARCH;
      break;
    case SEARCH:
      tuneType = PRESET;
      break;  
    case PRESET:
      tuneType = MANUAL;
      break; 
    default:
      break;
  }
  if (tuneType != prevTuneType) {
    filemanager.saveTuneType(static_cast<int>(tuneType));
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
  Serial.println("pressStart()");
  pressStartTime = millis();
} // pressStart()

void pressStop() {
  Serial.print("pressStop(");
  Serial.print(millis() - pressStartTime);
  Serial.println(") detected.");
} // pressStop()

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
  }
  
  tuneType = filemanager.readTuneType();
  freq = filemanager.readActualFrequency();
  index_station = filemanager.readActualPreset();
  encoder_count = freq;
  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);
  
  menu_button.attachClick(singleClick);
  menu_button.attachDoubleClick(doubleClick);
  menu_button.attachMultiClick(multiClick);
  menu_button.setPressTicks(500);
  menu_button.attachLongPressStart(pressStart);
  menu_button.attachLongPressStop(pressStop);
  menu_button.setDebounceTicks(50);

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
  }
  if(leds_changed){
    updatedLEDS();
    leds_changed = false;
  }
  menu_button.tick();
  mute_button.tick();
}
