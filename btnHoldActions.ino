void btnHeldActions(uint8_t ii, uint8_t bb) {   // happens every loop while held
    if (!(section[ii]._btn[bb]->isHeld))
        section[ii]._btn[bb]->isHeld = true;

    //if max brightness when fading up, enable extended fade
    //(if min brightness when fading down, disable extended fade)
    

    if (bb == 1) {
        switch (section[ii]._btn[bb]->pressCt) {
            case (3):
                btnTopHeld3p(ii);
            break;

            case (2):
                btnTopHeld2p(ii);
            break;

            case (1):
                btnTopHeld1p(ii);
            break;
        }
    } else {// b == 0
        switch (section[ii]._btn[bb]->pressCt) {
            case (3):
                btnBotHeld3p(ii);
            break;

            case (2):
                btnBotHeld2p(ii);
            break;

            case (1):
                btnBotHeld1p(ii);
            break;
        }
    }
}







void btnTopHeld1p(uint8_t ii) {
    if (!(section[ii].isOn)) {  // if turning on from off
        section[ii].isOn = true;
        section[ii].colorProgress = false;
        if (section[ii].mode == 0) {
            section[ii].RGBWon[3] = true;
        } else if (section[ii].mode == (4 || 5 || 6)) {
            section[ii].RGBWon[section[ii].mode-4] = true;
        }
    }
    
    float tempSpeed = FADE_AMOUNT; // regular fade increment
    if (currentTime >= (section[ii]._btn[0]->timePressed + (BTN_FADE_DELAY * 3))) 
        tempSpeed = FADE_AMOUNT * 2 * 2; // quadruple amount after triple time

    else if (currentTime >= (section[ii]._btn[0]->timePressed + (BTN_FADE_DELAY * 2))) 
        tempSpeed = FADE_AMOUNT * 2; // double amount after double time



    if (section[ii].mode == 0) { // white
        if (section[ii].extendedFade) {
            fadeIncrement(ii, tempSpeed, 0);
            fadeIncrement(ii, tempSpeed, 1);
            fadeIncrement(ii, tempSpeed, 2);

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

    }else if (section[ii].mode == 4 || section[ii].mode == 5 || section[ii].mode == 6) { // r, g, b
        if (section[ii].extendedFade) {
            fadeIncrement(ii, tempSpeed, 3);

        } else {
            fadeIncrement(ii, tempSpeed, section[ii].RGBW[section[ii].mode-4]);
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
            fadeDecrement(ii, tempSpeed, 0); // r
            fadeDecrement(ii, tempSpeed, 1); // g
            fadeDecrement(ii, tempSpeed, 2); // b

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
            fadeDecrement(ii, tempSpeed, section[ii].RGBW[section[ii].mode-4]);
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