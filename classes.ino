/*
 * BTN CLASS DEFINITION
 */
class Btn {
    private:
        uint8_t pinBtn;

        uint32_t timeReleased = 0;
        uint32_t timePressed = 0;

        uint8_t pressCt = 0;

        bool isHeld = false;

    public:
        //constructor
        Btn(uint8_t pinBtn) {
            this->pinBtn = pinBtn;

            pinMode(pinBtn, INPUT);
        }

        //functions/methods
        void registerPress(uint32_t t) { // takes the currentTime
            pressCt++;  // add a press
            timePressed = t; // save the time
            // section[i]._btn[b]->pressCt ++;
            // section[i]._btn[b]->timePressed = currentTime;
        }

        void registerRelease(uint32_t t) { // takes the currentTime
            timePressed = 0; // reset
            timeReleased = t; // save the time
            // section[i]._btn[b]->timePressed = 0;
            // section[i]._btn[b]->timeReleased = currentTime;
        }

};

Btn btnC[] = {
    Btn(ENTRY_BTN_PIN),
    Btn(ENTRY_BTN_PIN),

    Btn(KITCHEN_BTN_PIN),
    Btn(KITCHEN_BTN_PIN),

    Btn(ENTRY2_BTN_PIN),
    Btn(ENTRY2_BTN_PIN),

    Btn(BATH_BTN_PIN),
    Btn(BATH_BTN_PIN),
};


/*
 * SECTION CLASS DEFINITION
 */

class Section {
    private:
        //Btn *_btnC[2];
        uint8_t DMX_OUT;
        float BRIGHTNESS_FACTOR;

        bool isOn;
        bool colorProgress;
        bool extendedFade;

        float masterLevel;
        float lastMasterLevel;
        uint8_t mode;

        uint32_t colorStartTime;
        uint16_t colorDelayInt;
        float colorDelayCtr;
        uint8_t colorState;

        bool RGBWon[4];
        float RGBW[4];
        float lastRGBW[4];
        float nextRGB[3];

    public:
        //public vars, constructor, methods
        Section(uint8_t DMX_OUT) {   //, *Btn btn1, *Btn btn2) {
            DMX_OUT = DMX_OUT;
            // _btnC[0] = &btn1;
            // _btnC[1] = &btn2;
        }
        //methods for lights:
        //fade up/down
        //switch mode
        //updatelights
        //colorProgress
        //
        void updateLights(uint8_t i) { //might not need to take i but will for now
            if (DEBUG) {
                DEBUG_updateLights(i);

            } else {
                uint8_t brightnessValue = 0; // index for brightness lookup table

                for (uint8_t color = 0; color < 4; color++) {
                    if (RGBWon[color]) {

                        //brightnessValue = lookupTable(i, color);
                        
                        uint8_t height = (uint16_t(RGBW[color] * masterLevel * TABLE_SIZE) / HEIGHT);
                        uint8_t width = (uint16_t(RGBW[color] * masterLevel * TABLE_SIZE) % WIDTH);

                        // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
                        memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

                        if ((RGBW[color] > 0) && (masterLevel > 0) && (brightnessValue == 0)) 
                            brightnessValue = 1;
                        
                    } else {
                        //brightnessValue = 0;
                        // light is off, so turn it off
                    }

                    DmxSimple.write((DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
                    lastRGBW[color] = RGBW[color];
                }
            }
                
            if ( (RGBW[0] <= 0) && (RGBW[1] <= 0) && (RGBW[2] <= 0) && (RGBW[3] <= 0) ) {
                //switch to mode 0?

                isOn = false;
                masterLevel = 0;

                for (uint8_t color = 0; color < 4; color++) {  //clear rgbw
                    RGBW[color] = 0;
                    RGBWon[color] = false;
                }

                if (DEBUG) {
                    DEBUG_updateLightsOff(i);
                }
            }
        }
};

Section sectionC[] = {
    Section(4), //, &btnC[0], &btnC[1]),
    Section(3),
    Section(2),
    Section(1),
};