const float FADE_AMOUNT = 0.005;      // base fade adjustment; modified by section[].BRIGHTNESS_FACTOR
const uint8_t COLOR_LOOP_DELAY_CTR_INT = 5;    // 5 * 20ms (main loop time) per adjustment
uint16_t colorLoopDelayCtr = 0;


void btnHeldActions(uint8_t ii, uint8_t bb) {   // happens every loop after "held delay (230ms)"
    if (!(sectionC[ii]._btnC[bb]->isHeld))
        sectionC[ii]._btnC[bb]->isHeld = true;

    if (bb == 1) {
        if (sectionC[ii]._btnC[bb]->pressCt == 3) 
            btnTopHeld3p(ii);
        else if (sectionC[ii]._btnC[bb]->pressCt == 2) 
            btnTopHeld2p(ii);
        else if (sectionC[ii]._btnC[bb]->pressCt == 1) 
            btnTopHeld1p(ii);
        
    } else { // b == 0
        // disable extended fade while fading down (fades from extended thru to off)
        sectionC[ii].disableExtendedFade(); // bottom, fade down, causes dim without stopping
    
        if (sectionC[ii]._btnC[bb]->pressCt == 3)
            btnBotHeld3p(ii);
        else if (sectionC[ii]._btnC[bb]->pressCt == 2)
            btnBotHeld2p(ii);
        else if (sectionC[ii]._btnC[bb]->pressCt == 1)
            btnBotHeld1p(ii);
        
    }
}



//**********************************************************************



void btnTopHeld1p(uint8_t ii) {
    if (!(sectionC[ii].isOn)) {  // if fading up from off, turn "on" the correct light
        sectionC[ii].isOn = true;
        if (sectionC[ii].mode == 0) {
            sectionC[ii].mode = LOW_CYCLE_STARTS_AT;
            sectionC[ii].RGBWon[3] = true; //white on
            sectionC[ii].RGBW[3] = 1;
            sectionC[ii].masterLevel = 0.001; //fixes fade up from off
        } 
        
        else if (sectionC[ii].mode == HIGH_CYCLE_STARTS_AT || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) {
            sectionC[ii].RGBWon[sectionC[ii].mode-SINGLE_COLOR_MODE_OFFSET] = true;
        }
    }
    

    float tempSpeed = FADE_AMOUNT; // regular fade increment
    
    if (currentTime >= (sectionC[ii]._btnC[1]->timePressed + (BTN_FADE_DELAY * 3))) {
        tempSpeed *= 4; // double a second time for triple time (so FADE_AMOUNT * (2*2))
    } else if (currentTime >= (sectionC[ii]._btnC[1]->timePressed + (BTN_FADE_DELAY * 2))) {
        tempSpeed *= 2; // double amount after double time
    }


    if (sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
        sectionC[ii].masterFadeIncrement(tempSpeed);
        //fadeIncrement(ii, tempSpeed, NULL);
    } else if (sectionC[ii].extendedFade) {
        //extended fade
        if (sectionC[ii].mode == LOW_CYCLE_STARTS_AT) { // white
            for (uint8_t color = 0; color < 3; color++) {
                sectionC[ii].fadeIncrement(tempSpeed, color); // rgb
            }
        } else {
            sectionC[ii].fadeIncrement(tempSpeed, 3);
        }
    } else {
        //regular fade
        if (sectionC[ii].mode == LOW_CYCLE_STARTS_AT || sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // white, rgb smooth, sudden;  combined
            sectionC[ii].masterFadeIncrement(tempSpeed);
            //fadeIncrement(ii, tempSpeed, NULL);

        } else if (sectionC[ii].mode == HIGH_CYCLE_STARTS_AT || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
            sectionC[ii].fadeIncrement(tempSpeed, sectionC[ii].mode-SINGLE_COLOR_MODE_OFFSET);
        }
    }
}

void btnTopHeld2p(uint8_t ii) {
    //fade up all (similar) lights?
    /**
    if (sectionC[ii].isOn) {
        for (uint8_t section = 0; section < LIGHT_SECTION_COUNT; section++) {
            if (section[section].mode == sectionC[ii].mode) {
                switch(section[section].mode) {
                    case(1):
                        //fade up W
                        break;

                    case(2):
                    case(3):
                        //fade up RGB
                    
                        break;

                    case(4):
                        //fade up all
                        break;

                    case(252):
                    case(253):
                    case(254):
                        //fade up color
                        break;
                    
                    case(255):
                        //fade up combined
                        break;
                }
            }
        }
    }
     */
}

void btnTopHeld3p(uint8_t ii) {
    if ((sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 1)) || (sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 2))) {
        // speed up colorProgress (decrease delay)
        if (colorLoopDelayCtr == COLOR_LOOP_DELAY_CTR_INT) {   
            if (sectionC[ii].colorDelayInt > 1)
                sectionC[ii].colorDelayInt--;
            
            colorLoopDelayCtr = 0;

        } else 
            colorLoopDelayCtr++;
    }
}


    

void btnBotHeld1p(uint8_t ii) {
    //first fade down rgb/white if on, then the current mode fade down.
    float tempSpeed = FADE_AMOUNT; // regular fade decrement
    if (currentTime >= (sectionC[ii]._btnC[0]->timePressed + (BTN_FADE_DELAY * 3)))
        tempSpeed = FADE_AMOUNT * 2 * 2; // quadruple speed after triple time

    else if (currentTime >= (sectionC[ii]._btnC[0]->timePressed + (BTN_FADE_DELAY * 2))) 
        tempSpeed = FADE_AMOUNT * 2; // double delay speed after double time



    if (sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
        sectionC[ii].masterFadeDecrement(tempSpeed);
        //fadeDecrement(ii, tempSpeed, NULL);
    } else if (sectionC[ii].extendedFade) {
        //extended fade
        if (sectionC[ii].mode == LOW_CYCLE_STARTS_AT) { // white
            for (uint8_t color = 0; color < 3; color++) {
                sectionC[ii].fadeDecrement(tempSpeed, color); // rgb
            }
        } else {
            sectionC[ii].fadeDecrement(tempSpeed, 3);
        }
    } else {
        //regular fade
        if (sectionC[ii].mode == LOW_CYCLE_STARTS_AT || sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // rgb smooth, sudden, combined
            sectionC[ii].masterFadeDecrement(tempSpeed);
            //fadeDecrement(ii, tempSpeed, NULL);

        } else if (sectionC[ii].mode == HIGH_CYCLE_STARTS_AT || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || sectionC[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
            uint8_t color = sectionC[ii].mode-SINGLE_COLOR_MODE_OFFSET;
            sectionC[ii].fadeDecrement(tempSpeed, color);
        }
    }
}


void btnBotHeld2p(uint8_t ii) {
    //fade down all (similar) lights?
    /**
    if (sectionC[ii].isOn) {
        for (uint8_t section = 0; section < LIGHT_SECTION_COUNT; section++) {
            if (section[section].mode == sectionC[ii].mode) {
                switch(section[section].mode) {
                    case(1):
                        //fade down W
                        break;

                    case(2):
                    case(3):
                        //fade down RGB
                    
                        break;

                    case(4):
                        //fade down all
                        break;

                    case(252):
                    case(253):
                    case(254):
                        //fade down color
                        break;
                    
                    case(255):
                        //fade down combined
                        break;
                }
            }
        }
    }
     */
}


void btnBotHeld3p(uint8_t ii) {
    if ((sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 1)) || (sectionC[ii].mode == (LOW_CYCLE_STARTS_AT + 2))) {
        // slow down colorProgress (increase delay)
        if (colorLoopDelayCtr == COLOR_LOOP_DELAY_CTR_INT) {   
            colorLoopDelayCtr = 0;

            if (sectionC[ii].colorDelayInt < 20000) 
                sectionC[ii].colorDelayInt++;
            
        } else 
            colorLoopDelayCtr++;
    }
}