/*
 * BTN CLASS DEFINITION
 */
class Btn {
    public:
        uint32_t timeReleased = 0;
        uint32_t timePressed = 0;

        uint8_t pressCt = 0;

        bool isHeld = false;
        //no constructor

        //functions/methods
        void registerPress() {
            pressCt++;  // add a press
            timePressed = currentTime; // save the time
        }

        void registerRelease() {
            timePressed = 0; // reset
            timeReleased = currentTime; // save the time
        }

};

Btn btnC[] = {
    Btn(),
    Btn(),

    Btn(),
    Btn(),

    Btn(),
    Btn(),

    Btn(),
    Btn(),
};


/*
 * SECTION CLASS DEFINITION
 */

class Section {
    private:

    public:
        Btn *_btnC[2];  //pointer variable points to the btns
        uint8_t PIN;
        uint8_t mode = 0;
        uint32_t colorStartTime = 0;
        uint16_t colorDelayInt = 0;

        bool colorProgress = false;
        bool extendedFade = false;
        bool isOn = false;

        float masterLevel = 0;
        float RGBW[4] = {0, 0, 0, 0};
        bool RGBWon[4] = {false, false, false, false};
        
        uint8_t DMX_OUT;
        float BRIGHTNESS_FACTOR = 1;

        float lastMasterLevel = 0;

        float colorDelayCtr;
        uint8_t colorState = random(12);

        float lastRGBW[4] = {0, 0, 0, 0};
        float nextRGB[3] = {0, 0, 0};
        
        //public vars, constructor, methods
        Section(uint8_t dmx_out, Btn *_bot, Btn *_top, uint8_t pin, float brightnessFactor) { //constructor
            DMX_OUT = dmx_out;
            btnC[0] = *_bot;
            btnC[1] = *_top;
            PIN = pin;
            BRIGHTNESS_FACTOR = brightnessFactor;
            
            pinMode(pin, INPUT);
            pinMode(pin, INPUT);

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
    Section(4, &btnC[0], &btnC[1], ENTRY_BTN_PIN, 1.0), //store address of btns
    Section(3, &btnC[4], &btnC[5], KITCHEN_BTN_PIN, 1.35), //store address of btns
    Section(2, &btnC[2], &btnC[3], ENTRY2_BTN_PIN, 1.6), //store address of btns
    Section(1, &btnC[6], &btnC[7], BATH_BTN_PIN, 1), //store address of btns
};