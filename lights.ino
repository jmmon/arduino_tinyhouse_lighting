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

void extendedFade(uint8_t ii) {
    if (!(section[ii].extendedFade)) {  // if extendedFade is not enabled yet for this section
        //check if mode's regular level is max and if so enable extendedFade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT) { // white 
            // if (section[ii].RGBW[3] >= 1) {
            if (section[ii].masterLevel >= 1) {
                section[ii].extendedFade = true;
            } // else hasn't hit max yet

        // } else if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness 
        //     // do nothing

        } else if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2)) {
                // RGB Smooth // RGB Sudden
            if (section[ii].masterLevel >= 1) {
                section[ii].extendedFade = true;
            } // else hasn't hit max yet

        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2) ) { // Red // Green // Blue
            if (section[ii].RGBW[section[ii].mode-SINGLE_COLOR_MODE_OFFSET] >= 1) {
                section[ii].extendedFade = true;
            } // else hasn't hit max yet

        } else if ( section[ii].mode == (HIGH_CYCLE_STARTS_AT + 3) ) { // Combined
            //if fading up and any of these hit max, we're at max. So enable extended fade
            if ( section[ii].masterLevel >= 1 ) {
                section[ii].extendedFade = true;
            } // else hasn't hit max yet
        }
    }
}

void disableExtendedFade(uint8_t ii) {
    if (section[ii].extendedFade) {
        //check if mode's extended level is max and if so disable extendedFade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT) { // white 
            // if rgb are 0, disable extended fade
            if ( section[ii].RGBW[0] <= 0 && section[ii].RGBW[1] <= 0 && section[ii].RGBW[2] <= 0 ) {
                section[ii].extendedFade = false;
            } // else hasn't hit min yet
        } else if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness 
            //do nothing
        } else if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 3) ) { 
            // Red // Green // Blue // Combined // RGB Smooth // RGB Sudden
            // white is extended fade, wait for this to be off
            if (section[ii].RGBW[3] <= 0) {
                section[ii].extendedFade = false;
            } // else hasn't hit min yet
        }
    }
}


// void turnOffSection(uint8_t ii) {
//     section[ii].isOn = false; // if "on," set to "off"
//     section[ii].mode = LOW_CYCLE_STARTS_AT;
//     section[ii].colorProgress = false;

//     for (uint8_t color = 0; color < 4; color++) {
//         section[ii].RGBW[color] = 0; // and set values to 0 for each color for that light
//         section[ii].RGBWon[color] = false;
//     }
//     //save masterLevel so that next time it can be used as the starting point?
//     section[ii].lastMasterLevel = section[ii].masterLevel;
//     section[ii].masterLevel = 0;
//     updateLights(ii);
// }


void switchMode(uint8_t nn) {
    if (DEBUG) {
        Serial.print(F("Now in mode: "));
        Serial.println(section[nn].mode);
    }

    section[nn].isOn = true;

    switch (section[nn].mode) {
        case (0): // off
            section[nn].isOn = false; // if "on," set to "off"
            section[nn].colorProgress = false;
            section[nn].extendedFade = false;

            for (uint8_t color = 0; color < 4; color++) {
                section[nn].RGBW[color] = 0; // and set values to 0 for each color for that light
                section[nn].RGBWon[color] = false;
            }
            //save masterLevel so that next time it can be used as the starting point?
            section[nn].lastMasterLevel = section[nn].masterLevel;
            section[nn].masterLevel = 0;
        break;

        case (LOW_CYCLE_STARTS_AT): // white
        //wipe:
            section[nn].colorProgress = false;
            for (uint8_t color = 0; color < 4; color++) {
                section[nn].RGBW[color] = 0;
                section[nn].RGBWon[color] = false;
            }
            
            section[nn].RGBW[3] = 1;
            section[nn].RGBWon[3] = true;
            section[nn].masterLevel = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
        break;
        
        case (LOW_CYCLE_STARTS_AT + 3):   //max brightness
            for (uint8_t color = 0; color < 4; color++) {
                section[nn].RGBW[color] = 1;
                section[nn].RGBWon[color] = true;
            }
                
            section[nn].masterLevel = 1;
        break;

        case (LOW_CYCLE_STARTS_AT + 1): // RGB smoothn
        case (LOW_CYCLE_STARTS_AT + 2): // RGB sudden
            if (!(section[nn].colorProgress)) {
                //initialize colorProgress
                section[nn].colorProgress = true;
                section[nn].colorStartTime = currentTime;

                section[nn].RGBW[3] = 0;
                section[nn].RGBWon[3] = false;
                for (uint8_t color = 0; color < 3; color++) {
                    section[nn].RGBWon[color] = true;
                }
                section[nn].masterLevel = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
            }

            section[nn].colorState = random(MAX_COLOR_STATE);
            section[nn].RGBW[0] = RED_LIST[section[nn].colorState];
            section[nn].RGBW[1] = GREEN_LIST[section[nn].colorState];
            section[nn].RGBW[2] = BLUE_LIST[section[nn].colorState];

            if (section[nn].mode == (LOW_CYCLE_STARTS_AT + 1)) {
                section[nn].colorDelayInt = COLOR_LOOP_SMOOTH_DELAY_INT;

            } else { // mode == (LOW_CYCLE_STARTS_AT + 2)
                section[nn].colorDelayInt = COLOR_LOOP_SUDDEN_DELAY_INT;
            }
        break;

        case (HIGH_CYCLE_STARTS_AT):        // r
        case (HIGH_CYCLE_STARTS_AT + 1):    // g
        case (HIGH_CYCLE_STARTS_AT + 2):    // b
            section[nn].isOn = true;
            for (uint8_t color = 0; color < 3; color++) {
                section[nn].RGBWon[color] = false;
                if (section[nn].RGBW[color] == 0) { // give em some value for combined from double bottom press from red
                    section[nn].RGBW[color] = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS * 2 / 3;
                }
            }
            section[nn].RGBWon[3] = false; // turn off white
            section[nn].RGBW[3] = 0;
            section[nn].RGBW[section[nn].mode-SINGLE_COLOR_MODE_OFFSET] = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS * 2 / 3;
            section[nn].RGBWon[section[nn].mode-SINGLE_COLOR_MODE_OFFSET] = true;
            section[nn].masterLevel = 1;
        break;

        case (HIGH_CYCLE_STARTS_AT + 3): // combined
            // fade with masterLevel, extend with white
            for (uint8_t color = 0; color < 3; color++) {
                section[nn].RGBWon[color] = true;
            }
            // normalize brightness to sometimes expand range
            if ((section[nn].RGBW[0] >= section[nn].RGBW[1]) && (section[nn].RGBW[0] >= section[nn].RGBW[2])) {
                section[nn].masterLevel = section[nn].RGBW[0];  //0 is highest
                section[nn].RGBW[1] = (section[nn].RGBW[1] / section[nn].RGBW[0]);
                section[nn].RGBW[2] = (section[nn].RGBW[2] / section[nn].RGBW[0]);
                section[nn].RGBW[0] = 1;
                
            } else if ((section[nn].RGBW[1] >= section[nn].RGBW[0]) && (section[nn].RGBW[1] >= section[nn].RGBW[2])) {
                section[nn].masterLevel = section[nn].RGBW[1];  //1 is highest
                section[nn].RGBW[0] = (section[nn].RGBW[0] / section[nn].RGBW[1]);
                section[nn].RGBW[2] = (section[nn].RGBW[2] / section[nn].RGBW[1]);
                section[nn].RGBW[1] = 1;
            
            } else if ((section[nn].RGBW[2] >= section[nn].RGBW[0]) && (section[nn].RGBW[2] >= section[nn].RGBW[1])) {
                section[nn].masterLevel = section[nn].RGBW[2];  //2 is highest
                section[nn].RGBW[0] = (section[nn].RGBW[0] / section[nn].RGBW[2]);
                section[nn].RGBW[1] = (section[nn].RGBW[1] / section[nn].RGBW[2]);
                section[nn].RGBW[2] = 1;
            }
        break;
    }

    if ((section[nn].mode != (LOW_CYCLE_STARTS_AT + 1)) && (section[nn].mode != (LOW_CYCLE_STARTS_AT + 2))) { //if not rgb smooth/sudden 
        updateLights(nn);
    }    
}

void updateLights(uint8_t i) { // updates a specific light section
    if (DEBUG) {
        DEBUG_updateLights(i);

    } else {
        uint8_t brightnessValue = 0; // index for brightness lookup table

        for (uint8_t color = 0; color < 4; color++) {
            if (section[i].RGBWon[color]) {

                //brightnessValue = lookupTable(i, color);
                
                uint8_t height = (uint16_t(section[i].RGBW[color] * section[i].masterLevel * TABLE_SIZE) / HEIGHT);
                uint8_t width = (uint16_t(section[i].RGBW[color] * section[i].masterLevel * TABLE_SIZE) % WIDTH);

                // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
                memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

                if ((section[i].RGBW[color] > 0) && (section[i].masterLevel > 0) && (brightnessValue == 0)) 
                    brightnessValue = 1;
                
            } else {
                //brightnessValue = 0;
                // light is off, so turn it off
            }

            DmxSimple.write((section[i].DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
            section[i].lastRGBW[color] = section[i].RGBW[color];
        }
    }
        
    if ( (section[i].RGBW[0] <= 0) && (section[i].RGBW[1] <= 0) && (section[i].RGBW[2] <= 0) && (section[i].RGBW[3] <= 0) ) {
        //switch to mode 0?

        section[i].isOn = false;
        section[i].masterLevel = 0;

        for (uint8_t color = 0; color < 4; color++) {  //clear rgbw
            section[i].RGBW[color] = 0;
            section[i].RGBWon[color] = false;
        }

        if (DEBUG) {
            DEBUG_updateLightsOff(i);
        }
    }
}

// **************************************************************************************
// Fade tick functions:
void masterFadeIncrement(uint8_t i, float f) {
    //master fade
    float temp = section[i].BRIGHTNESS_FACTOR * f;
    section[i].isOn = true;
    section[i].masterLevel += temp;
    if (section[i].masterLevel > 1) {
        section[i].masterLevel = 1; // max
    }
    updateLights(i);
}


void fadeIncrement(uint8_t i, float f, uint8_t color) {
    float temp = section[i].BRIGHTNESS_FACTOR * f;
    // if (color == NULL) {
    //     //master fade
    //     section[i].isOn = true;
    //     section[i].masterLevel += temp;
    //     if (section[i].masterLevel > 1) {
    //         section[i].masterLevel = 1; // max
    //     }
    // } else {
        //regular fade
        section[i].RGBWon[color] = true;
        section[i].RGBW[color] += temp;
        if (section[i].RGBW[color] > 1) {
            section[i].RGBW[color] = 1; // max
        }
    //}
    
    updateLights(i);
}

void masterFadeDecrement(uint8_t i, float f) {
    //master fade
    float temp = section[i].BRIGHTNESS_FACTOR * f;
    section[i].masterLevel -= temp;
    if (section[i].masterLevel < 0) {
        section[i].isOn = false;
        section[i].masterLevel = 0; // min
    }
    updateLights(i);
}

void fadeDecrement(uint8_t i, float f, uint8_t color) {
    float temp = section[i].BRIGHTNESS_FACTOR * f;
    // if (color == NULL) {
    //     //master fade
    //     section[i].masterLevel -= temp;
    //     if (section[i].masterLevel < 0) {
    //         section[i].isOn = false;
    //         section[i].masterLevel = 0; // min
    //     }
    // } else {
        //regular fade
        section[i].RGBW[color] -= temp;
        if (section[i].RGBW[color] < 0) {
            section[i].RGBWon[color] = false;
            section[i].RGBW[color] = 0; // min
        }
    //}
    
    updateLights(i);
}


// **************************************************************************************
// color progress functions:

void progressColorSudden(uint8_t ii) {
    if (section[ii].colorDelayCtr < 1)
        section[ii].colorDelayCtr += (2 * COLOR_LOOP_FADE_AMOUNT);

    else {
        section[ii].colorDelayCtr = 0;

        section[ii].colorState += 1;
        if (section[ii].colorState == MAX_COLOR_STATE) 
            section[ii].colorState = 0;

        //section[ii].colorState = random(MAX_COLOR_STATE);

        //set new light color
        section[ii].RGBW[0] = RED_LIST[section[ii].colorState];
        section[ii].RGBW[1] = GREEN_LIST[section[ii].colorState];
        section[ii].RGBW[2] = BLUE_LIST[section[ii].colorState];

        updateLights(ii);

        if (DEBUG) {
            Serial.print(F("color progress state: ")); 
            Serial.println(section[ii].colorState);
        }
    }
}

void progressColorSmooth(uint8_t ii)
{
    section[ii].nextRGB[0] = RED_LIST[section[ii].colorState]; // target levels for the current state
    section[ii].nextRGB[1] = GREEN_LIST[section[ii].colorState];
    section[ii].nextRGB[2] = BLUE_LIST[section[ii].colorState];

    if ((section[ii].RGBW[0] == section[ii].nextRGB[0]) && (section[ii].RGBW[1] == section[ii].nextRGB[1]) && (section[ii].RGBW[2] == section[ii].nextRGB[2])) {

        section[ii].colorState += 1;
        if (section[ii].colorState == MAX_COLOR_STATE) 
            section[ii].colorState = 0;
        
        if (DEBUG) {
            Serial.print(F("color progress state: ")); 
            Serial.println(section[ii].colorState);
        }

    } else { // else change colors to get closer to current state
        for (uint8_t color = 0; color < 3; color++) {

            if (section[ii].RGBW[color] < section[ii].nextRGB[color]) {

                section[ii].RGBW[color] += COLOR_LOOP_FADE_AMOUNT;

                if (section[ii].RGBW[color] >= section[ii].nextRGB[color])
                    section[ii].RGBW[color] = section[ii].nextRGB[color];
                
            }
            else if (section[ii].RGBW[color] > section[ii].nextRGB[color]) {

                section[ii].RGBW[color] -= COLOR_LOOP_FADE_AMOUNT;

                if (section[ii].RGBW[color] <= section[ii].nextRGB[color]) 
                    section[ii].RGBW[color] = section[ii].nextRGB[color];
            }
        }
    }
    updateLights(ii);
}