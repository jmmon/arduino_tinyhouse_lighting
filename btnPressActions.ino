void btnActions(uint8_t ii, uint8_t bb) {
    section[ii]._btn[bb]->timeReleased = 0;

    if (section[ii]._btn[bb]->isHeld) {  // runs once if the button was released from a hold
        section[ii]._btn[bb]->isHeld = false;

        //if max brightness, enable extended fade
        //(if min brightness, disable extended fade)
        if (bb == 0) {
            //bottom
            if (section[ii].extendedFade) {
                if (section[ii].mode == 0) {
                    if (section[ii].RGBW[3] <= 0) {
                        section[ii].extendedFade = false;
                    } // else hasn't hit min yet
                } else if (section[ii].mode == (1 || 2)) {
                    if (section[ii].masterLevel <= 0) {
                        section[ii].extendedFade = false;
                    } // else hasn't hit min yet
                } else if (section[ii].mode == (4 || 5 || 6)) {
                    if (section[ii].RGBW[section[ii].mode - 4] <= 0) {
                        section[ii].extendedFade = false;
                    } // else hasn't hit min yet
                }
            }
            
        } else {
            if (!(section[ii].extendedFade)) {
                if (section[ii].mode == 0) {
                    if (section[ii].RGBW[3] >= 1) {
                        section[ii].extendedFade = true;
                    } // else hasn't hit max yet
                } else if (section[ii].mode == (1 || 2)) {
                    if (section[ii].masterLevel >= 1) {
                        section[ii].extendedFade = true;
                    } // else hasn't hit max yet
                } else if (section[ii].mode == (4 || 5 || 6)) {
                    if (section[ii].RGBW[section[ii].mode - 4] >= 1) {
                        section[ii].extendedFade = true;
                    } // else hasn't hit max yet
                }
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
    section[ii].mode ++;
    if (section[ii].mode >= 5)  // +1 to adjust for the previous line happening before instead of after
        if (section[ii].mode > MAX_MODE_SINGLE_COLOR)
            section[ii].mode = 4;

    else 
        if (section[ii].mode >= NUM_OF_MODES_CYCLE)
            section[ii].mode = 0;
    
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
        section[ii].mode --;
        if (section[ii].mode >= 3)    // -1 to adjust for previous line happening before instead of after
            if (section[ii].mode < 4) 
                section[ii].mode = MAX_MODE_SINGLE_COLOR;
            
        else       // mode is 0-3
            if (section[ii].mode < 0) 
                section[ii].mode = NUM_OF_MODES_CYCLE - 1;
            
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