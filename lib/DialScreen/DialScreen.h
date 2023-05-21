#ifndef DialScreen_h
#define DialScreen_h
#include "TFT_eSPI.h"
#include "radio.h"

#define SCREEN_HEIGHT 135
#define SCREEN_WIDTH  240

#define SHORT_LINE_HEIGHT   10
#define MEDIUM_LINE_HEIGHT  15
#define LARGE_LINE_HEIGHT   21
#define color1 0xC638
#define color2 0xC638

enum TuneType {
  MANUAL = 0,
  SEARCH = 1,
  PRESET = 2
};
class DialScreen {
    private:
        TFT_eSprite* spr;
        TuneType tuneType;
        float frequency;
        void drawDial(int value);
        void drawFM();
        void drawTuneType();
        void drawMuted();
        void drawSignal(uint8_t strength);
        void drawStereo(bool isStereo);
        bool muted;

    public:
        DialScreen(TFT_eSPI* tft);
        ~DialScreen();
        void update(int value, RADIO_INFO *radio_info);
        void setMute(bool mute);
        void setTuneType(TuneType tuneType);
};

#endif