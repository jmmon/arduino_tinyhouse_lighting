void btnRelease(uint8_t ii, uint8_t bb) {
    // happens after a short delay following a button release
    section[ii]._btn[bb]->timeReleased = 0;

    if (section[ii]._btn[bb]->isHeld) {  // runs once if the button was released from a hold
        section[ii]._btn[bb]->isHeld = false;
    
        // if max brightness was hit last button release, enable extended fade for next fade
        if (bb == 1) { // top (fade up)
            extendedFade(ii);
        }
    
    } else { // else this runs if it was not being held (regular press and release)

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
            if (section[ii]._btn[bb]->pressCt == 3) {
                topAction3presses(ii, bb);
                if (DEBUG) {
                    Serial.println(F("Max Brightness {mode:3}"));
                }
            }else if (section[ii]._btn[bb]->pressCt == 2) {
                topAction2presses(ii);
            }else if (section[ii]._btn[bb]->pressCt == 1) {
                topAction1press(ii);
            }
        } else {  // bottom button actions
            if (section[ii]._btn[bb]->pressCt == 3) {
                botAction3presses(ii, bb);
                if (DEBUG) {
                    Serial.println(F("Max Brightness {mode:3}"));
                }
            }else if (section[ii]._btn[bb]->pressCt == 2) {
                botAction2presses(ii);
            }else if (section[ii]._btn[bb]->pressCt == 1) {
                botAction1press(ii);
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
        if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2)) { //rgb, rgb
            // pause colorProgress
            section[ii].colorProgress = !section[ii].colorProgress;
        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
            section[ii].RGBW[section[ii].mode - SINGLE_COLOR_MODE_OFFSET] += 0.2;
            if (section[ii].RGBW[section[ii].mode-SINGLE_COLOR_MODE_OFFSET] > 1)
                section[ii].RGBW[section[ii].mode-SINGLE_COLOR_MODE_OFFSET] = 1;

        } else {
            // for other modes, cycle up brightness for colors that are on
            section[ii].masterLevel += 0.2;
            if (section[ii].masterLevel > 1)
                section[ii].masterLevel = 1;
        }
        
        updateLights(ii);

    } else { // from off:
        // TODO: use last brightness if available, else default brightness
        section[ii].mode = LOW_CYCLE_STARTS_AT;
        switchMode(ii);
    }
}


// DOUBLE PRESS TOP: turn on and switch to next mode.
void topAction2presses(uint8_t ii) {
    if (section[ii].mode >= HIGH_CYCLE_STARTS_AT) {     // CYCLE_HIGH
        section[ii].mode ++;
        if (section[ii].mode >= (HIGH_CYCLE_STARTS_AT + NUM_OF_MODES_IN_CYCLE_HIGH))
            section[ii].mode = HIGH_CYCLE_STARTS_AT;

    } else {                                            // CYCLE_LOW
        section[ii].mode ++;
        if (section[ii].mode >= (LOW_CYCLE_STARTS_AT + NUM_OF_MODES_IN_CYCLE_LOW) )
            section[ii].mode = LOW_CYCLE_STARTS_AT;
    }
    switchMode(ii);
}

//  TRIPLE PRESS TOP: MAX BRIGHTNESS
// from on/off (any mode): turn on max brightness
void topAction3presses(uint8_t ii, uint8_t bb) {
    section[ii].mode = (LOW_CYCLE_STARTS_AT + 3); // max brightness mode
    switchMode(ii);
}




//*********************************************************************************************************

//                  BOTTOM PRESS ACTIONS
//  These happen after BTN_RELEASE_TIMER time has passed after a button has been released.
//*********************************************************************************************************


// PRESS BOT from off: turn on nightlight mode (red and/or ww)
// PRESS BOT from on: turn off all lights, reset mode;
void botAction1press(uint8_t n) {
    if (section[n].isOn) { // if on turn off
        if (DEBUG) {
            Serial.println(F(" BOT 1: Turn Off "));
        }
        //turnOffSection(n);
        section[n].mode = 0;
        switchMode(n);
        //add mode for turn off?

    } else { // turn on night light mode
        if (DEBUG) {
            Serial.println(F(" BOT 1: Red "));
        }

        section[n].mode = HIGH_CYCLE_STARTS_AT;
        switchMode(n);
    }
}


// DOUBLE PRESS BOT: from on: reduce mode by 1
// DOUBLE PRESS BOT: from off: do nothing
void botAction2presses(uint8_t ii) {
    if (section[ii].isOn) { // do nothing if off
        if (section[ii].mode >= HIGH_CYCLE_STARTS_AT) { // if in the single color modes
            section[ii].mode --;
            if (section[ii].mode < HIGH_CYCLE_STARTS_AT)    // if under 5
                section[ii].mode = (HIGH_CYCLE_STARTS_AT + NUM_OF_MODES_IN_CYCLE_HIGH - 1);   // 5 + 4 - 1 = 8
            
        } else {      // mode is 1-3
            section[ii].mode --;
            if (section[ii].mode < LOW_CYCLE_STARTS_AT) // if under 1
                section[ii].mode = (LOW_CYCLE_STARTS_AT + NUM_OF_MODES_IN_CYCLE_LOW - 1);   //4 - 1 = 3
        } 

        switchMode(ii);
    }
}


void botAction3presses(uint8_t n, uint8_t m) {
    bool somethingElseIsOn = false;
    for (uint8_t index = 0; index < SECTION_COUNT; index++) 
        if (section[index].isOn && (index != 2)) 
            somethingElseIsOn = true;

    if (somethingElseIsOn) { // something else on, (porch on or off)
        for (uint8_t index = 0; index < SECTION_COUNT; index++) // turn off everything except porch
            if (section[index].isOn && (index != 2)) {
                //turnOffSection(index);
                section[index].mode = 0;
                switchMode(index);
            }
                

    } else {
        if (section[2].isOn) { // nothing else on, porch on
            //turnOffSection(2);
            section[2].mode = 0;
            switchMode(2);
        }
        else // DISCO MODE // nothing else on, porch off
            for (uint8_t index = 0; index < SECTION_COUNT; index++) {
                // for each light, switch the mode to (LOW_CYCLE_STARTS_AT + 2) (colorProgress sudden)
                section[index].mode = (LOW_CYCLE_STARTS_AT + 2);
                switchMode(index);
            }
    }
}
