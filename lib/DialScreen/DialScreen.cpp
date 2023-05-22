#include "DialScreen.h"

DialScreen::DialScreen(TFT_eSPI* tft) {
    this->muted = false;
    this->spr = new TFT_eSprite(tft);
    this->spr->createSprite(240, 135);
    this->spr->setSwapBytes(true);
    this->spr->setFreeFont(&Orbitron_Light_24);
    this->spr->setTextColor(color1, TFT_BLACK);
    this->tuneType = MANUAL;
}

DialScreen::~DialScreen() {
}

void DialScreen::update(int freq, RADIO_INFO *radio_info){
    this->spr->fillSprite(TFT_BLACK);
    this->spr->setTextColor(TFT_WHITE, TFT_BLACK);

    this->drawDial(freq);
    this->drawFM();
    this->drawTuneType();
    this->drawMuted();
    this->drawSignal(radio_info->rssi);
    this->drawStereo(radio_info->stereo);

    this->spr->pushSprite(0, 0);
};

void DialScreen::drawDial(int freq) {
    this->spr->setTextDatum(CC_DATUM);
    this->frequency = freq;
    this->spr->drawFloat(this->frequency / 100.0, 1, SCREEN_WIDTH / 2 - 10, 54, 7); // frecuencia actual
    int temp = this->frequency / 10 - 15;
    for (int i = 0; i < 30; i++){
        if ((temp % 10) == 0) {
            spr->drawLine(i * 8, SCREEN_HEIGHT, i * 8, SCREEN_HEIGHT - LARGE_LINE_HEIGHT, color1);
            spr->drawLine((i * 8) + 1, SCREEN_HEIGHT, (i * 8) + 1, SCREEN_HEIGHT - LARGE_LINE_HEIGHT, color1);
            spr->drawFloat(temp / 10.0, 1, i * 8, SCREEN_HEIGHT - LARGE_LINE_HEIGHT - 10, 2);
        } else if ((temp % 5) == 0 && (temp % 10) != 0) {
            this->spr->drawLine(i * 8, SCREEN_HEIGHT, i * 8, SCREEN_HEIGHT - MEDIUM_LINE_HEIGHT, color1);
            this->spr->drawLine((i * 8) + 1, SCREEN_HEIGHT, (i * 8) + 1, SCREEN_HEIGHT - MEDIUM_LINE_HEIGHT, color1);
        } else {
            this->spr->drawLine(i * 8, SCREEN_HEIGHT, i * 8, SCREEN_HEIGHT - SHORT_LINE_HEIGHT, color1);
        }
        temp = temp + 1;
    }
    const int triangle_height = 10;
    const int triangle_width = 8;
    const int triangle_line = 40;
    this->spr->fillTriangle(SCREEN_WIDTH / 2  - triangle_width/2, SCREEN_HEIGHT - triangle_line - triangle_height, SCREEN_WIDTH / 2, SCREEN_HEIGHT - triangle_line, SCREEN_WIDTH / 2 + triangle_width/2, SCREEN_HEIGHT - triangle_line - triangle_height, TFT_RED);
    this->spr->drawLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT - triangle_line, SCREEN_WIDTH / 2, SCREEN_HEIGHT, TFT_RED);
}

void DialScreen::drawFM() {
    this->spr->setTextDatum(CC_DATUM);
    this->spr->setFreeFont(&Orbitron_Light_24);
    this->spr->drawString("FM", 25, 54);
}

void DialScreen::drawTuneType() {
  String text = "";
    this->spr->setTextDatum(CC_DATUM);
    this->spr->drawRoundRect(180, 2, 58, 20, 4, TFT_WHITE);
    switch (this->tuneType){
    case MANUAL:
      text = "Manual"; 
      break;
    case SEARCH:
      text = "Seek"; 
      break;
    case PRESET:
      text = "Pre"; 
      break;
    default:
      text = ""; 
      break;
    }
    this->spr->drawString(text, 215, 12, 2); 
}

void DialScreen::drawMuted() {
    if (this->muted == true){
        this->spr->setTextDatum(CC_DATUM);
        this->spr->drawString("MUTED", 210, 54, 2);
    }
  //  this->spr->fillRoundRect(53, 30, 20, 20, 3, 0xCC40);
  // if (muted == true){
  //    this->spr->fillCircle(62, 40, 6, TFT_WHITE);
  // }
}

void DialScreen::drawSignal(uint8_t strength){
  for (int i = 0; i < strength; i++){
      this->spr->fillRect(3 + (i * 4), 15 - (i * 1), 2, 4 + (i * 1), 0x3526);
  }
}

void DialScreen::drawStereo(bool isStereo){
  if(isStereo == true){
    this->spr->setTextColor(TFT_WHITE, TFT_BLACK);
    this->spr->setTextDatum(TL_DATUM);
    this->spr->drawString("Stereo", 6, 30, 2);
  }
}

void DialScreen::setMute(bool mute){
    this->muted = mute;
};

void DialScreen::setTuneType(TuneType tuneType){
  this->tuneType = tuneType;
}

/* ESTACIONES GUARDADAS
  spr.drawString("Guardado", 38, 14, 2);
  spr.drawRoundRect(1, 1, 76, 110, 4, 0xAD55); //rectangulo de estaciones guardadas
  spr.setTextFont(0);
  spr.setTextColor(0xBEDF, TFT_BLACK);
  for (int i = 0; i < 6; i++) {
    spr.drawString(sta[i], 38, 32 + (i * 12));
    spr.fillCircle(16, 31 + (i * 12), 2, 0xFBAE);
  }
*/

/* PILA
  spr.drawRect(112, 6, 20, 9, TFT_WHITE); //rectangulo base
  spr.fillRect(113, 7, 12, 7, 0x34CD); //rectangulo lleno verde
  spr.fillRect(132, 8, 2, 5, TFT_WHITE); //punta de la pila
*/
