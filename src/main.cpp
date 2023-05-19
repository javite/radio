
#include <Wire.h>
#include <TEA5767.h>
// #include <FastLED.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <OneButton.h>
#include "DialScreen.h"

TFT_eSPI tft = TFT_eSPI();
DialScreen dialScreen(&tft);

#define PIN_IN1 15
#define PIN_IN2 13
#define PIN_INPUT 2

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

int value = 959;
int minimal = 880;
int maximal = 1080;
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

enum TuneType {
  MANUAL = 0,
  SEARCH = 1,
  PRESET = 2
};

enum Direction direction = FORDWARD;
enum TuneType tuneType = MANUAL;

RADIO_INFO radio_info;

void drawScreen() {
  float freq = value * 10.00;

  if (muted == false){
    switch (tuneType){
      case MANUAL:
        radio.setFrequency(freq);
        radio.getRadioInfo(&radio_info);

        Serial.println(radio_info.active);
        Serial.println(radio_info.mono);
        Serial.println(radio_info.rds);
        Serial.println(radio_info.rssi);//
        Serial.println(radio_info.snr);
        Serial.println(radio_info.stereo);//
        Serial.println(radio_info.tuned);
        break;
      case SEARCH:
        if(direction == FORDWARD){
          Serial.println("direction");
          Serial.println(direction);
          radio.seekUp();
        } else {
          radio.seekDown();
          Serial.println(direction);
        }
        Serial.println(radio.getFrequency());
        break;
      case PRESET:
        radio.setFrequency(freq);
        break;
      default:
        break;
    }
  }
  dialScreen.update(value, &radio_info);
}

void readEncoder() {
  static int pos = 0;
  int newPos = encoder.getPosition();
  if (pos != newPos) {
    if (newPos > pos) {
      value = value - 2;
      if(value < 875){
        value = 1081;
      }
      direction = BACKWARD;
    }
    if (newPos < pos) {
      value = value + 2;
      if(value > 1081){
        value = 875;
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
    drawScreen();
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
  Serial.println(tuneType);
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
  Wire.begin(21, 22);
  radio.init();
  radio.debugEnable(false);
  radio.setBand(RADIO_BAND_FM);
  radio.setFrequency(9590);
  radio.getRadioInfo(&radio_info);
  radio.setMute(false);

  // leds[0] = CRGB::Red;
  // leds[1] = CRGB::White;
  // leds[2] = CRGB::Red;
  // leds[3] = CRGB::Green;
  // leds[4] = CRGB::Red;
  // leds[5] = CRGB::Blue;
  // leds[6] = CRGB::Red;
  // FastLED.show();
  dialScreen.update(value, &radio_info);
}

void loop() {
  if(encoder_changed == true){
    encoder_changed = false;
    Serial.println(value);
    drawScreen();
  }
  menu_button.tick();
  mute_button.tick();

}
