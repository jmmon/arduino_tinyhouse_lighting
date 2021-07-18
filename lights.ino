const float DEFAULT_BRIGHTNESS = 0.60; // 0-1 (percent) value for default brightness when turned on
const uint8_t COLOR_LOOP_SMOOTH_DELAY_INT = 20; //ms
const uint16_t COLOR_LOOP_SUDDEN_DELAY_INT = 3000; //ms
const float COLOR_LOOP_FADE_AMOUNT = 0.001;  // brightness adjust amount per tick

const float _HIGH = 0.95,
             _MID =  0.85,
             _LOW = 0.60;
const uint8_t MAX_COLOR_STATE = 12;
const float RED_LIST[MAX_COLOR_STATE] =   {1., _HIGH,  _MID, _LOW,     0, 0,       0, 0,           0, _LOW,    _MID, _HIGH },
            GREEN_LIST[MAX_COLOR_STATE] = {0, _LOW,    _MID, _HIGH,    1., _HIGH,  _MID, _LOW,     0, 0,       0, 0        },
            BLUE_LIST[MAX_COLOR_STATE] =  {0, 0,       0, 0,           0, _LOW,    _MID, _HIGH,    1., _HIGH,  _MID, _LOW};
// const uint8_t MAX_COLOR_STATE = 6;
// const float RED_LIST[] = {1., _MID, 0, 0, 0, _MID};
// const float GREEN_LIST[] = {0, _MID, 1., _MID, 0, 0};
// const float BLUE_LIST[] = {0, 0, 0, _MID, 1., _MID};

//*************************************************************
void extendedFade(uint8_t ii) {
    if (!(sectionC[ii].extendedFade)) {  // if extendedFade is not enabled yet for this section
        //check if mode's regular level is max and if so enable extendedFade
        if (sectionC[ii].mode == LOW_CYCLE_STARTS_AT) { // white 
            // if (sectionC[ii].RGBW[3] >= 1) {
            if (sectionC[ii].masterLevel >= 1) {
                sectionC[ii].extendedFade = true;
            } // else hasn't hit max yet

        // } else if (sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness 
        //     // do nothing

        } else if (sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 2)) {
                // RGB Smooth // RGB Sudden
            if (sectionC[ii].masterLevel >= 1) {
                sectionC[ii].extendedFade = true;
            } // else hasn't hit max yet

        } else if (sectionC[ii].mode == HIGH_CYCLE_STARTS_AT || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 2) ) { // Red // Green // Blue
            if (sectionC[ii].RGBW[sectionC[ii].mode-SINGLE_COLOR_MODE_OFFSET] >= 1) {
                sectionC[ii].extendedFade = true;
            } // else hasn't hit max yet

        } else if ( sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 3) ) { // Combined
            //if fading up and any of these hit max, we're at max. So enable extended fade
            if ( sectionC[ii].masterLevel >= 1 ) {
                sectionC[ii].extendedFade = true;
            } // else hasn't hit max yet
        }
    }
}

void disableExtendedFade(uint8_t ii) {
    if (sectionC[ii].extendedFade) {
        //check if mode's extended level is max and if so disable extendedFade
        if (sectionC[ii].mode == LOW_CYCLE_STARTS_AT) { // white 
            // if rgb are 0, disable extended fade
            if ( sectionC[ii].RGBW[0] <= 0 && sectionC[ii].RGBW[1] <= 0 && sectionC[ii].RGBW[2] <= 0 ) {
                sectionC[ii].extendedFade = false;
            } // else hasn't hit min yet
        } else if (sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness 
            //do nothing
        } else if (sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || sectionC[ii].mode == HIGH_CYCLE_STARTS_AT || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 2) || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 3) ) { 
            // Red // Green // Blue // Combined // RGB Smooth // RGB Sudden
            // white is extended fade, wait for this to be off
            if (sectionC[ii].RGBW[3] <= 0) {
                sectionC[ii].extendedFade = false;
            } // else hasn't hit min yet
        }
    }
}

//***************************************************************************
// Fade tick functions:
void masterFadeIncrement(uint8_t i, float f) {
    //master fade
    float temp = sectionC[i].BRIGHTNESS_FACTOR * f;
    sectionC[i].isOn = true;
    sectionC[i].masterLevel += temp;
    if (sectionC[i].masterLevel > 1.0) {
        sectionC[i].masterLevel = 1.0; // max
    }
    updateLights(i);
}


void fadeIncrement(uint8_t i, float f, uint8_t color) {
    float temp = sectionC[i].BRIGHTNESS_FACTOR * f;
    // if (color == NULL) {
    //     //master fade
    //     sectionC[i].isOn = true;
    //     sectionC[i].masterLevel += temp;
    //     if (sectionC[i].masterLevel > 1) {
    //         sectionC[i].masterLevel = 1; // max
    //     }
    // } else {
        //regular fade
        sectionC[i].RGBWon[color] = true;
        sectionC[i].RGBW[color] += temp;
        if (sectionC[i].RGBW[color] > 1) {
            sectionC[i].RGBW[color] = 1; // max
        }
    //}
    
    updateLights(i);
}

void masterFadeDecrement(uint8_t i, float f) {
    //master fade
    float temp = sectionC[i].BRIGHTNESS_FACTOR * f;
    sectionC[i].masterLevel -= temp;
    if (sectionC[i].masterLevel < 0) {
        sectionC[i].isOn = false;
        sectionC[i].masterLevel = 0; // min
    }
    updateLights(i);
}

void fadeDecrement(uint8_t i, float f, uint8_t color) {
    float temp = sectionC[i].BRIGHTNESS_FACTOR * f;
    // if (color == NULL) {
    //     //master fade
    //     sectionC[i].masterLevel -= temp;
    //     if (sectionC[i].masterLevel < 0) {
    //         sectionC[i].isOn = false;
    //         sectionC[i].masterLevel = 0; // min
    //     }
    // } else {
        //regular fade
        sectionC[i].RGBW[color] -= temp;
        if (sectionC[i].RGBW[color] < 0) {
            sectionC[i].RGBWon[color] = false;
            sectionC[i].RGBW[color] = 0; // min
        }
    //}
    
    updateLights(i);
}


//**********************************************************************

void switchMode(uint8_t nn) {
    if (DEBUG) {
        Serial.print(F("Now in mode: "));
        Serial.println(sectionC[nn].mode);
    }

    sectionC[nn].isOn = true;

    switch (sectionC[nn].mode) {
        case (0): // off
            switchToOff(nn);
            break;
        case (LOW_CYCLE_STARTS_AT): // white
            switchToWhite(nn);
            break;
        case (LOW_CYCLE_STARTS_AT + 1): // RGB smoothn
        case (LOW_CYCLE_STARTS_AT + 2): // RGB sudden
            switchToRGB(nn);
            break;
        case (LOW_CYCLE_STARTS_AT + 3):   //max brightness
            switchToMax(nn);
            break;
        case (HIGH_CYCLE_STARTS_AT):        // r
        case (HIGH_CYCLE_STARTS_AT + 1):    // g
        case (HIGH_CYCLE_STARTS_AT + 2):    // b
            switchToSingleColor(nn);
            break;
        case (HIGH_CYCLE_STARTS_AT + 3): // combined
            switchToCombined(nn);
            break;
    }

    if ((sectionC[nn].mode != (LOW_CYCLE_STARTS_AT + 1)) && (sectionC[nn].mode != (LOW_CYCLE_STARTS_AT + 2))) { //if not rgb smooth/sudden 
        updateLights(nn);
    }    
}

void switchToOff(uint8_t nn) { // 0
    sectionC[nn].isOn = false; // if "on," set to "off"
    sectionC[nn].colorProgress = false;
    sectionC[nn].extendedFade = false;

    for (uint8_t color = 0; color < 4; color++) {
        sectionC[nn].RGBW[color] = 0; // and set values to 0 for each color for that light
        sectionC[nn].RGBWon[color] = false;
    }
    //save masterLevel so that next time it can be used as the starting point?
    sectionC[nn].lastMasterLevel = sectionC[nn].masterLevel;
    sectionC[nn].masterLevel = 0;
}

void switchToWhite(uint8_t nn) { // 1
    //wipe:
    sectionC[nn].colorProgress = false;
    for (uint8_t color = 0; color < 4; color++) {
        sectionC[nn].RGBW[color] = 0;
        sectionC[nn].RGBWon[color] = false;
    }
    
    sectionC[nn].RGBW[3] = 1;
    sectionC[nn].RGBWon[3] = true;
    sectionC[nn].masterLevel = sectionC[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
}

void switchToRGB(uint8_t nn) { // 2, 3
    if (!(sectionC[nn].colorProgress)) {
        //initialize colorProgress
        sectionC[nn].colorProgress = true;
        sectionC[nn].colorStartTime = currentTime;

        sectionC[nn].RGBW[3] = 0;
        sectionC[nn].RGBWon[3] = false;
        for (uint8_t color = 0; color < 3; color++) {
            sectionC[nn].RGBWon[color] = true;
        }
        sectionC[nn].masterLevel = sectionC[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
    }

    sectionC[nn].colorState = random(MAX_COLOR_STATE);
    sectionC[nn].RGBW[0] = RED_LIST[sectionC[nn].colorState];
    sectionC[nn].RGBW[1] = GREEN_LIST[sectionC[nn].colorState];
    sectionC[nn].RGBW[2] = BLUE_LIST[sectionC[nn].colorState];

    if (sectionC[nn].mode == (LOW_CYCLE_STARTS_AT + 1)) {
        sectionC[nn].colorDelayInt = COLOR_LOOP_SMOOTH_DELAY_INT;

    } else { // mode == (LOW_CYCLE_STARTS_AT + 2)
        sectionC[nn].colorDelayInt = COLOR_LOOP_SUDDEN_DELAY_INT;
    }
}

void switchToMax(uint8_t nn) { // 4
    for (uint8_t color = 0; color < 4; color++) {
        sectionC[nn].RGBW[color] = 1;
        sectionC[nn].RGBWon[color] = true;
    }
        
    sectionC[nn].masterLevel = 1;
}


void switchToSingleColor(uint8_t nn) { // 252-254
    sectionC[nn].isOn = true;
    for (uint8_t color = 0; color < 3; color++) {
        sectionC[nn].RGBWon[color] = false;
        if (sectionC[nn].RGBW[color] == 0) { // give em some value for combined from double bottom press from red
            sectionC[nn].RGBW[color] = sectionC[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS * 2 / 3;
        }
    }
    sectionC[nn].RGBWon[3] = false; // turn off white
    sectionC[nn].RGBW[3] = 0;
    // sectionC[nn].RGBW[sectionC[nn].mode-SINGLE_COLOR_MODE_OFFSET] = sectionC[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS * 2 / 3;
    sectionC[nn].RGBWon[sectionC[nn].mode-SINGLE_COLOR_MODE_OFFSET] = true;
    sectionC[nn].masterLevel = 1;
}

void switchToCombined(uint8_t nn) { // 255
    // fade with masterLevel, extend with white
    for (uint8_t color = 0; color < 3; color++) {
        sectionC[nn].RGBWon[color] = true;
    }
    // normalize brightness to sometimes expand range
    if ((sectionC[nn].RGBW[0] >= sectionC[nn].RGBW[1]) && (sectionC[nn].RGBW[0] >= sectionC[nn].RGBW[2])) {
        sectionC[nn].masterLevel = sectionC[nn].RGBW[0];  //0 is highest
        sectionC[nn].RGBW[1] = (sectionC[nn].RGBW[1] / sectionC[nn].RGBW[0]);
        sectionC[nn].RGBW[2] = (sectionC[nn].RGBW[2] / sectionC[nn].RGBW[0]);
        sectionC[nn].RGBW[0] = 1;
        
    } else if ((sectionC[nn].RGBW[1] >= sectionC[nn].RGBW[0]) && (sectionC[nn].RGBW[1] >= sectionC[nn].RGBW[2])) {
        sectionC[nn].masterLevel = sectionC[nn].RGBW[1];  //1 is highest
        sectionC[nn].RGBW[0] = (sectionC[nn].RGBW[0] / sectionC[nn].RGBW[1]);
        sectionC[nn].RGBW[2] = (sectionC[nn].RGBW[2] / sectionC[nn].RGBW[1]);
        sectionC[nn].RGBW[1] = 1;
    
    } else if ((sectionC[nn].RGBW[2] >= sectionC[nn].RGBW[0]) && (sectionC[nn].RGBW[2] >= sectionC[nn].RGBW[1])) {
        sectionC[nn].masterLevel = sectionC[nn].RGBW[2];  //2 is highest
        sectionC[nn].RGBW[0] = (sectionC[nn].RGBW[0] / sectionC[nn].RGBW[2]);
        sectionC[nn].RGBW[1] = (sectionC[nn].RGBW[1] / sectionC[nn].RGBW[2]);
        sectionC[nn].RGBW[2] = 1;
    }
}
//**********************************************************************

void updateLights(uint8_t i) { // updates a specific light section
    if (DEBUG) {
        DEBUG_updateLights(i);

    } else {
        uint8_t brightnessValue = 0; // index for brightness lookup table

        for (uint8_t color = 0; color < 4; color++) {
            if (sectionC[i].RGBWon[color]) {

                //brightnessValue = lookupTable(i, color);
                
                uint8_t height = (uint16_t(sectionC[i].RGBW[color] * sectionC[i].masterLevel * TABLE_SIZE) / HEIGHT);
                uint8_t width = (uint16_t(sectionC[i].RGBW[color] * sectionC[i].masterLevel * TABLE_SIZE) % WIDTH);

                // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
                memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

                if ((sectionC[i].RGBW[color] > 0) && (sectionC[i].masterLevel > 0) && (brightnessValue == 0)) 
                    brightnessValue = 1;
                
            } else {
                //brightnessValue = 0;
                // light is off, so turn it off
            }

            DmxSimple.write((sectionC[i].DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
            sectionC[i].lastRGBW[color] = sectionC[i].RGBW[color];
        }
    }
        
    if ( (sectionC[i].RGBW[0] <= 0) && (sectionC[i].RGBW[1] <= 0) && (sectionC[i].RGBW[2] <= 0) && (sectionC[i].RGBW[3] <= 0) ) {
        //switch to mode 0?

        sectionC[i].isOn = false;
        sectionC[i].masterLevel = 0;

        for (uint8_t color = 0; color < 4; color++) {  //clear rgbw
            sectionC[i].RGBW[color] = 0;
            sectionC[i].RGBWon[color] = false;
        }

        if (DEBUG) {
            DEBUG_updateLightsOff(i);
        }
    }
}


//***************************************************************************
// color progress functions:

void progressColorSudden(uint8_t ii) {
    if (sectionC[ii].colorDelayCtr < 1)
        sectionC[ii].colorDelayCtr += (2 * COLOR_LOOP_FADE_AMOUNT);

    else {
        sectionC[ii].colorDelayCtr = 0;

        sectionC[ii].colorState += 1;
        if (sectionC[ii].colorState == MAX_COLOR_STATE) 
            sectionC[ii].colorState = 0;

        //sectionC[ii].colorState = random(MAX_COLOR_STATE);

        //set new light color
        sectionC[ii].RGBW[0] = RED_LIST[sectionC[ii].colorState];
        sectionC[ii].RGBW[1] = GREEN_LIST[sectionC[ii].colorState];
        sectionC[ii].RGBW[2] = BLUE_LIST[sectionC[ii].colorState];

        updateLights(ii);

        if (DEBUG) {
            Serial.print(F("color progress state: ")); 
            Serial.println(sectionC[ii].colorState);
        }
    }
}

void progressColorSmooth(uint8_t ii)
{
    sectionC[ii].nextRGB[0] = RED_LIST[sectionC[ii].colorState]; // target levels for the current state
    sectionC[ii].nextRGB[1] = GREEN_LIST[sectionC[ii].colorState];
    sectionC[ii].nextRGB[2] = BLUE_LIST[sectionC[ii].colorState];

    if ((sectionC[ii].RGBW[0] == sectionC[ii].nextRGB[0]) && (sectionC[ii].RGBW[1] == sectionC[ii].nextRGB[1]) && (sectionC[ii].RGBW[2] == sectionC[ii].nextRGB[2])) {

        sectionC[ii].colorState += 1;
        if (sectionC[ii].colorState == MAX_COLOR_STATE) 
            sectionC[ii].colorState = 0;
        
        if (DEBUG) {
            Serial.print(F("color progress state: ")); 
            Serial.println(sectionC[ii].colorState);
        }

    } else { // else change colors to get closer to current state
        for (uint8_t color = 0; color < 3; color++) {

            if (sectionC[ii].RGBW[color] < sectionC[ii].nextRGB[color]) {

                sectionC[ii].RGBW[color] += COLOR_LOOP_FADE_AMOUNT;

                if (sectionC[ii].RGBW[color] >= sectionC[ii].nextRGB[color])
                    sectionC[ii].RGBW[color] = sectionC[ii].nextRGB[color];
                
            }
            else if (sectionC[ii].RGBW[color] > sectionC[ii].nextRGB[color]) {

                sectionC[ii].RGBW[color] -= COLOR_LOOP_FADE_AMOUNT;

                if (sectionC[ii].RGBW[color] <= sectionC[ii].nextRGB[color]) 
                    sectionC[ii].RGBW[color] = sectionC[ii].nextRGB[color];
            }
        }
    }
    updateLights(ii);
}