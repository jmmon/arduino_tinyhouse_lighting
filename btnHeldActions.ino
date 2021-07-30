void btnHeldActions(uint8_t ii, uint8_t bb) {   // happens every loop after "held delay (230ms)"
    if (!(section[ii].btn[bb].isHeld))
        section[ii].btn[bb].isHeld = true;

    if (bb == 1) {
        if (section[ii].btn[bb].pressCtr == 3) 
            btnTopHeld3p(ii);
        else if (section[ii].btn[bb].pressCtr == 2) 
            btnTopHeld2p(ii);
        else if (section[ii].btn[bb].pressCtr == 1) 
            btnTopHeld1p(ii);
        
    } else { // b == 0
        // disable extended fade while fading down (fades from extended thru to off)
        section[ii].DisableExtFade(); // bottom, fade down, causes dim without stopping
    
        if (section[ii].btn[bb].pressCtr == 3)
            btnBotHeld3p(ii);
        else if (section[ii].btn[bb].pressCtr == 2)
            btnBotHeld2p(ii);
        else if (section[ii].btn[bb].pressCtr == 1)
            btnBotHeld1p(ii);
        
    }
}



//**********************************************************************



void btnTopHeld1p(uint8_t ii) {
    if (!(section[ii].isOn)) {  // if fading up from off, turn "on" the correct light
        section[ii].isOn = true;
        if (section[ii].mode == 0) {
            section[ii].mode = LOW_CYCLE_STARTS_AT;
            section[ii].RGBWon[3] = true; //white on
            section[ii].RGBW[3] = 1;
            section[ii].masterLevel = 0.001; //fixes fade up from off
        } 
        
        else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) {
            section[ii].RGBWon[section[ii].mode-SINGLE_COLOR_MODE_OFFSET] = true;
        }
    }
    

    uint8_t factor = 1; // regular fade increment
    
    if (currentTime >= (section[ii].btn[1].timePressed + (BTN_HELD_DELAY * 3))) {
        factor = 4; // double a second time for triple time (so FADE_AMOUNT * (2*2))
    } else if (currentTime >= (section[ii].btn[1].timePressed + (BTN_HELD_DELAY * 2))) {
        factor = 2; // double amount after double time
    }


    if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
        section[ii].masterInc(factor);
    } else if (section[ii].fadeExtended) {
        //extended fade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT) { // white
            for (uint8_t color = 0; color < 3; color++) {
                section[ii].colorInc(factor, color); // rgb
            }
        } else {
            section[ii].colorInc(factor, 3);
        }
    } else { // regular fade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT || section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // white, rgb smooth, sudden;  combined
            section[ii].masterInc(factor);

        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
            section[ii].colorInc(factor, section[ii].mode-SINGLE_COLOR_MODE_OFFSET);
        }
    }
}

void btnTopHeld2p(uint8_t ii) {
    //fade up all (similar) lights?
    /**
    if (section[ii].isOn) {
        for (uint8_t section = 0; section < LIGHT_SECTION_COUNT; section++) {
            if (section[section].mode == section[ii].mode) {
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
    if ((section[ii].mode == (LOW_CYCLE_STARTS_AT + 1)) || (section[ii].mode == (LOW_CYCLE_STARTS_AT + 2))) {
        // speed up colorProgress (decrease delay)
        if (colorLoopDelayCtr == COLOR_LOOP_DELAY_CTR_INT) {   
            if (section[ii].colorDelayInt > 1)
                section[ii].colorDelayInt--;
            
            colorLoopDelayCtr = 0;

        } else 
            colorLoopDelayCtr++;
    }
}


    

void btnBotHeld1p(uint8_t ii) {
    //first fade down rgb/white if on, then the current mode fade down.
    uint8_t factor = 1; // regular fade decrement
    if (currentTime >= (section[ii].btn[0].timePressed + (BTN_HELD_DELAY * 3)))
        factor = 4; // quadruple speed after triple time

    else if (currentTime >= (section[ii].btn[0].timePressed + (BTN_HELD_DELAY * 2))) 
        factor = 2; // double delay speed after double time



    if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 3)) { // max brightness
        section[ii].masterDec(factor);

    } else if (section[ii].fadeExtended) { // extended fade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT) { // white
            for (uint8_t color = 0; color < 3; color++) {
                section[ii].colorDec(factor, color); // rgb
            }
        } else {
            section[ii].colorDec(factor, 3);
        }

    } else {//regular fade
        if (section[ii].mode == LOW_CYCLE_STARTS_AT || section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 3)) { // rgb smooth, sudden, combined
            section[ii].masterDec(factor);

        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
            uint8_t color = section[ii].mode-SINGLE_COLOR_MODE_OFFSET;
            section[ii].colorDec(factor, color);
        }
    }
}


void btnBotHeld2p(uint8_t ii) {
    //fade down all (similar) lights?
    /**
    if (section[ii].isOn) {
        for (uint8_t section = 0; section < LIGHT_SECTION_COUNT; section++) {
            if (section[section].mode == section[ii].mode) {
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
    if ((section[ii].mode == (LOW_CYCLE_STARTS_AT + 1)) || (section[ii].mode == (LOW_CYCLE_STARTS_AT + 2))) {
        // slow down colorProgress (increase delay)
        if (colorLoopDelayCtr == COLOR_LOOP_DELAY_CTR_INT) {   
            colorLoopDelayCtr = 0;

            if (section[ii].colorDelayInt < 20000) 
                section[ii].colorDelayInt++;
            
        } else 
            colorLoopDelayCtr++;
    }
}