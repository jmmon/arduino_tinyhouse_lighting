void btnHeldActions(uint8_t ii, uint8_t bb) {   // happens every loop while held
    if (!(section[ii]._btn[bb]->isHeld))
        section[ii]._btn[bb]->isHeld = true;

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
    if (!(section[ii])) {  // if turning on from off
        section[ii].isOn = true;
        section[nn].colorProgress = false;
        section[ii].RGBW[3] = 1;
        section[ii].RGBWon[3] = true;
    }
    
    float tempSpeed = 0;
    if (currentTime >= (section[ii]._btn[1]->timePressed + BTN_FADE_DELAY_RAPID2))
        tempSpeed = FADE_FACTOR_RAPID; // after QUICK_DELAY time, adjust quadruple fast

    else if (currentTime >= (section[ii]._btn[1]->timePressed + BTN_FADE_DELAY_RAPID))
        tempSpeed = FADE_FACTOR_RAPID; // after QUICK_DELAY time, adjust double fast
        
    else
        tempSpeed = FADE_FACTOR_RAPID;       //increment

    masterFadeIncrement(ii, tempSpeed);

    // switch(section[ii].mode) {
    //     case(0):
    //         //white
    //         if (section[ii].extendedFade) {
    //             fadeIncrement(ii, tempSpeed, 0);
    //             fadeIncrement(ii, tempSpeed, 1);
    //             fadeIncrement(ii, tempSpeed, 2);
    //         } else {
    //             fadeIncrement(ii, tempSpeed, 3);
    //         }
    //     break;
    //     case(1):
    //     case(2):
    //         //rgb smooth
    //         //rgb sudden
    //         if (section[ii].extendedFade) {
    //             fadeIncrement(ii, tempSpeed, 3);
    //         } else {
    //             masterFadeIncrement(ii, tempSpeed);
    //         }
    //     break;
    //     case(3):
    //         masterFadeIncrement(ii, tempSpeed);
    //         //max
    //     break;
    //     case(4): // red
    //     case(5): // green
    //     case(6): // blue
    //         if (section[ii].extendedFade) {
    //             fadeIncrement(ii, tempSpeed, 3);
    //         } else {
    //             fadeIncrement(ii, tempSpeed, section[ii].RGBW[section[ii].mode]-4);
    //         }
    //     break;
    //     case(7):
    //         //combined
    //         if (section[ii].extendedFade) {
    //             fadeIncrement(ii, tempSpeed, 3);
    //         } else {
    //             masterFadeIncrement(ii, tempSpeed);
    //         }
    //     break;
    // }
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
    float tempSpeed = 0;
    if (currentTime >= (section[ii]._btn[0]->timePressed + BTN_FADE_DELAY_RAPID2)) 
        tempSpeed = FADE_FACTOR_RAPID2; // after QUICK_DELAY time, adjust quadruple fast

    else if (currentTime >= (section[ii]._btn[0]->timePressed + BTN_FADE_DELAY_RAPID)) 
        tempSpeed = FADE_FACTOR_RAPID; // after QUICK_DELAY time, adjust double fast

    else {
        tempSpeed = FADE_FACTOR;       // regular decrement
    }

    masterFadeDecrement(ii, tempSpeed);

    // switch(section[ii].mode) {
    //     case(0):
    //         //white
    //         if (section[ii].extendedFade) {
    //             fadeDecrement(ii, tempSpeed, 0);
    //             fadeDecrement(ii, tempSpeed, 1);
    //             fadeDecrement(ii, tempSpeed, 2);
    //         } else {
    //             fadeDecrement(ii, tempSpeed, 3);
    //         }
    //     break;
    //     case(1):
    //     case(2):
    //         //rgb smooth
    //         //rgb sudden
    //         if (section[ii].extendedFade) {
    //             fadeDecrement(ii, tempSpeed, 3);
    //         } else {
    //             masterFadeDecrement(ii, tempSpeed);
    //         }
    //     break;
    //     case(3):
    //         masterFadeDecrement(ii, tempSpeed);
    //         //max
    //     break;
    //     case(4): // red
    //     case(5): // green
    //     case(6): // blue
    //         if (section[ii].extendedFade) {
    //             fadeDecrement(ii, tempSpeed, 3);
    //         } else {
    //             fadeDecrement(ii, tempSpeed, section[ii].RGBW[section[ii].mode]-4);
    //         }
    //     break;
    //     case(7):
    //         //combined
    //         if (section[ii].extendedFade) {
    //             fadeDecrement(ii, tempSpeed, 3);
    //         } else {
    //             masterFadeDecrement(ii, tempSpeed);
    //         }
    //     break;
    // }
    
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