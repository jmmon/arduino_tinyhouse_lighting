/*
 * BTN CLASS DEFINITION
 */
class Btn_C {
    private:
    public:
        uint32_t timeReleased = 0;
        uint32_t timePressed = 0;
        uint8_t pressCtr = 0;
        bool isHeld = false;
        //isHeld if time passes heldLength req and timePressed != 0

        bool checkIsHeld() { //not yet used
            return ((timePressed != 0) && (currentTime > (timePressed + BTN_HELD_DELAY)));
        }
        
        //functions/methods
        void registerPress() {
            pressCtr++;  // add a press
            timePressed = currentTime; // save the time
        }

        void registerRelease() {
            timePressed = 0; // reset depressed timer
            timeReleased = currentTime; // save the time
        }

};


class Btn_Top : public Btn_C {
    private:
    public:
        
};
class Btn_Bot : public Btn_C {
    private:
    public:
};

// /*
//  * SECTION CLASS DEFINITION
//  */

// /*
class Section { // rebuild of other class? Not sure why class isn't working
    public:
        Btn_C btn[2];
        uint8_t PIN;
        uint8_t DMX_OUT;          // DMX OUT number (set of 4 channels) for this section
        float LVL_ADJUST; // affects [default brightness + fade speed], pref range [0-1]
    
        bool isOn = false;              // Are any levels > 0?
        bool colorProgress = false;     // while true, colors for this section will cycle
        bool fadeExtended = false;      // enable extended fade
    
        float masterLevel = 1.0; // master brightness level
        float lastMasterLevel = 0.0; // level from last time the light was on
        uint8_t mode = 0; // 0-4: WW, colors, colors+ww, (All, Nightlight)
            // typical cycle is 0-1-2-0... modes 3 and 4 are hidden from the typical cycle.
    
        uint32_t colorStartTime = 0;   // starting time of colorProgress
        uint16_t colorDelayInt = 0;     // adjust this to change colorProgress speed
        float colorDelayCtr = 0; // slows the color cycle, used to slow the "sudden" mode
        uint8_t colorState = 0;        // next color state in the cycle
    
        bool RGBWon[4] = {false, false, false, false};     // is on? each color
        //bool lastRGBW[4] = {false, false, false, false};
        float RGBW[4] = {0., 0., 0., 0.};     // stores current RGBW color levels
        float lastRGBW[4] = {0., 0., 0., 0.}; // last color levels
        float nextRGB[3] = {0., 0., 0.}; // next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.
        
        Section(uint8_t pin, uint8_t dmx_out, float bf) { // constructor
            btn[0] = Btn_Bot();
            btn[1] = Btn_Top();
            PIN = pin;
            pinMode(pin, INPUT);
            DMX_OUT = dmx_out;
            LVL_ADJUST = bf;
        };




        // methods for lights:

        void saveColor() {
            lastMasterLevel = masterLevel;
            for (uint8_t color = 0; color < 4; color++) {
                lastRGBW[color] = RGBW[color];
                //lastRGBWon[color] = RGBWon[color]; //if needed
            }
        }

        void turnOff(uint8_t color = 5) {
            if (color == 5) { //just master (default)
                isOn = false;
                masterLevel = 0;

            } else if (color == 4) { //all 4 colors
                for (uint8_t z = 0; z < 4; z++) {
                    RGBW[z] = 0;
                    RGBWon[z] = false;
                }

            } else { ///individual color
                RGBW[color] = 0;
                RGBWon[color] = false;
            }
        }

        void turnOn(uint8_t color = 4) {
            isOn = true;
            RGBW[color] = 1;
            RGBWon[color] = true;
        }


        void update() { // updates this light section
            uint8_t brightnessValue = 0; //index for brightness lookup table
            if (DEBUG) {
                uint8_t height = (uint16_t(RGBW[3] * masterLevel * TABLE_SIZE) / HEIGHT);
                uint8_t width = (uint16_t(RGBW[3] * masterLevel * TABLE_SIZE) % WIDTH);
                // look up brighness from table and saves as temp ( sizeof(temp) resolves to 1 [byte of data])
                memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 
                if ((RGBW[3] > 0) && (masterLevel > 0) && (brightnessValue == 0)) 
                    brightnessValue = 1;
                
                Serial.print(F(" t:")); Serial.println(currentTime); Serial.print(F(" lvl:"));   Serial.print(brightnessValue);
                Serial.print(F("  R: ")); Serial.print(RGBW[0]); Serial.print((RGBWon[0]) ? F("(ON)") : F("(OFF)"));
                Serial.print(F(" G: ")); Serial.print(RGBW[1]); Serial.print((RGBWon[1]) ? F("(ON)") : F("(OFF)"));
                Serial.print(F(" B: ")); Serial.print(RGBW[2]); Serial.print((RGBWon[2]) ? F("(ON)") : F("(OFF)"));
                Serial.print(F(" W: ")); Serial.print(RGBW[3]); Serial.print((RGBWon[3]) ? F("(ON)") : F("(OFF)"));
                Serial.print(F(" masterLevel: ")); Serial.print(masterLevel); Serial.print((isOn) ? F("(ON)") : F("(OFF)"));

            } else { // regular update
                for (uint8_t color = 0; color < 4; color++) {
                    if (RGBWon[color]) {                
                        uint8_t height = (uint16_t(RGBW[color] * masterLevel * TABLE_SIZE) / HEIGHT);
                        uint8_t width = (uint16_t(RGBW[color] * masterLevel * TABLE_SIZE) % WIDTH);
                        // (&savesAsVar, lookUpInTable[height][width], sizeof(var) aka byte)
                        memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 
                        if ((RGBW[color] > 0) && (masterLevel > 0) && (brightnessValue == 0)) brightnessValue = 1;
                    } // else light is off, so turn it off

                    DmxSimple.write((DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
                }
                saveColor();
            }
                
            if ( (RGBW[0] <= 0) && (RGBW[1] <= 0) && (RGBW[2] <= 0) && (RGBW[3] <= 0) ) {
                turnOff(4); ///turn off  colors
                turnOff();  //turn off maaster

                if (DEBUG) {
                    Serial.print(F("MasterBrightness: ")); Serial.println(masterLevel); Serial.print((isOn) ? F("(ON)") : F("(OFF)"));
                }
            }
        }

        //***************************************************************************
        // color progress functions:
        //***************************************************************************

        void progressColorSmooth() {
            for (uint8_t color = 0; color < 3; color++) {  // Progmem version:
                float value = 0;
                memcpy_P(&value, &(COLOR_PROGRESS_LIST[color][colorState]), sizeof(value)); 
                nextRGB[color] = value;
            }

            if ((RGBW[0] == nextRGB[0]) && (RGBW[1] == nextRGB[1]) && (RGBW[2] == nextRGB[2])) {
                colorState += 1;
                if (colorState == MAX_COLOR_STATE) 
                    colorState = 0;
                if (DEBUG) {
                    Serial.print(F("color progress state: ")); Serial.println(colorState);
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
            update();
        }

        void progressColorSudden() {
            if (colorDelayCtr < 1)
                colorDelayCtr += (2 * COLOR_LOOP_FADE_AMOUNT);

            else {
                colorDelayCtr = 0;
                colorState += 1;
                if (colorState == MAX_COLOR_STATE) 
                    colorState = 0;

                //colorState = random(MAX_COLOR_STATE);
                for (uint8_t color = 0; color < 3; color++) {
                    float value = 0;
                    memcpy_P(&value, &(COLOR_PROGRESS_LIST[color][colorState]), sizeof(value)); 
                    // set new light color
                    RGBW[color] = value;
                }

                update();

                if (DEBUG) {
                    Serial.print(F("color progress state: ")); Serial.println(colorState);
                }
            }
        }



        //***************************************************************************
        void switchMode() {
            if (DEBUG) {
                Serial.print(F("Now in mode: ")); Serial.println(mode);
            }


            switch (mode) {
                case (0): // off
                        colorProgress = false;
                        fadeExtended = false;
                        saveColor();
                        turnOff(4); //colors
                        turnOff(); // master
                    break;
                case (LOW_CYCLE_STARTS_AT): // white
                        colorProgress = false;
                        turnOff(4); //colors
                        turnOff(); // master
                        turnOn(3);
                        masterLevel = LVL_ADJUST * DEFAULT_BRIGHTNESS;
                    break;

                case (LOW_CYCLE_STARTS_AT + 1): // RGB smoothn
                case (LOW_CYCLE_STARTS_AT + 2): // RGB sudden
                        if (!(colorProgress)) { // initialize colorProgress
                            colorProgress = true;
                            colorStartTime = currentTime;
                            turnOff(3);
                            for (uint8_t color = 0; color < 3; color++) {
                                turnOn(color);
                            }
                            masterLevel = LVL_ADJUST * DEFAULT_BRIGHTNESS;
                        }
                        colorState = random(MAX_COLOR_STATE);

                        for (uint8_t color = 0; color < 3; color++) {
                            float value = 0;
                            memcpy_P(&value, &(COLOR_PROGRESS_LIST[color][colorState]), sizeof(value)); 
                            RGBW[color] = value;
                        }

                        colorDelayInt = (mode == (LOW_CYCLE_STARTS_AT + 1)) ? 
                            COLOR_LOOP_SMOOTH_DELAY_INT: 
                            COLOR_LOOP_SUDDEN_DELAY_INT;
                    break;

                case (LOW_CYCLE_STARTS_AT + 3):   //max brightness
                        for (uint8_t color = 0; color < 4; color++) {
                           turnOn(color);
                        }
                        masterLevel = 1;
                    break;

                case (HIGH_CYCLE_STARTS_AT):        // r
                case (HIGH_CYCLE_STARTS_AT + 1):    // g
                case (HIGH_CYCLE_STARTS_AT + 2):    // b
                        isOn = true;
                        for (uint8_t color = 0; color < 3; color++) {
                            RGBWon[color] = false;
                            if (RGBW[color] == 0) { // give em some value for combined from double bottom press from red
                                RGBW[color] = LVL_ADJUST * DEFAULT_BRIGHTNESS * 2 / 3;
                            }
                        }
                        turnOff(3); // white
                        RGBWon[mode - SINGLE_COLOR_MODE_OFFSET] = true;
                        masterLevel = 1;
                    break;

                case (HIGH_CYCLE_STARTS_AT + 3): // combined
                        // fade with masterLevel, extend with white
                        isOn = true;
                        for (uint8_t color = 0; color < 3; color++) {
                            RGBWon[color] = true;
                        }
                        // normalize brightness to sometimes expand range
                        if ((RGBW[0] >= RGBW[1]) && (RGBW[0] >= RGBW[2])) {
                            masterLevel = RGBW[0];  //0 is highest
                            RGBW[1] /= RGBW[0];
                            RGBW[2] /= RGBW[0];
                            RGBW[0] = 1;
                            
                        } else if ((RGBW[1] >= RGBW[0]) && (RGBW[1] >= RGBW[2])) {
                            masterLevel = RGBW[1];  //1 is highest
                            RGBW[0] /= RGBW[1];
                            RGBW[2] /= RGBW[1];
                            RGBW[1] = 1;
                        
                        } else if ((RGBW[2] >= RGBW[0]) && (RGBW[2] >= RGBW[1])) {
                            masterLevel = RGBW[2];  //2 is highest
                            RGBW[0] /= RGBW[2];
                            RGBW[1] /= RGBW[2];
                            RGBW[2] = 1;
                        }
                    break;
            }

            if ((mode != (LOW_CYCLE_STARTS_AT + 1)) && (mode != (LOW_CYCLE_STARTS_AT + 2))) { //if not rgb smooth/sudden 
                update();
            }    
        }

        void colorInc(uint8_t f, uint8_t color = 4) {
            float temp = LVL_ADJUST * FADE_AMOUNT * f;
            if (!isOn) isOn = true;

            if (color == 4) { ///master fade
                masterLevel += temp;
                if (masterLevel > 1.0) masterLevel = 1.0; // max

            } else { ////color fade
                if (!RGBWon[color]) RGBWon[color] = true;
                RGBW[color] += temp;
                if (RGBW[color] > 1.0) RGBW[color] = 1.0; // max
            }
            update();
        }

        void colorDec(uint8_t f, uint8_t color = 4) {
            float temp = LVL_ADJUST * FADE_AMOUNT * f;
            if (color == 4) { //master
                masterLevel -= temp;
                if (masterLevel <= 0) {
                    turnOff(); //master off
                }

            } else { //reg
                RGBW[color] -= temp;
                if (RGBW[color] <= 0) {
                    turnOff(color);
                }
            }
            update();
        }

        void enableExtFade() { // check if mode's regular level is max and if so enable fadeExtended
            if (!(fadeExtended)) {
                switch(mode) {
                    case(LOW_CYCLE_STARTS_AT): // white         extends with RGB
                    case(LOW_CYCLE_STARTS_AT + 1):                     // extends with W
                    case(LOW_CYCLE_STARTS_AT + 2):// RGB Smooth // RGB Sudden extends with W
                    case(HIGH_CYCLE_STARTS_AT + 3):// combined              extends with W
                        if ( masterLevel >= 1 ) fadeExtended = true;
                        //these colors use 
                        break;

                    case(HIGH_CYCLE_STARTS_AT):                         // extends with w
                    case(HIGH_CYCLE_STARTS_AT + 1):                     // extends with w
                    case(HIGH_CYCLE_STARTS_AT + 2): // Red // Green // Blue extends with w
                        if (RGBW[mode-SINGLE_COLOR_MODE_OFFSET] >= 1)  fadeExtended = true;
                        break;
                }
            }
        }

        void DisableExtFade() { // check if mode's extended level is max and if so disable fadeExtended
            if (fadeExtended) {
                switch(mode) {
                    case(LOW_CYCLE_STARTS_AT): // white
                        if ( RGBW[0] <= 0 && RGBW[1] <= 0 && RGBW[2] <= 0 )  fadeExtended = false;
                        break;

                    case(LOW_CYCLE_STARTS_AT + 1):
                    case(LOW_CYCLE_STARTS_AT + 2): // RGB Smooth // RGB Sudden
                    case(HIGH_CYCLE_STARTS_AT):
                    case(HIGH_CYCLE_STARTS_AT + 1):
                    case(HIGH_CYCLE_STARTS_AT + 2):
                    case(HIGH_CYCLE_STARTS_AT + 3): // Red // Green // Blue // Combined
                        if (RGBW[3] <= 0) fadeExtended = false;
                        break;
                }
            }
        }









        void btnTopHeld1p() {
            if (!(isOn)) {  // if fading up from off, turn "on" the correct light
                isOn = true;
                if (mode == 0) {
                    mode = LOW_CYCLE_STARTS_AT;
                    turnOn(3);// white on
                    masterLevel = 0.001; //fixes fade up from off
                } 
                
                else if (mode == HIGH_CYCLE_STARTS_AT || mode == (HIGH_CYCLE_STARTS_AT + 1) || mode == (HIGH_CYCLE_STARTS_AT + 2)) {
                    RGBWon[mode-SINGLE_COLOR_MODE_OFFSET] = true;
                }
            }
            uint8_t factor = (currentTime >= (btn[1].timePressed + (BTN_HELD_DELAY * 3))) ? 4 :
                            (currentTime >= (btn[1].timePressed + (BTN_HELD_DELAY * 2))) ? 2 :
                            1;

            if (mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
                colorInc(factor);

            } else if (fadeExtended) {
                if (mode == LOW_CYCLE_STARTS_AT) { // white
                    for (uint8_t color = 0; color < 3; color++) {
                        colorInc(factor, color); // rgb
                    }
                } else {
                    colorInc(factor, 3);
                }

            } else { // regular fade
                if (mode == LOW_CYCLE_STARTS_AT || mode == (LOW_CYCLE_STARTS_AT + 1) || mode == (LOW_CYCLE_STARTS_AT + 2) || mode == (HIGH_CYCLE_STARTS_AT + 3)) { // white, rgb smooth, sudden;  combined
                    colorInc(factor);

                } else if (mode == HIGH_CYCLE_STARTS_AT || mode == (HIGH_CYCLE_STARTS_AT + 1) || mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
                    colorInc(factor, mode-SINGLE_COLOR_MODE_OFFSET);
                }
            }
        }

        
} section[] = {
    Section(ENTRY_BTN_PIN, 4, 1.0), // under main loft
    Section(KITCHEN_BTN_PIN, 3,  1.32), // kitchen
    Section(ENTRY2_BTN_PIN, 2,  1.6), // porch
    Section(BATH_BTN_PIN, 1, 1.0), // bath TODO

    //  ID     Overhead Bedroom
    //  ID     Overhead Main
    //  ID     Overhead Small Loft
    //  ID     Greenhouse Lights
};