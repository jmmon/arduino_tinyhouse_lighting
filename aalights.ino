void extendedFade(uint8_t ii) {
    if (!(section[ii].extendedFade)) { // check if mode's regular level is max and if so enable extendedFade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT || section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2) ||  section[ii].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // white  // RGB Smooth // RGB Sudden // Combined

            if (section[ii].RGBWM[4]->lvl == TABLE_SIZE) { // else hasn't hit max yet
                section[ii].extendedFade = true;
            }

        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2) ) { // Red // Green // Blue

            if (section[ii].RGBWM[section[ii].mode-HIGH_CYCLE_STARTS_AT]->lvl == TABLE_SIZE) { // else hasn't hit max yet
                section[ii].extendedFade = true;
            }

        }
    }
}

void disableExtendedFade(uint8_t ii) {
    if (section[ii].extendedFade) { // check if mode's extended level is max and if so disable extendedFade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT) { // white

            if ( section[ii].RGBWM[0]->lvl == 0 && section[ii].RGBWM[1]->lvl == 0 && section[ii].RGBWM[2]->lvl == 0 ) { // else hasn't hit min yet
                section[ii].extendedFade = false;
            }

        } else if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 3) ) { 

            if (section[ii].RGBWM[3]->lvl == 0) { // else hasn't hit min yet
                section[ii].extendedFade = false;
            }
        }
    }
}

//******************************************************************************************************************************************************
// ~~~~~~~~ Fade Functions ~~~~~~~~
//******************************************************************************************************************************************************

void fadeIncrement(uint8_t i, uint16_t amt, uint8_t color = 4) {
    if (color == 4) {
        section[i].RGBWM[4]->isOn = true;
        section[i].RGBWM[4]->lvl = (section[i].RGBWM[4]->lvl >= TABLE_SIZE - amt) ? 
            TABLE_SIZE : 
            (section[i].RGBWM[4]->lvl + amt);

    } else {
        section[i].RGBWM[color]->isOn = true;
        section[i].RGBWM[color]->lvl = (section[i].RGBWM[color]->lvl >= TABLE_SIZE - amt) ? 
            TABLE_SIZE : 
            (section[i].RGBWM[color]->lvl + amt);

    }
    
}


void fadeDecrement(uint8_t i, uint16_t amt, uint8_t color = 4) {
    if (color == 4) {
        if (section[i].RGBWM[4]->lvl < 0 + amt) {
            section[i].RGBWM[4]->isOn = false;
            section[i].RGBWM[4]->lvl = 0; // min

        } else {
            section[i].RGBWM[4]->lvl -= amt;
        }

    } else {
        //regular fade
        if (section[i].RGBWM[color]->lvl < 0 + amt) {
            section[i].RGBWM[color]->isOn = false;
            section[i].RGBWM[color]->lvl = 0; // min

        } else{
            section[i].RGBWM[color]->lvl -= amt;
        }

    }

}

//***************************************************************************


void fadeUp(uint8_t ii) {
    uint16_t s = calcSpeed(ii, 1, FADE_AMT_BASE);
    if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
        fadeIncrement(ii, s);

    } else if (section[ii].extendedFade) {
        if (section[ii].mode == LOW_CYCLE_STARTS_AT) { // white
            for (uint8_t color = 0; color < 3; color++) {
                fadeIncrement(ii, s, color); // rgb
            }
        } else {
            fadeIncrement(ii, s, 3);
        }
    } else {
        //regular fade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT || section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // white, rgb smooth, sudden;  combined
            fadeIncrement(ii, s);

        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
            fadeIncrement(ii, s, section[ii].mode-HIGH_CYCLE_STARTS_AT);
        }
    }
}


void fadeDown(uint8_t ii) {
    uint16_t s = calcSpeed(ii, 0, FADE_AMT_BASE);
    if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
        fadeDecrement(ii, s);

    } else if (section[ii].extendedFade) {
        if (section[ii].mode == LOW_CYCLE_STARTS_AT) { // white
            for (uint8_t color = 0; color < 3; color++) {
                fadeDecrement(ii, s, color);
            }
        } else {
            fadeDecrement(ii, s, 3);
        }
    } else { // regular fade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT || section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // rgb smooth, sudden, combined
            fadeDecrement(ii, s);

        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
            uint8_t color = section[ii].mode-HIGH_CYCLE_STARTS_AT;
            fadeDecrement(ii, s, color);
        }
    }
}


uint16_t calcSpeed(uint8_t ii, uint8_t bb, uint16_t tempSpeed = FADE_AMT_BASE) {
/*     uint16_t tempSpeed = FADE_AMT_BASE; */
    uint8_t delayBonus = 10;

    if (currentTime >= (section[ii].btn[bb].timePressed + ((BTN_FADE_DELAY + delayBonus) * 4))) {
        tempSpeed *= 8;
    } else if (currentTime >= (section[ii].btn[bb].timePressed + ((BTN_FADE_DELAY + delayBonus) * 3))) {
        tempSpeed *= 4;
    } else if (currentTime >= (section[ii].btn[bb].timePressed + ((BTN_FADE_DELAY + delayBonus) * 2))) {
        tempSpeed *= 2;
    }

    tempSpeed = uint16_t(tempSpeed * section[ii].LEVEL_FACTOR / 100.0); //// factor: 100 == 100%

    return tempSpeed;
}


// *********************************************************************************

void DEBUG_updateLights(uint8_t ii) {
    for (uint8_t c = 0; c < 4; c++) {
        uint8_t brightnessValue = 0;

        uint8_t height = uint16_t((1.0 * section[ii].RGBWM[c]->lvl / TABLE_SIZE) * (section[ii].RGBWM[4]->lvl)) / WIDTH;
        uint8_t width = uint16_t((1.0 * section[ii].RGBWM[c]->lvl / TABLE_SIZE) * (section[ii].RGBWM[4]->lvl)) % WIDTH;

        // look up brighness from table and saves as temp ( sizeof(temp) resolves to 1 [byte of data])
        memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

        if ((section[ii].RGBWM[c]->lvl > 0) && (section[ii].RGBWM[4]->lvl > 0) && (brightnessValue == 0)) 
            brightnessValue = 1;
        
        Serial.print((section[ii].RGBWM[c]->isOn) ? F("  "): F("!!"));
        Serial.print(c);
        Serial.print(F("_"));    Serial.print(width);
        Serial.print(F(":"));  Serial.print(height);
        Serial.print(F(" ("));   Serial.print(brightnessValue);  Serial.print(F(") - "));
        Serial.print(section[ii].RGBWM[c]->lvl); Serial.print(F("  "));
        
    }
    Serial.print((section[ii].RGBWM[4]->isOn) ? F("    mLevel: "): F("  !!mLevel: ")); Serial.print(section[ii].RGBWM[4]->lvl);
    Serial.print(F(" _t:")); Serial.println(currentTime);
}


void updateLights(uint8_t i) { // updates a specific light section
    if (DEBUG) {
        DEBUG_updateLights(i);

    } else {
        for (uint8_t color = 0; color < 4; color++) {
            uint8_t brightnessValue = 0;
            
            if (section[i].RGBWM[color]->isOn) {
                uint8_t height = uint16_t((1.0 * section[i].RGBWM[color]->lvl/TABLE_SIZE) * (section[i].RGBWM[4]->lvl)) / WIDTH;
                uint8_t width = uint16_t((1.0 * section[i].RGBWM[color]->lvl/TABLE_SIZE) * (section[i].RGBWM[4]->lvl)) % WIDTH;

                // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
                memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

                if ((section[i].RGBWM[color]->lvl > 0) && (section[i].RGBWM[4]->lvl > 0) && (brightnessValue == 0)) 
                    brightnessValue = 1;
                
            }

            DmxSimple.write((section[i].DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
            section[i].RGBWM[color]->lastLvl = section[i].RGBWM[color]->lvl;
        }
    }

    if ( (section[i].RGBWM[0]->lvl == 0) && (section[i].RGBWM[1]->lvl == 0) && (section[i].RGBWM[2]->lvl == 0) && (section[i].RGBWM[3]->lvl == 0) && !(section[i].colorProgress)) {

        section[i].RGBWM[4]->isOn = false;
        section[i].RGBWM[4]->lvl = 0;

        for (uint8_t color = 0; color < 4; color++) {  //clear rgbw
            section[i].RGBWM[color]->lvl = 0;
            section[i].RGBWM[color]->isOn = false;
        }

        if (DEBUG) {
            Serial.print(F("MasterBrightness: ")); Serial.println(section[i].RGBWM[4]->lvl);
            Serial.print(F("IsOn:")); Serial.println(section[i].RGBWM[4]->isOn);
        }
    }
}


//********************************************************************************************************************
// color progress functions:

void progressColorSudden(uint8_t ii) {

    section[ii].colorState = ((section[ii].colorState + 1) == MAX_COLOR_STATE) ? 
        0 : 
        section[ii].colorState + 1;
    
    if (DEBUG) {
        Serial.print(F("color progress state: ")); 
        Serial.println(section[ii].colorState);
    }

    //set new light color
    section[ii].RGBWM[0]->lvl = uint16_t(TABLE_SIZE * (RED_LIST_NEW[section[ii].colorState]  / 100.0));
    section[ii].RGBWM[1]->lvl = uint16_t(TABLE_SIZE * (GREEN_LIST_NEW[section[ii].colorState]  / 100.0));
    section[ii].RGBWM[2]->lvl = uint16_t(TABLE_SIZE * (BLUE_LIST_NEW[section[ii].colorState]  / 100.0));

    updateLights(ii);
}

void progressColorSmooth(uint8_t ii) {
    section[ii].RGBWM[0]->nextLvl = uint16_t(1.0 * TABLE_SIZE * RED_LIST_NEW[section[ii].colorState]  / 100.0); // target levels for the current state
    section[ii].RGBWM[1]->nextLvl = uint16_t(1.0 * TABLE_SIZE * GREEN_LIST_NEW[section[ii].colorState]  / 100.0);
    section[ii].RGBWM[2]->nextLvl = uint16_t(1.0 * TABLE_SIZE * BLUE_LIST_NEW[section[ii].colorState]  / 100.0);


    if ((section[ii].RGBWM[0]->lvl == section[ii].RGBWM[0]->nextLvl) && (section[ii].RGBWM[1]->lvl == section[ii].RGBWM[1]->nextLvl) && (section[ii].RGBWM[2]->lvl == section[ii].RGBWM[2]->nextLvl)) {
        section[ii].colorState = ((section[ii].colorState + 1) == MAX_COLOR_STATE) ? 
            0 : 
            section[ii].colorState + 1;
        
        if (DEBUG) {
            Serial.print(F("color progress state: ")); Serial.print(section[ii].colorState);Serial.print(F(" - RGB: "));
            Serial.print(section[ii].RGBWM[0]->nextLvl); Serial.print(F(":")); Serial.print(section[ii].RGBWM[1]->nextLvl); Serial.print(F(":")); Serial.print(section[ii].RGBWM[2]->nextLvl);
            Serial.println();
        }

    } else { // else change colors to get closer to current state
        uint16_t tempFadeAmt = (COLOR_PROG_FADE_FACTOR_DEFAULT * section[ii].colorProgFadeFactor);
        for (uint8_t color = 0; color < 3; color++) {
            if (section[ii].RGBWM[color]->lvl < section[ii].RGBWM[color]->nextLvl) {
                section[ii].RGBWM[color]->lvl = (section[ii].RGBWM[color]->lvl  >= (section[ii].RGBWM[color]->nextLvl - tempFadeAmt)) ? 
                    section[ii].RGBWM[color]->nextLvl : 
                    section[ii].RGBWM[color]->lvl + tempFadeAmt;
                
            } else if (section[ii].RGBWM[color]->lvl > section[ii].RGBWM[color]->nextLvl) {
                section[ii].RGBWM[color]->lvl = (section[ii].RGBWM[color]->lvl <= (section[ii].RGBWM[color]->nextLvl + tempFadeAmt)) ?
                    section[ii].RGBWM[color]->nextLvl : 
                    section[ii].RGBWM[color]->lvl - tempFadeAmt;
            }
        }
    }
    updateLights(ii);
}
