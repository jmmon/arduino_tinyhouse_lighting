/*
 * BTN CLASS DEFINITION
 */
class Btn {
    public:
        uint8_t pinBtn;

        uint32_t timeReleased = 0;
        uint32_t timePressed = 0;
        uint8_t pressCt = 0;
        bool isHeld = false;

        //constructor
        Btn(uint8_t pinBtn) {
            pinBtn = pinBtn;

            pinMode(pinBtn, INPUT);
        }

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
    Btn(ENTRY_BTN_PIN), //variables
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
        uint8_t DMX_OUT;
        float BRIGHTNESS_FACTOR = 1;

        float lastMasterLevel = 0;

        float colorDelayCtr;
        uint8_t colorState = random(12);

        float lastRGBW[4] = {0, 0, 0, 0};
        float nextRGB[3] = {0, 0, 0};

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
        //public vars, constructor, methods
        Section(uint8_t DMX_OUT, Btn *_bot, Btn *_top, uint8_t pin, float brightnessFactor) { //constructor
            DMX_OUT = DMX_OUT;
            btnC[0] = *_bot;
            btnC[1] = *_top;
            PIN = pin;
            BRIGHTNESS_FACTOR = brightnessFactor;

        }

        //methods for lights: (What can lights do?)
        //fade up/down
        //switch mode
        //update
        //colorProgress
        //
        
        void updateLights() { //might not need to take i but will for now
            if (DEBUG) {
                //uint8_t brightnessValue = lookupTable(ii, 3); //index for brightness lookup table
                uint8_t brightnessValue = 0; //index for brightness lookup table

                uint8_t height = (uint16_t(RGBW[3] * masterLevel * TABLE_SIZE) / HEIGHT);
                uint8_t width = (uint16_t(RGBW[3] * masterLevel * TABLE_SIZE) % WIDTH);

                // look up brighness from table and saves as temp ( sizeof(temp) resolves to 1 [byte of data])
                memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

                if ((RGBW[3] > 0) && (masterLevel > 0) && (brightnessValue == 0)) 
                    brightnessValue = 1;
                

                Serial.print(F("table_w:"));    Serial.print(width);
                Serial.print(F(", table_h:"));  Serial.print(height);
                
                Serial.print(F(" lvl:"));   Serial.print(brightnessValue);
                Serial.print(F(" W: "));    Serial.print(RGBW[3]);
                Serial.print(F(" R: "));    Serial.print(RGBW[0]);
                Serial.print(F(" G: "));    Serial.print(RGBW[1]);
                Serial.print(F(" B: "));    Serial.print(RGBW[2]);
                Serial.print(F(" Master level: ")); Serial.print(masterLevel);
                Serial.print(F(" cur_t:")); Serial.println(currentTime);

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
                    Serial.print(F("MasterBrightness: ")); Serial.println(masterLevel);
                    Serial.print(F("IsOn:")); Serial.println(isOn);
                }
            }
        }


        //***************************************************************************
        // Fade tick functions:
        void masterFadeIncrement(float f) {
            //master fade
            float temp = BRIGHTNESS_FACTOR * f;
            isOn = true;
            masterLevel += temp;
            if (masterLevel > 1.0) {
                masterLevel = 1.0; // max
            }
            updateLights();
        }


        void fadeIncrement(float f, uint8_t color) {
            float temp = BRIGHTNESS_FACTOR * f;
            // if (color == NULL) {
            //     //master fade
            //     isOn = true;
            //     masterLevel += temp;
            //     if (masterLevel > 1) {
            //         masterLevel = 1; // max
            //     }
            // } else {
                //regular fade
                RGBWon[color] = true;
                RGBW[color] += temp;
                if (RGBW[color] > 1) {
                    RGBW[color] = 1; // max
                }
            //}
            
            updateLights();
        }

        void masterFadeDecrement(float f) {
            //master fade
            float temp = BRIGHTNESS_FACTOR * f;
            masterLevel -= temp;
            if (masterLevel < 0) {
                isOn = false;
                masterLevel = 0; // min
            }
            updateLights();
        }

        void fadeDecrement(float f, uint8_t color) {
            float temp = BRIGHTNESS_FACTOR * f;
            // if (color == NULL) {
            //     //master fade
            //     masterLevel -= temp;
            //     if (masterLevel < 0) {
            //         isOn = false;
            //         masterLevel = 0; // min
            //     }
            // } else {
                //regular fade
                RGBW[color] -= temp;
                if (RGBW[color] < 0) {
                    RGBWon[color] = false;
                    RGBW[color] = 0; // min
                }
            //}
            
            updateLights();
        }


        //*************************************************************
        void enableExtendedFade() { // check if mode's regular level is max and if so enable extendedFade
            if (!(extendedFade)) {  // if extendedFade is not enabled yet for this section
                if (mode == LOW_CYCLE_STARTS_AT) { // white
                    if (masterLevel >= 1) {
                        extendedFade = true;
                    } // else hasn't hit max yet

                } else if (mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness 
                    // do nothing
                
                } else if (mode == (LOW_CYCLE_STARTS_AT + 1) || mode == (LOW_CYCLE_STARTS_AT + 2)) { // RGB Smooth/Sudden
                    if (masterLevel >= 1) {
                        extendedFade = true;
                    } // else hasn't hit max yet

                } else if (mode == HIGH_CYCLE_STARTS_AT || mode == (HIGH_CYCLE_STARTS_AT + 1) || mode == (HIGH_CYCLE_STARTS_AT + 2) ) { // RGB
                    if (RGBW[mode-SINGLE_COLOR_MODE_OFFSET] >= 1) {
                        extendedFade = true;
                    } // else hasn't hit max yet

                } else if ( mode == (HIGH_CYCLE_STARTS_AT + 3) ) { // Combined
                    if ( masterLevel >= 1 ) { // if masterLevel hits max, we enable extended fade
                        extendedFade = true;
                    } // else hasn't hit max yet
                }
            }
        }

        void disableExtendedFade() {    // check if mode's extended level is max and if so disable extendedFade
            if (extendedFade) {
                if (mode == LOW_CYCLE_STARTS_AT) { // white
                    if ( RGBW[0] <= 0 && RGBW[1] <= 0 && RGBW[2] <= 0 ) {
                        extendedFade = false; // if rgb are 0, disable extended fade
                    } // else hasn't hit min yet
                } else if (mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness 
                    // (no extended fade feature)
                } else if (mode == (LOW_CYCLE_STARTS_AT + 1) || mode == (LOW_CYCLE_STARTS_AT + 2) || mode == HIGH_CYCLE_STARTS_AT || mode == (HIGH_CYCLE_STARTS_AT + 1) || mode == (HIGH_CYCLE_STARTS_AT + 2) || mode == (HIGH_CYCLE_STARTS_AT + 3) ) { 
                    // Red // Green // Blue // Combined // RGB Smooth // RGB Sudden
                    if (RGBW[3] <= 0) { // white is extended fade, wait for this to be off
                        extendedFade = false;
                    } // else hasn't hit min yet
                }
            }
        }

        //***************************************************************************
        // color progress functions:

        void progressColorSudden() {
            if (colorDelayCtr < 1)
                colorDelayCtr += (2 * COLOR_LOOP_FADE_AMOUNT);

            else {
                colorDelayCtr = 0;

                colorState += 1;
                if (colorState == MAX_COLOR_STATE) 
                    colorState = 0;

                //colorState = random(MAX_COLOR_STATE);

                //set new light color
                RGBW[0] = RED_LIST[colorState];
                RGBW[1] = GREEN_LIST[colorState];
                RGBW[2] = BLUE_LIST[colorState];

                updateLights();

                if (DEBUG) {
                    Serial.print(F("color progress state: ")); 
                    Serial.println(colorState);
                }
            }
        }

        void progressColorSmooth()
        {
            nextRGB[0] = RED_LIST[colorState]; // target levels for the current state
            nextRGB[1] = GREEN_LIST[colorState];
            nextRGB[2] = BLUE_LIST[colorState];

            if ((RGBW[0] == nextRGB[0]) && (RGBW[1] == nextRGB[1]) && (RGBW[2] == nextRGB[2])) {

                colorState += 1;
                if (colorState == MAX_COLOR_STATE) 
                    colorState = 0;
                
                if (DEBUG) {
                    Serial.print(F("color progress state: ")); 
                    Serial.println(colorState);
                }

            } else { // else change colors to get closer to current state
                for (uint8_t color = 0; color < 3; color++) {

                    if (RGBW[color] < nextRGB[color]) {

                        RGBW[color] += COLOR_LOOP_FADE_AMOUNT;

                        if (RGBW[color] >= nextRGB[color])
                            RGBW[color] = nextRGB[color];
                        
                    }
                    else if (RGBW[color] > nextRGB[color]) {

                        RGBW[color] -= COLOR_LOOP_FADE_AMOUNT;

                        if (RGBW[color] <= nextRGB[color]) 
                            RGBW[color] = nextRGB[color];
                    }
                }
            }
            updateLights();
        }

        // ******************************************************************************

        void switchToOff() { // 0
            isOn = false; // if "on," set to "off"
            colorProgress = false;
            extendedFade = false;

            for (uint8_t color = 0; color < 4; color++) {
                RGBW[color] = 0; // and set values to 0 for each color for that light
                RGBWon[color] = false;
            }
            //save masterLevel so that next time it can be used as the starting point?
            lastMasterLevel = masterLevel;
            masterLevel = 0;
        }

        void switchToWhite() { // 1
            //wipe:
            colorProgress = false;
            for (uint8_t color = 0; color < 4; color++) {
                RGBW[color] = 0;
                RGBWon[color] = false;
            }
            
            RGBW[3] = 1;
            RGBWon[3] = true;
            masterLevel = BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
        }

        void switchToRGB() { // 2, 3
            if (!(colorProgress)) {
                //initialize colorProgress
                colorProgress = true;
                colorStartTime = currentTime;

                RGBW[3] = 0;
                RGBWon[3] = false;
                for (uint8_t color = 0; color < 3; color++) {
                    RGBWon[color] = true;
                }
                masterLevel = BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
            }

            colorState = random(MAX_COLOR_STATE);
            RGBW[0] = RED_LIST[colorState];
            RGBW[1] = GREEN_LIST[colorState];
            RGBW[2] = BLUE_LIST[colorState];

            if (mode == (LOW_CYCLE_STARTS_AT + 1)) {
                colorDelayInt = COLOR_LOOP_SMOOTH_DELAY_INT;

            } else { // mode == (LOW_CYCLE_STARTS_AT + 2)
                colorDelayInt = COLOR_LOOP_SUDDEN_DELAY_INT;
            }
        }

        void switchToMax() { // 4
            for (uint8_t color = 0; color < 4; color++) {
                RGBW[color] = 1;
                RGBWon[color] = true;
            }
                
            masterLevel = 1;
        }


        void switchToSingleColor() { // 252-254
            isOn = true;
            for (uint8_t color = 0; color < 3; color++) {
                RGBWon[color] = false;
                if (RGBW[color] == 0) { // give em some value for combined from double bottom press from red
                    RGBW[color] = BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS * 2 / 3;
                }
            }
            RGBWon[3] = false; // turn off white
            RGBW[3] = 0;
            // RGBW[mode-SINGLE_COLOR_MODE_OFFSET] = BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS * 2 / 3;
            RGBWon[mode-SINGLE_COLOR_MODE_OFFSET] = true;
            masterLevel = 1;
        }

        void switchToCombined() { // 255
            // fade with masterLevel, extend with white
            for (uint8_t color = 0; color < 3; color++) {
                RGBWon[color] = true;
            }
            // normalize brightness to sometimes expand range
            if ((RGBW[0] >= RGBW[1]) && (RGBW[0] >= RGBW[2])) {
                masterLevel = RGBW[0];  //0 is highest
                RGBW[1] = (RGBW[1] / RGBW[0]);
                RGBW[2] = (RGBW[2] / RGBW[0]);
                RGBW[0] = 1;
                
            } else if ((RGBW[1] >= RGBW[0]) && (RGBW[1] >= RGBW[2])) {
                masterLevel = RGBW[1];  //1 is highest
                RGBW[0] = (RGBW[0] / RGBW[1]);
                RGBW[2] = (RGBW[2] / RGBW[1]);
                RGBW[1] = 1;
            
            } else if ((RGBW[2] >= RGBW[0]) && (RGBW[2] >= RGBW[1])) {
                masterLevel = RGBW[2];  //2 is highest
                RGBW[0] = (RGBW[0] / RGBW[2]);
                RGBW[1] = (RGBW[1] / RGBW[2]);
                RGBW[2] = 1;
            }
        }

        void switchMode() {
            if (DEBUG) {
                Serial.print(F("Now in mode: "));
                Serial.println(mode);
            }

            isOn = true;
            
            switch (mode) {
                case (0): // off
                    switchToOff();
                    break;
                case (LOW_CYCLE_STARTS_AT): // white
                    switchToWhite();
                    break;
                case (LOW_CYCLE_STARTS_AT + 1): // RGB smoothn
                case (LOW_CYCLE_STARTS_AT + 2): // RGB sudden
                    switchToRGB();
                    break;
                case (LOW_CYCLE_STARTS_AT + 3):   //max brightness
                    switchToMax();
                    break;
                case (HIGH_CYCLE_STARTS_AT):        // r
                case (HIGH_CYCLE_STARTS_AT + 1):    // g
                case (HIGH_CYCLE_STARTS_AT + 2):    // b
                    switchToSingleColor();
                    break;
                case (HIGH_CYCLE_STARTS_AT + 3): // combined
                    switchToCombined();
                    break;
            }

            if ((mode != (LOW_CYCLE_STARTS_AT + 1)) && (mode != (LOW_CYCLE_STARTS_AT + 2))) { //if not rgb smooth/sudden 
                updateLights();
            }    
        }

};

Section sectionC[] = {
    Section(4, &btnC[0], &btnC[1], ENTRY_BTN_PIN, 1.0), //store address of btns
    Section(3, &btnC[4], &btnC[5], KITCHEN_BTN_PIN, 1.35), //store address of btns
    Section(2, &btnC[2], &btnC[3], ENTRY2_BTN_PIN, 1.6), //store address of btns
    Section(1, &btnC[6], &btnC[7], BATH_BTN_PIN, 1), //store address of btns
};





/*
 * OLD: STRUCT DEFINITIONS:
 */

struct btn_t {
    uint32_t timeReleased; //when was this button released?
    uint32_t timePressed;  //when was this button pressed?
    uint8_t pressCt;  //If button is pressed before timeReleased ends, add one to count
    bool isHeld;        //is the button being held? (for longer than BTN_FADE_DELAY

} btn[] = {
    // Inside underloft
    {0, 0, 0, false}, //entry button up         
    {0, 0, 0, false}, //entry button down

    // Outside (Porch)
    {0, 0, 0, false}, //entry2 button up
    {0, 0, 0, false}, //entry2 button down

    // Kitchen underloft
    {0, 0, 0, false}, //kitchen left wall
    {0, 0, 0, false}, //kitchen

    // Bathroom (back wall right nook)
    {0, 0, 0, false}, //bath
    {0, 0, 0, false},

    //Back wall button
    // {0, 0, 0, 0, false}, //Back wall (left) / Greenhouse?
    // {0, 0, 0, 0, false},

    //sconce 1 button close button
};




struct section_t {
    btn_t *_btn[2];
    uint8_t PIN;
    uint8_t DMX_OUT;          // DMX OUT number (set of 4 channels) for this section
    float BRIGHTNESS_FACTOR; // affects [default brightness + fade speed], pref range [0-1]

    bool isOn;              // Are any levels > 0?
    bool colorProgress;     // while true, colors for this section will cycle
    bool extendedFade;      // enable extended fade

    float masterLevel; // master brightness level
    float lastMasterLevel; // level from last time the light was on
    uint8_t mode; // 0-4: WW, colors, colors+ww, (All, Nightlight)
        // typical cycle is 0-1-2-0... modes 3 and 4 are hidden from the typical cycle.

    uint32_t colorStartTime;   // starting time of colorProgress
    uint16_t colorDelayInt;     // adjust this to change colorProgress speed
    float colorDelayCtr; // slows the color cycle, used to slow the "sudden" mode
    uint8_t colorState;        // next color state in the cycle

    bool RGBWon[4];     // is on? each color
    float RGBW[4];     // stores current RGBW color levels
    float lastRGBW[4]; // last color levels
    float nextRGB[3]; // next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.

} section[] = {
    //  ID 0    Living Room Lights
    {
        {&btn[0], &btn[1]},
        ENTRY_BTN_PIN, 4, 1.0, 
        false, false, false,
        1., 0., 0,
        0, 0, 0., 0, 
        {false, false, false, false},
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0., 0., 0.}, 
    },

    //  ID 1    Kitchen Lights
    {
        {&btn[4], &btn[5]},
        KITCHEN_BTN_PIN, 3, 1.35, 
        false, false, false,
        1., 0., 0,
        0, 0, 0., 0, 
        {false, false, false, false},
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0, 0, 0}, 
    },

    //  ID 2   Porch Lights
    {
        {&btn[2], &btn[3]},
        ENTRY2_BTN_PIN, 2, 1.6, 
        false, false, false,
        1., 0., 0, 
        0, 0, 0., 0, 
        {false, false, false, false},
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0, 0, 0}, 
    },

    //  ID 3   bath
    {
        {&btn[6], &btn[7]},
        BATH_BTN_PIN, 1, 1.0, 
        false, false, false,
        1., 0., 0,
        0, 0, 0., 0, 
        {false, false, false, false},
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0, 0, 0}, 
    },

    //  ID     Overhead Bedroom
    //  ID     Overhead Main
    //  ID     Overhead Small Loft
    //  ID     Greenhouse Lights
};

