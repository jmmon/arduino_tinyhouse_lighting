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
            if (section[ii].RGBW[section[ii].mode-HIGH_CYCLE_STARTS_AT] >= 1) {
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

//***************************************************************************
// Fade tick functions:
void masterFadeIncrement(uint8_t i, float f) {
    //master fade
    section[i].isOn = true;
    section[i].masterLevel = ((section[i].masterLevel + f) > 1.0) ? 1.0 : (section[i].masterLevel + f);

    updateLights(i);
}


void fadeIncrement(uint8_t i, float f, uint8_t color = 4) {
    if (color == 4) {
        section[i].isOn = true;
        section[i].masterLevel = ((section[i].masterLevel + f) > 1.0) ? 1.0 : (section[i].masterLevel + f);

    } else {
        section[i].RGBWon[color] = true;
        section[i].RGBW[color] = ((section[i].RGBW[color] + f) > 1.0) ? 1.0 : (section[i].RGBW[color] + f);

    }
    
    updateLights(i);
}

void masterFadeDecrement(uint8_t i, float f) {
    //master fade
    
    section[i].masterLevel -= f;
    if (section[i].masterLevel < 0) {
        section[i].isOn = false;
        section[i].masterLevel = 0; // min
    }
    updateLights(i);
}

void fadeDecrement(uint8_t i, float f, uint8_t color = 4) {
    if (color == 4) {
        section[i].masterLevel -= f;
        if (section[i].masterLevel < 0) {
            section[i].isOn = false;
            section[i].masterLevel = 0; // min
        }
    } else {
        //regular fade
        section[i].RGBW[color] -= f;
        if (section[i].RGBW[color] < 0) {
            section[i].RGBWon[color] = false;
            section[i].RGBW[color] = 0; // min
        }

    }

    updateLights(i);
}


//**********************************************************************

void switchMode(uint8_t nn) {
    if (DEBUG) {
        Serial.print(F("Now in mode: ")); Serial.println(section[nn].mode);
    }

    section[nn].isOn = true;

    switch (section[nn].mode) {
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
}

void switchToOff(uint8_t nn) { // 0
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
    
    updateLights(nn);
}

void switchToWhite(uint8_t nn) { // 1
    //wipe:
    section[nn].colorProgress = false;
    for (uint8_t color = 0; color < 4; color++) {
        section[nn].RGBW[color] = 0;
        section[nn].RGBWon[color] = false;
    }
    
    section[nn].RGBW[3] = 1;
    section[nn].RGBWon[3] = true;
    section[nn].masterLevel = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
    
    updateLights(nn);
}

void switchToRGB(uint8_t nn) { // 2, 3
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

    section[nn].colorDelayInt = (section[nn].mode == (LOW_CYCLE_STARTS_AT + 1)) ? COLOR_LOOP_SMOOTH_DELAY_INT : COLOR_LOOP_SUDDEN_DELAY_INT;
}

void switchToMax(uint8_t nn) { // 4
    for (uint8_t color = 0; color < 4; color++) {
        section[nn].RGBW[color] = 1;
        section[nn].RGBWon[color] = true;
    }
        
    section[nn].masterLevel = 1;
    
    updateLights(nn);
}


void switchToSingleColor(uint8_t nn) {
    section[nn].isOn = true;
    section[nn].RGBWon[3] = false; // turn off white
    section[nn].RGBW[3] = 0;
    
    for (uint8_t color = 0; color < 3; color++) {
        section[nn].RGBWon[color] = false;
        if (section[nn].RGBW[color] == 0) { // give em some value for combined from double bottom press from red
            section[nn].RGBW[color] = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS * 2 / 3;
        }
    }
    // section[nn].RGBW[section[nn].mode-HIGH_CYCLE_STARTS_AT] = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS * 2 / 3;
    section[nn].RGBWon[section[nn].mode-HIGH_CYCLE_STARTS_AT] = true;
    section[nn].masterLevel = 1;
    
    updateLights(nn);
}

void switchToCombined(uint8_t nn) { // 255
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
    
    updateLights(nn);
}

//********************************************************************************************************************
void DEBUG_updateLights(uint8_t ii) {
    //uint8_t brightnessValue = lookupTable(ii, 3); //index for brightness lookup table
    uint8_t brightnessValue = 0; //index for brightness lookup table

    uint8_t height = (uint16_t(section[ii].RGBW[3] * section[ii].masterLevel * TABLE_SIZE) / HEIGHT);
    uint8_t width = (uint16_t(section[ii].RGBW[3] * section[ii].masterLevel * TABLE_SIZE) % WIDTH);

    // look up brighness from table and saves as temp ( sizeof(temp) resolves to 1 [byte of data])
    memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

    if ((section[ii].RGBW[3] > 0) && (section[ii].masterLevel > 0) && (brightnessValue == 0)) 
        brightnessValue = 1;
    

    Serial.print(F("table_w:"));    Serial.print(width);
    Serial.print(F(", table_h:"));  Serial.print(height);
    
    Serial.print(F(" lvl:"));   Serial.print(brightnessValue);  Serial.print(F("  "));
    Serial.print((section[ii].RGBWon[3]) ? F("   W:"): F(" !!W:"));    Serial.print(section[ii].RGBW[3]);
    Serial.print((section[ii].RGBWon[0]) ? F("   R:"): F(" !!R:"));    Serial.print(section[ii].RGBW[0]);
    Serial.print((section[ii].RGBWon[1]) ? F("   G:"): F(" !!G:"));    Serial.print(section[ii].RGBW[1]);
    Serial.print((section[ii].RGBWon[2]) ? F("   B:"): F(" !!B:"));    Serial.print(section[ii].RGBW[2]);
    Serial.print((section[ii].isOn) ? F(" Master level: "): F(" !!Master level: ")); Serial.print(section[ii].masterLevel);
    Serial.print(F(" cur_t:")); Serial.println(currentTime);
}


void updateLights(uint8_t i) { // updates a specific light section
    if (DEBUG) {
        DEBUG_updateLights(i);

    } else {

        for (uint8_t color = 0; color < 4; color++) {
            uint8_t brightnessValue = 0; // index for brightness lookup table
            if (section[i].RGBWon[color]) {                
                uint8_t height = (uint16_t(section[i].RGBW[color] * section[i].masterLevel * TABLE_SIZE) / HEIGHT);
                uint8_t width = (uint16_t(section[i].RGBW[color] * section[i].masterLevel * TABLE_SIZE) % WIDTH);

                // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
                memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

                if ((section[i].RGBW[color] > 0) && (section[i].masterLevel > 0) && (brightnessValue == 0)) 
                    brightnessValue = 1;
                
            }

            DmxSimple.write((section[i].DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
            section[i].lastRGBW[color] = section[i].RGBW[color];
        }
    }
        
    if ( (section[i].RGBW[0] <= 0) && (section[i].RGBW[1] <= 0) && (section[i].RGBW[2] <= 0) && (section[i].RGBW[3] <= 0) ) {

        section[i].isOn = false;
        section[i].masterLevel = 0;

        for (uint8_t color = 0; color < 4; color++) {  //clear rgbw
            section[i].RGBW[color] = 0;
            section[i].RGBWon[color] = false;
        }

        if (DEBUG) {
            Serial.print(F("MasterBrightness: ")); Serial.println(section[i].masterLevel);
            Serial.print(F("IsOn:")); Serial.println(section[i].isOn);
        }
    }
}


//********************************************************************************************************************
// color progress functions:

void progressColorSudden(uint8_t ii) {
    if (section[ii].colorDelayCtr < 1)
        section[ii].colorDelayCtr += (2 * COLOR_LOOP_FADE_AMOUNT);

    else {
        section[ii].colorDelayCtr = 0;

        section[ii].colorState = ((section[ii].colorState + 1) == MAX_COLOR_STATE) ? 0 : section[ii].colorState + 1;

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

void progressColorSmooth(uint8_t ii) {
    section[ii].nextRGB[0] = RED_LIST[section[ii].colorState]; // target levels for the current state
    section[ii].nextRGB[1] = GREEN_LIST[section[ii].colorState];
    section[ii].nextRGB[2] = BLUE_LIST[section[ii].colorState];

    if ((section[ii].RGBW[0] == section[ii].nextRGB[0]) && (section[ii].RGBW[1] == section[ii].nextRGB[1]) && (section[ii].RGBW[2] == section[ii].nextRGB[2])) {

        section[ii].colorState = ((section[ii].colorState + 1) == MAX_COLOR_STATE) ? 0 : section[ii].colorState + 1;
        
        if (DEBUG) {
            Serial.print(F("color progress state: ")); 
            Serial.println(section[ii].colorState);
        }

    } else { // else change colors to get closer to current state
        for (uint8_t color = 0; color < 3; color++) {

            if (section[ii].RGBW[color] < section[ii].nextRGB[color]) {

                section[ii].RGBW[color] = ((section[ii].RGBW[color] + COLOR_LOOP_FADE_AMOUNT) >= section[ii].nextRGB[color]) ? section[ii].nextRGB[color] : section[ii].RGBW[color] + COLOR_LOOP_FADE_AMOUNT;
            }
            else if (section[ii].RGBW[color] > section[ii].nextRGB[color]) {

                section[ii].RGBW[color] = ((section[ii].RGBW[color] - COLOR_LOOP_FADE_AMOUNT) <= section[ii].nextRGB[color]) ? section[ii].nextRGB[color] : section[ii].RGBW[color] - COLOR_LOOP_FADE_AMOUNT;
            }
        }
    }
    updateLights(ii);
}
