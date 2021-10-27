void btnHeld(uint8_t ii, uint8_t bb) {   // happens every loop after "held delay (230ms)"
    if (!(section[ii].btn[bb].isHeld)) {
        section[ii].btn[bb].isHeld = true;
    }

    if (bb == 1) {
        if (section[ii].btn[bb].pressCt == 3) {
            btnTopHeld3p(ii);
        } else if (section[ii].btn[bb].pressCt == 2) {
            btnTopHeld2p(ii);
        } else if (section[ii].btn[bb].pressCt == 1) {
            btnTopHeld1p(ii);
        }
        
    } else { // b == 0
        // disable extended fade while fading down (fades from extended thru to off)
        disableExtendedFade(ii); // bottom, fade down, causes dim without stopping
    
        if (section[ii].btn[bb].pressCt == 3) {
            btnBotHeld3p(ii);
        } else if (section[ii].btn[bb].pressCt == 2) {
            btnBotHeld2p(ii);
        } else if (section[ii].btn[bb].pressCt == 1) {
            btnBotHeld1p(ii);
        } 
    }
}



//**********************************************************************************************************

void btnTopHeld1p(uint8_t ii) {
    if (!(section[ii].RGBWM[4]->isOn)) {  // if fading up from off, turn "on" the correct light
        section[ii].RGBWM[4]->isOn = true;
        if (section[ii].mode == 0) {
            section[ii].mode = LOW_CYCLE_STARTS_AT;
            section[ii].RGBWM[3]->isOn = true; //white on

            section[ii].RGBWM[3]->lvl = TABLE_SIZE;
            section[ii].RGBWM[4]->lvl = 1; //fixes fade up from off

        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) {
            section[ii].RGBWM[section[ii].mode-HIGH_CYCLE_STARTS_AT]->isOn = true;
        }
    }

    fadeUp(ii);

    updateLights(ii);
}

void btnBotHeld1p(uint8_t ii) {
    // first fade down rgb/white if on, then the current mode fade down.
    fadeDown(ii);
    
    updateLights(ii);
}




void btnTopHeld2p(uint8_t ii) {
    if ((section[ii].mode == (LOW_CYCLE_STARTS_AT + 1)) || (section[ii].mode == (LOW_CYCLE_STARTS_AT + 2))) {
        if (colorProgFadeDelayCtr < COLOR_PROG_FADE_DELAY_CYCLES) {   
            colorProgFadeDelayCtr++;
            
        } else {
            colorProgFadeDelayCtr = 0;

            // if (section[ii].colorProgFadeFactor > COLOR_PROG_FADE_FACTOR_MIN)
            //     section[ii].colorProgFadeFactor--;
            //     // decrease factor (speed)

            if (section[ii].colorProgFadeFactor < COLOR_PROG_FADE_FACTOR_MAX) 
                section[ii].colorProgFadeFactor++;
                // increase factor (speed)

            uint16_t tempSpeed = calcSpeed(ii, 1, COLOR_PROG_FADE_FACTOR_MIN);
            

            /* uint16_t tempSpeed = COLOR_PROG_FADE_FACTOR_MIN;
            uint8_t delayBonus = 20;

            if (currentTime >= (section[ii].btn[0].timePressed + ((BTN_FADE_DELAY + delayBonus) * 4))) {
                tempSpeed *= 8;

            } else if (currentTime >= (section[ii].btn[0].timePressed + ((BTN_FADE_DELAY + delayBonus) * 3))) {
                tempSpeed *= 4; // quadruple speed after triple time

            } else if (currentTime >= (section[ii].btn[0].timePressed + ((BTN_FADE_DELAY + delayBonus) * 2))) {
                tempSpeed *= 2; // double delay speed after double time
            } */

            section[ii].colorProgFadeFactor = (section[ii].colorProgFadeFactor < COLOR_PROG_FADE_FACTOR_MAX - tempSpeed) ? section[ii].
                colorProgFadeFactor + tempSpeed : 
                COLOR_PROG_FADE_FACTOR_MAX;
        }
    }
}

void btnBotHeld2p(uint8_t ii) {
    if ((section[ii].mode == (LOW_CYCLE_STARTS_AT + 1)) || (section[ii].mode == (LOW_CYCLE_STARTS_AT + 2))) {
        if (colorProgFadeDelayCtr < COLOR_PROG_FADE_DELAY_CYCLES) {   
            colorProgFadeDelayCtr++;
            
        } else {
            colorProgFadeDelayCtr = 0;

            uint16_t tempSpeed = calcSpeed(ii, 0, COLOR_PROG_FADE_FACTOR_MIN);
            
            section[ii].colorProgFadeFactor = (section[ii].colorProgFadeFactor > COLOR_PROG_FADE_FACTOR_MIN + tempSpeed) ? section[ii].
                colorProgFadeFactor - tempSpeed : 
                COLOR_PROG_FADE_FACTOR_MIN;

        }
    }
}




void btnTopHeld3p(uint8_t ii) {
    //fade up similar lights
    /**
        - Save the original level as color light levels and then convert to "max brightness" scaling.
        - Then, need another var to hold overflow (past 100% brightness) to keep lights in sync
        - Once a different type of action is used, reset the variable to wipe the memory


        if on, fade up all on lights of same mode and on.
        if off, fade up all lights of all modes.
         - Need to maintain scaling of lights relative to 0 and eachother, 
            so if fading to max and then fading back down the light levels should all be in scale still
         - Lights should always maintain the same zero-point
         - Should not turn "off" if accidentally hitting zero

     */
    
    // save as lastLvl

    //  /**
    if (section[ii].RGBWM[4]->isOn) {
        // fade up all lights of same mode
        
        //fade up simillar lights
        for (uint8_t k = 0; k < SECTION_COUNT; k++) {
            if (section[ii].mode == section[k].mode && section[k].RGBWM[4]->isOn) {
                
                uint16_t s = calcSpeed(ii, 1, FADE_AMT_BASE);
                if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
                    fadeIncrement(k, s);
                    // section[k].RGBWM[4]->isOn = true;
                    // uint16_t tempLvl = section[k].RGBWM[4]->lvl + amt;
                    // if (tempLvl > TABLE_SIZE) {
                    //     section[k].RGBWM[4]->buffer = tempLvl - TABLE_SIZE;
                    //     section[k].RGBWM[4]->lvl = TABLE_SIZE;
                    // } else {
                    //     section[k].RGBWM[4]->lvl = tempLvl;
                    // }

                        

                } else if (section[k].extendedFade) {
                    if (section[k].mode == LOW_CYCLE_STARTS_AT) { // white
                        for (uint8_t color = 0; color < 3; color++) {
                            fadeIncrement(k, s, color);
                            // section[k].RGBWM[color]->isOn = true;
                            // uint16_t tempLvl = section[k].RGBWM[color]->lvl + amt;

                            // if (tempLvl > TABLE_SIZE) {
                            //     section[k].RGBWM[color]->buffer = tempLvl - TABLE_SIZE;
                            //     section[k].RGBWM[color]->lvl = TABLE_SIZE;
                            // } else {
                            //     section[k].RGBWM[color]->lvl = tempLvl;
                            // }

                        }
                    } else {
                        fadeIncrement(k, s, 3);


                        // section[k].RGBWM[3]->isOn = true;
                        // uint16_t tempLvl = section[k].RGBWM[3]->lvl + amt;
                        // if (tempLvl > TABLE_SIZE) {
                        //     section[k].RGBWM[3]->buffer = tempLvl - TABLE_SIZE;
                        //     section[k].RGBWM[3]->lvl = TABLE_SIZE;
                        // } else {
                        //     section[k].RGBWM[3]->lvl = tempLvl;
                        // }

                        
                    }
                } else {
                        //regular fade
                        if (section[k].mode == LOW_CYCLE_STARTS_AT || section[k].mode == (LOW_CYCLE_STARTS_AT + 1) || section[k].mode == (LOW_CYCLE_STARTS_AT + 2) || section[k].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // white, rgb smooth, sudden;  combined
                            fadeIncrement(k, s);

                        } else if (section[k].mode == HIGH_CYCLE_STARTS_AT || section[k].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[k].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
                            fadeIncrement(k, s, section[k].mode-HIGH_CYCLE_STARTS_AT);
                        }
                    }

                updateLights(k);
                
            }
        }

    } 
    /**
    else {
        // fade up ALL lights

        for (uint8_t section = 0; section < SECTION_COUNT; section++) {
            switch(section[section].mode) {
                case(1):
                    //fade up Whites
                    break;

                case(2):
                case(3):
                    //fade up RGBs
                
                    break;

                case(4):
                    //fade up max brightness
                    break;

                case(11):
                case(12):
                case(13):
                    //fade up color
                    break;
                
                case(14):
                    //fade up combined
                    break;
        }
    }
     */
    //  */
}

void btnBotHeld3p(uint8_t ii) {
        //fade down all (similar) lights?
    /**
     * @brief 
        if on, fade down all on lights of same mode.
        if off, fade down all lights of all modes.
     * 
     */

    
    // /**
    if (section[ii].RGBWM[4]->isOn) { //if master is on

        for (uint8_t k = 0; k < SECTION_COUNT; k++) {

            if (section[k].mode == section[ii].mode && section[k].RGBWM[4]->isOn) {
                // for each section that's on and matches modes,
                
                //fade down simillar lights
                uint16_t s = calcSpeed(k, 0, FADE_AMT_BASE);
                if (section[k].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
                    fadeDecrement(k, s);

                    // if (section[k].RGBWM[4]->buffer >= 0 + amt) {
                    //     section[k].RGBWM[4]->buffer -= amt;     //
                        
                    // } else if (section[k].RGBWM[4]->buffer > 0) {
                    //     section[k].RGBWM[4]->lvl = section[k].RGBWM[4]->buffer + TABLE_SIZE - amt;
                    //     section[k].RGBWM[4]->buffer = 0;

                    // } else if (section[k].RGBWM[4]->lvl >= 0 + amt) {
                    //     section[k].RGBWM[4]->lvl -= amt;

                    // } else {
                    //     section[k].RGBWM[4]->lvl = 0; // min
                    // }

                    

                } else if (section[k].extendedFade) {
                    if (section[k].mode == LOW_CYCLE_STARTS_AT) { // white
                        for (uint8_t color = 0; color < 3; color++) {
                            fadeDecrement(k, s, color);

                                    
                            // if (section[k].RGBWM[color]->buffer > 0 + amt) {
                            //     section[k].RGBWM[color]->buffer -= amt;     //
                                
                            // } else if (section[k].RGBWM[color]->buffer > 0) {
                            //     section[k].RGBWM[color]->lvl = section[k].RGBWM[color]->buffer + TABLE_SIZE - amt;
                            //     section[k].RGBWM[color]->buffer = 0;

                            // } else if (section[k].RGBWM[color]->lvl > 0 + amt) {
                            //     section[k].RGBWM[color]->lvl -= amt;

                            // } else {
                            //     section[k].RGBWM[color]->lvl = 0; // min
                            // }
                            
                        }
                    } else {
                        fadeDecrement(k, s, 3);

                        // if (section[k].RGBWM[3]->buffer > 0 + amt) {
                        //     section[k].RGBWM[3]->buffer -= amt;     //
                            
                        // } else if (section[k].RGBWM[3]->buffer > 0) {
                        //     section[k].RGBWM[3]->lvl = section[k].RGBWM[3]->buffer + TABLE_SIZE - amt;
                        //     section[k].RGBWM[3]->buffer = 0;

                        // } else if (section[k].RGBWM[3]->lvl > 0 + amt) {
                        //     section[k].RGBWM[3]->lvl -= amt;

                        // } else {
                        //     section[k].RGBWM[3]->lvl = 0; // min
                        // }

                    }
                } else { // regular fade
                    if (section[k].mode == LOW_CYCLE_STARTS_AT || section[k].mode == (LOW_CYCLE_STARTS_AT + 1) || section[k].mode == (LOW_CYCLE_STARTS_AT + 2) || section[k].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // rgb smooth, sudden, combined
                        fadeDecrement(k, s);

                    } else if (section[k].mode == HIGH_CYCLE_STARTS_AT || section[k].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[k].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
                        uint8_t color = section[k].mode-HIGH_CYCLE_STARTS_AT;
                        fadeDecrement(k, s, color);
                    }
                }
                
                updateLights(k);
            }
        }
    }
    //  */
}