void btnActions(uint8_t ii, uint8_t bb) {
    section[ii]._btn[bb]->timeReleased = 0;

    if (section[ii]._btn[bb]->isHeld) {  // runs once if the button was released from a hold
        section[ii]._btn[bb]->isHeld = false;
    }

    // has to go here, if it's under holdActions it doesn't break the fade, just allows 0 thru max extended
    if (bb == 0) { // bottom (fade down)
        if (section[ii].extendedFade) {
            //check if mode's extended level is max and if so disable extendedFade
            switch(section[ii].mode) {
                case(0): // white 
                    // if rgb are 0, disable extended fade
                    // if (section[ii].RGBW[0] <= 0 && section[ii].RGBW[1] <= 0 && section[ii].RGBW[2] <= 0) {
                    if ( section[ii].RGBW[0] <= 0 && section[ii].RGBW[1] <= 0 && section[ii].RGBW[2] <= 0 ) { //RGB is extended fade
                        section[ii].extendedFade = false;
                    } // else hasn't hit min yet
                break;

                case(3): // max brightness
                    //no change
                break;
                
                case(1): // RGB Smooth
                case(2): // RGB Sudden
                case(4): // Red
                case(5): // Green
                case(6): // Blue
                case(7): // Combined
                    if (section[ii].RGBW[3] <= 0) { // white is extended fade, wait for this to be off
                        section[ii].extendedFade = false;
                    } // else hasn't hit min yet
                break;
            }
        }
        
    } else { // top
        if (!(section[ii].extendedFade)) {  // if extendedFade is not enabled yet for this section
            //check if mode's regular level is max and if so enable extendedFade
            switch(section[ii].mode) {
                case(0): // white 
                    if (section[ii].RGBW[3] >= 1) {
                        section[ii].extendedFade = true;
                    } // else hasn't hit max yet
                break;

                case(1): // RGB Smooth
                case(2): // RGB Sudden
                    if (section[ii].masterLevel >= 1) {
                        section[ii].extendedFade = true;
                    } // else hasn't hit max yet
                break;

                case(3): // max brightness
                    //no change
                break;

                case(4): // Red
                case(5): // Green
                case(6): // Blue
                    if (section[ii].RGBW[section[ii].mode-4] >= 1) {
                        section[ii].extendedFade = true;
                    } // else hasn't hit max yet
                break;

                case(7): // Combined
                    //if fading up and any of these hit max, we're at max. So enable extended fade
                    if ( (section[ii].RGBW[0] >= 1) || (section[ii].RGBW[1] >= 1) || (section[ii].RGBW[2] >= 1) ) {
                        section[ii].extendedFade = true;
                    } // else hasn't hit max yet
                break;
            }
        }
    }
    
    else {                              // else this runs if it was not being held, regular button release!
        if (section[ii]._btn[bb]->pressCt > MAX_PRESS_COUNT) 
            section[ii]._btn[bb]->pressCt = MAX_PRESS_COUNT;

        if (DEBUG) {
            if (bb == 1) 
                Serial.print(F(" TOP "));
            else
                Serial.print(F(" BOT "));
            Serial.print(section[ii]._btn[bb]->pressCt);
            Serial.println(F(" "));
        }

        if (bb == 1) {  // top button actions
            switch(section[ii]._btn[bb]->pressCt) {
                case (3):
                    topAction3presses(ii, bb);
                    if (DEBUG) {
                        Serial.println(F("Max Brightness {mode:3}"));
                    }
                break;

                case (2):
                    topAction2presses(ii);
                break;

                case (1):
                    topAction1press(ii);
                break;
            }

        } else {  // bottom button actions
            switch (section[ii]._btn[bb]->pressCt) {
                case (3):
                    botAction3presses(ii, bb);
                break;

                case (2):
                    botAction2presses(ii);
                break;

                case (1):
                    botAction1press(ii);
                break;
            }
        }
    }
    
    section[ii]._btn[bb]->pressCt = 0;
}




//*********************************************************************************************************

//                  TOP PRESS ACTIONS
//  These happen after BTN_RELEASE_TIMER time has passed after a button has been released.
//*********************************************************************************************************

// PRESS TOP from   off :   turn on mode0 to default (or to lastBrightness if available)
// PRESS TOP from white / single colors / max brightness:   bump up brightness
// PRESS TOP from   RGB :   pause colorProgress
void topAction1press(uint8_t ii) {
    if (section[ii].isOn) { // from on:
        switch (section[ii].mode) {
            case (1): // RGB
            case (2): // RGBW
            // pause colorProgress
                section[ii].colorProgress = !section[ii].colorProgress;
            break;

            default: // for other modes, cycle up brightness for colors that are on
                section[ii].masterLevel += 0.2;

                if (section[ii].masterLevel > 1)
                    section[ii].masterLevel = 1;
            break;
        }
        updateLights(ii);

    } else { // from off:
        // TODO: use last brightness if available, else default brightness
        section[ii].mode = 0;
        switchMode(ii);
    }
}


// DOUBLE PRESS TOP: turn on and switch to next mode.
void topAction2presses(uint8_t ii) {
    if (section[ii].mode >= 4) {  // +1 to adjust for the previous line happening before instead of after
        section[ii].mode ++;
        if (section[ii].mode > MAX_MODE_SINGLE_COLOR)
            section[ii].mode = 4;
    }
    else {
        section[ii].mode ++;
        if (section[ii].mode >= NUM_OF_MODES_CYCLE)
            section[ii].mode = 0;
    }
    switchMode(ii);
}

//  TRIPLE PRESS TOP: MAX BRIGHTNESS
//from on/off (any mode): turn on max brightness
void topAction3presses(uint8_t ii, uint8_t bb) {
    section[ii].mode = 3; // max brightness mode
    switchMode(ii);
}




//*********************************************************************************************************

//                  BOTTOM PRESS ACTIONS
//  These happen after BTN_RELEASE_TIMER time has passed after a button has been released.
//*********************************************************************************************************


// PRESS BOT from off: turn on nightlight mode (red and/or ww)
// PRESS BOT from on: turn off all lights, reset mode;
void botAction1press(uint8_t n) {
    if (section[n].isOn) { // turn off light
        if (DEBUG) {
            Serial.println(F(" BOT 1: Turn Off "));
        }
        turnOffSection(n);

    } else { // turn on night light mode
        if (DEBUG) {
            Serial.println(F(" BOT 1: Red "));
        }

        section[n].mode = 4;
        switchMode(n);
    }
}


// DOUBLE PRESS BOT: from on: reduce mode by 1
// DOUBLE PRESS BOT: from off: do nothing
void botAction2presses(uint8_t ii) {
    if (section[ii].isOn) {
        
        if (section[ii].mode >= 4) { // -1 to adjust for previous line happening before instead of after
            section[ii].mode --;
            if (section[ii].mode < 4) 
                section[ii].mode = MAX_MODE_SINGLE_COLOR;
            
        }   
            
        else {      // mode is 0-3
            section[ii].mode --;
            if (section[ii].mode < 0) 
                section[ii].mode = NUM_OF_MODES_CYCLE - 1;
        } 
        switchMode(ii);
    
    } //else do nothing from off
}


void botAction3presses(uint8_t n, uint8_t m) {
    bool somethingElseIsOn = false;
    for (uint8_t k = 0; k < SECTION_COUNT; k++) 
        if (section[k].isOn && (k != 2)) 
            somethingElseIsOn = true;

    if (somethingElseIsOn) {
        // something else on, (porch on or off)
        for (uint8_t k = 0; k < SECTION_COUNT; k++) // turn off everything except porch
            if (section[k].isOn && (k != 2))
                turnOffSection(k);

    } else {
        if (section[2].isOn)
            // nothing else on, porch on
            turnOffSection(2);

        else
            // nothing else on, porch off
            // DISCO MODE
            for (uint8_t k = 0; k < SECTION_COUNT; k++) {
                // for each light, switch the mode to 2 (colorProgress sudden)
                section[k].mode = 2;
                switchMode(k);
            }
    }
}