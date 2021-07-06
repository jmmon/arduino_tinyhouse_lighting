const float FADE_AMOUNT = 0.005;      // base fade adjustment; modified by section[].BRIGHTNESS_FACTOR
const uint8_t COLOR_LOOP_DELAY_CTR_INT = 5;    // 5 * 20ms (main loop time) per adjustment
uint16_t colorLoopDelayCtr = 0;


void btnHeldActions(uint8_t ii, uint8_t bb) {   // happens every loop while held
    if (!(section[ii]._btn[bb]->isHeld))
        section[ii]._btn[bb]->isHeld = true;

    // disable extended fade while fading down (fades from extended thru to off)
    if (bb == 0) { // bottom, fade down
        disableExtendedFade(ii);
    }

    if (bb == 1) {
        if (section[ii]._btn[bb]->pressCt == 3) {
            btnTopHeld3p(ii);
        } else if (section[ii]._btn[bb]->pressCt == 2) {
            btnTopHeld2p(ii);
        } else if (section[ii]._btn[bb]->pressCt == 1) {
            btnTopHeld1p(ii);
        }
    } else { // b == 0
        if (section[ii]._btn[bb]->pressCt == 3) {
            btnBotHeld3p(ii);
        } else if (section[ii]._btn[bb]->pressCt == 2) {
            btnBotHeld2p(ii);
        } else if (section[ii]._btn[bb]->pressCt == 1) {
            btnBotHeld1p(ii);
        }
    }
}







void btnTopHeld1p(uint8_t ii) {
    if (!(section[ii].isOn)) {  // if turning on from off (usually mode==0 but sometimes mode==4||5||6)
        section[ii].isOn = true;
        if (section[ii].mode == 0) {
            section[ii].RGBWon[3] = true;
        } else if (section[ii].mode == 4 || section[ii].mode == 5 || section[ii].mode == 6) {
            section[ii].RGBWon[section[ii].mode-4] = true;
        }
    }
    
    
    // float tempSpeed = FADE_AMOUNT; // regular fade increment
    // uint8_t increment = uint8_t( (currentTime - section[ii]._btn[1]->timePressed) / BTN_FADE_DELAY ) - 1;   //should be 2, 1, 0; hopefully rounds
    // while (increment > 0 && increment < 6) {
    //     tempSpeed = FADE_AMOUNT * 2; // double amount after double time
    //     increment--;
    // }


    float tempSpeed = FADE_AMOUNT; // regular fade increment
    
    if (currentTime >= (section[ii]._btn[1]->timePressed + (BTN_FADE_DELAY * 3))) {
        tempSpeed = FADE_AMOUNT * 2 * 2; // double a second time for triple time (so FADE_AMOUNT * (2*2))
    } else if (currentTime >= (section[ii]._btn[1]->timePressed + (BTN_FADE_DELAY * 2))) {
        tempSpeed = FADE_AMOUNT * 2; // double amount after double time
    }

    if (section[ii].mode == 0) { // white
        if (section[ii].extendedFade) {
            for (uint8_t color = 0; color < 3; color++) {
                fadeIncrement(ii, tempSpeed, color); // rgb
            }
        } else {
            fadeIncrement(ii, tempSpeed, 3);
        }

    } else if (section[ii].mode == 1 || section[ii].mode == 2 || section[ii].mode == 7) { // rgb smooth, sudden, combined
        if (section[ii].extendedFade) {
            fadeIncrement(ii, tempSpeed, 3);
        } else {
            masterFadeIncrement(ii, tempSpeed);
        }

    } else if (section[ii].mode == 3) {
        masterFadeIncrement(ii, tempSpeed);

    } else if (section[ii].mode == 4 || section[ii].mode == 5 || section[ii].mode == 6) { // r, g, b
        if (section[ii].extendedFade) {
            fadeIncrement(ii, tempSpeed, 3);
        } else {
            fadeIncrement(ii, tempSpeed, section[ii].mode-4);
        }
    }
}

void btnTopHeld2p(uint8_t ii) {
    //fade down all lights?
}

void btnTopHeld3p(uint8_t ii) {
    if ((section[ii].mode == 1) || (section[ii].mode == 2)) {
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
    float tempSpeed = FADE_AMOUNT; // regular fade decrement
    if (currentTime >= (section[ii]._btn[0]->timePressed + (BTN_FADE_DELAY * 3)))
        tempSpeed = FADE_AMOUNT * 2 * 2; // quadruple speed after triple time

    else if (currentTime >= (section[ii]._btn[0]->timePressed + (BTN_FADE_DELAY * 2))) 
        tempSpeed = FADE_AMOUNT * 2; // double delay speed after double time


    if (section[ii].mode == 0) { // white
        if (section[ii].extendedFade) {
            for (uint8_t color = 0; color < 3; color++) {
                fadeDecrement(ii, tempSpeed, color); // rgb
            }
        } else {
            fadeDecrement(ii, tempSpeed, 3); // w
        }

    } else if (section[ii].mode == 1 || section[ii].mode == 2 || section[ii].mode == 7) { // RGB smooth, sudden, combined
        if (section[ii].extendedFade) {
            fadeDecrement(ii, tempSpeed, 3);
        } else {
            masterFadeDecrement(ii, tempSpeed);
        }

    } else if (section[ii].mode == 3) { // max
        masterFadeDecrement(ii, tempSpeed);

    } else if (section[ii].mode == 4 || section[ii].mode == 5 || section[ii].mode == 6) { //r, g, b
        if (section[ii].extendedFade) {
            fadeDecrement(ii, tempSpeed, 3);
        } else {
            fadeDecrement(ii, tempSpeed, section[ii].mode-4);
        }
    }
}


void btnBotHeld2p(uint8_t ii) {
    //fade up all lights?
}


void btnBotHeld3p(uint8_t ii) {
    if ((section[ii].mode == 1) || (section[ii].mode == 2)) {
        // slow down colorProgress (increase delay)
        if (colorLoopDelayCtr == COLOR_LOOP_DELAY_CTR_INT) {   
            colorLoopDelayCtr = 0;

            if (section[ii].colorDelayInt < 20000) 
                section[ii].colorDelayInt++;
            
        } else 
            colorLoopDelayCtr++;
    }
}