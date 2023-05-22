
#include <Wire.h>
// #include <FastLED.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <OneButton.h>
#include "TEA5767.h"
#include "DialScreen.h"

TFT_eSPI tft = TFT_eSPI();
DialScreen dialScreen(&tft);

#define PIN_IN1 15
#define PIN_IN2 13
#define PIN_INPUT 2
#define PIN_SDA 21
#define PIN_SCL 22
// #define NUM_LEDS 7
// #define DATA_PIN 42
// #define CLOCK_PIN 45
// CRGB leds[NUM_LEDS];

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);
OneButton menu_button(PIN_INPUT, true);
OneButton mute_button(0, true);

TEA5767 radio;

#define color1 0xC638
#define color2 0xC638
#define MIN_FREQ 8750
#define MAX_FREQ 10810
#define STEP_FREQ 20

float freq = 9590;

int strength = 0;
String sta[6] = {"96.6", "101.0", "89.4", "106,5", "98.2", "92.4"};

bool muted = false;
int deb = 0;
bool encoder_changed =  false;
bool button_pressed = false;
unsigned long pressStartTime;

enum Direction {
  BACKWARD = 0,
  FORDWARD = 1
};

enum Direction direction = FORDWARD;
enum TuneType tuneType = MANUAL;

RADIO_INFO radio_info;

void updateScreen(){
  dialScreen.update(freq, &radio_info);
  // delay(200);
}

void update() {
  // float prev_freq = freq;
  const int minimum_rssi = 10;
  uint8_t rssi = 0;
  if (!muted){
    switch (tuneType){
      case MANUAL:
        radio.setFrequency(freq);
        freq = radio.getFrequency();
        break;
      case SEARCH:
        do {
          if(direction == FORDWARD){
            radio.seekUp();
          } else {
            radio.seekDown();
          }
          radio.getRadioInfo(&radio_info);
          rssi = radio_info.rssi;
          freq = radio.getFrequency();
          updateScreen();
        } while ( rssi < minimum_rssi && freq < MAX_FREQ && freq > MIN_FREQ );
        break;
      case PRESET:
        radio.setFrequency(freq);
        freq = radio.getFrequency();
        break;
      default:
        break;
    }
  }
  
  // radio.getRadioInfo(&radio_info);
  // Serial.println(radio_info.rssi);
  updateScreen();
}

void readEncoder() {
  static int pos = 0;
  int newPos = encoder.getPosition();
  if (pos != newPos) {
    if (newPos > pos) {
      freq = freq - STEP_FREQ;
      if(freq < MIN_FREQ){
        freq = MAX_FREQ;
      }
      direction = BACKWARD;
    }
    if (newPos < pos) {
      freq = freq + STEP_FREQ;
      if(freq > MAX_FREQ){
        freq = MIN_FREQ;
      }
      direction = FORDWARD;
    }
    pos = newPos;
    encoder_changed = true;
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
    update();
}

void singleClick() {
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
  dialScreen.setTuneType(tuneType);
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
} // pressStart()

void pressStop() {
  Serial.print("pressStop(");
  Serial.print(millis() - pressStartTime);
  Serial.println(") detected.");
} // pressStop()

void setup() {
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);
  
  menu_button.attachClick(singleClick);
  menu_button.attachDoubleClick(doubleClick);
  menu_button.attachMultiClick(multiClick);
  menu_button.setPressTicks(500);
  menu_button.attachLongPressStart(pressStart);
  menu_button.attachLongPressStop(pressStop);
  //menu_button.setDebounceTicks(100);
  mute_button.attachClick(mute);

  tft.begin();
  tft.writecommand(0x11);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  // FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  Wire.begin(PIN_SDA, PIN_SCL);
  radio.init();
  radio.debugEnable(false);
  radio.setBand(RADIO_BAND_FM);
  radio.setMute(muted);

  // leds[0] = CRGB::Red;
  // leds[1] = CRGB::White;
  // leds[2] = CRGB::Red;
  // leds[3] = CRGB::Green;
  // leds[4] = CRGB::Red;
  // leds[5] = CRGB::Blue;
  // leds[6] = CRGB::Red;
  // FastLED.show();
  // dialScreen.update(value, &radio_info);
  update();

}

void loop() {
  if(encoder_changed){
    encoder_changed = false;
    update();
  }
  menu_button.tick();
  mute_button.tick();

}
