void btnRelease(uint8_t ii, uint8_t bb) {
    // happens after a short delay following a button release
    section[ii].btn[bb].timeReleased = 0;

    if (section[ii].btn[bb].isHeld) {  // runs once if the button was released from a hold
        section[ii].btn[bb].isHeld = false;
        // if max brightness was hit last button release, enable extended fade for next fade
        if (bb == 1) { // top (fade up)
            extendedFade(ii);
        }
    

    } else { // else this runs if it was not being held (regular press and release)
        if (section[ii].btn[bb].pressCt > MAX_PRESS_COUNT) 
            section[ii].btn[bb].pressCt = MAX_PRESS_COUNT;

        if (DEBUG) {
            Serial.print((bb == 1) ? F(" TOP ") : F(" BOT "));
            Serial.print(section[ii].btn[bb].pressCt); Serial.println(F(" "));
        }

        if (bb == 1) {  // top button actions
            if (section[ii].btn[bb].pressCt == 3) {
                topAction3presses(ii, bb);
                if (DEBUG) {
                    Serial.println(F("Max Brightness {mode:3}"));
                }
            } else if (section[ii].btn[bb].pressCt == 2) {
                topAction2presses(ii);
            } else if (section[ii].btn[bb].pressCt == 1) {
                topAction1press(ii);
            }

        } else {  // bottom button actions
            if (section[ii].btn[bb].pressCt == 3) {
                botAction3presses(ii, bb);
                if (DEBUG) {
                    Serial.println(F("Max Brightness {mode:3}"));
                }
            } else if (section[ii].btn[bb].pressCt == 2) {
                botAction2presses(ii);
            } else if (section[ii].btn[bb].pressCt == 1) {
                botAction1press(ii);
            }
        }
    }
    section[ii].btn[bb].pressCt = 0;
}




//*********************************************************************************************************

//                  TOP PRESS ACTIONS
//  These happen after BTN_RELEASE_TIMER time has passed after a button has been released.
//                  BOTTOM PRESS ACTIONS
//  These happen after BTN_RELEASE_TIMER time has passed after a button has been released.
//*********************************************************************************************************

// PRESS TOP from   off :   turn on mode0 to default (or to lastBrightness if available)
// PRESS TOP from white / single colors / max brightness:   bump up brightness
// PRESS TOP from   RGB :   pause colorProgress
void topAction1press(uint8_t ii) {
    if (section[ii].RGBWM[4]->isOn) {
        if (section[ii].mode == (LOW_CYCLE_STARTS_AT + 1) || section[ii].mode == (LOW_CYCLE_STARTS_AT + 2)) { //rgb, rgb
            section[ii].colorProgress = !section[ii].colorProgress; // pause colorProgress

        } else if (section[ii].mode == HIGH_CYCLE_STARTS_AT || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 1) || section[ii].mode == (HIGH_CYCLE_STARTS_AT + 2)) { // r, g, b
            if (section[ii].RGBWM[section[ii].mode-HIGH_CYCLE_STARTS_AT]->lvl > (TABLE_SIZE - 3277)) {
                section[ii].RGBWM[section[ii].mode-HIGH_CYCLE_STARTS_AT]->lvl = TABLE_SIZE;
            } else {
                section[ii].RGBWM[section[ii].mode - HIGH_CYCLE_STARTS_AT]->lvl += 3277;
            }
            
        } else {
            if (section[ii].RGBWM[4]->lvl > uint16_t(TABLE_SIZE - (1.0 * TABLE_SIZE / 5))) {
                section[ii].RGBWM[4]->lvl = TABLE_SIZE;
            } else {
                section[ii].RGBWM[4]->lvl += (1.0 * TABLE_SIZE / 5);
            }
        }
        
        updateLights(ii);

    } else { // from off: // TODO: use last brightness if available, else default brightness
        section[ii].mode = LOW_CYCLE_STARTS_AT;
        if (DEBUG) {
            Serial.print(F("Now in mode: ")); Serial.println(section[ii].mode);
        }
        section[ii].RGBWM[4]->isOn = true;
        switchToWhite(ii);
    }
}

// PRESS BOT from off: turn on nightlight mode (red and/or ww)
// PRESS BOT from on: turn off all lights, reset mode;
void botAction1press(uint8_t ii) {
    if (DEBUG) {
        Serial.println((section[ii].RGBWM[4]->isOn)? F(" BOT 1: Turn Off ") : F(" BOT 1: Red "));
    }

    section[ii].mode = (section[ii].RGBWM[4]->isOn) ? 
        0 : 
        HIGH_CYCLE_STARTS_AT;

    if (section[ii].RGBWM[4]->isOn) {
        switchToOff(ii);
    } else {
        switchToSingleColor(ii);
    }
}
//*********************************************************************************************************



// DOUBLE PRESS TOP: turn on and switch to next mode.
void topAction2presses(uint8_t ii) {
    if (section[ii].mode == 0) {
        section[ii].mode += 2;

    } else if (section[ii].mode >= HIGH_CYCLE_STARTS_AT) {     // CYCLE_HIGH
        section[ii].mode ++;
        if (section[ii].mode >= (HIGH_CYCLE_STARTS_AT + HIGH_CYCLE_MODE_CT))
            section[ii].mode = HIGH_CYCLE_STARTS_AT;

    } else {                                            // CYCLE_LOW
        section[ii].mode ++;
        if (section[ii].mode >= (LOW_CYCLE_STARTS_AT + LOW_CYCLE_MODE_CT) )
            section[ii].mode = LOW_CYCLE_STARTS_AT;
    }
    switchMode(ii);
}


// DOUBLE PRESS BOT: from on: reduce mode by 1
// DOUBLE PRESS BOT: from off: do nothing
void botAction2presses(uint8_t ii) {
    if (section[ii].RGBWM[4]->isOn) {
        section[ii].mode --;

        if ((section[ii].mode + 1) >= HIGH_CYCLE_STARTS_AT) { // if in the single color modes
            if (section[ii].mode < HIGH_CYCLE_STARTS_AT)    // if under 5
                section[ii].mode = (HIGH_CYCLE_STARTS_AT + HIGH_CYCLE_MODE_CT - 1);   // 5 + 4 - 1 = 8
            
        } else {      // mode is 1-3
            if (section[ii].mode < LOW_CYCLE_STARTS_AT) // if under 1
                section[ii].mode = (LOW_CYCLE_STARTS_AT + LOW_CYCLE_MODE_CT - 1);   //4 - 1 = 3
        } 

    } else {
        section[ii].mode = 3;
    }
    switchMode(ii);
}
//*********************************************************************************************************


//  TRIPLE PRESS
// TOP: MAX BRIGHTNESS
// from on/off (any mode): turn on max brightness
void topAction3presses(uint8_t ii, uint8_t bb) {
    section[ii].mode = (LOW_CYCLE_STARTS_AT + 3); // max brightness mode
    section[ii].RGBWM[4]->isOn = true;
    switchToMax(ii);
}

void botAction3presses(uint8_t n, uint8_t m) {
    bool somethingElseIsOn = false;
    for (uint8_t index = 0; index < SECTION_COUNT; index++) 
        if (section[index].RGBWM[4]->isOn && (index != 2)) 
            somethingElseIsOn = true;

    if (somethingElseIsOn) { // something else on, (porch on or off)
        for (uint8_t index = 0; index < SECTION_COUNT; index++) {// turn off everything except porch
            if (section[index].RGBWM[4]->isOn && (index != 2)) {
                section[index].mode = 0;
                switchToOff(index);
            }
        }

    } else {
        if (section[2].RGBWM[4]->isOn) { // nothing else on, porch on
            section[2].mode = 0;
            switchToOff(2);
        }
        else // DISCO MODE // nothing else on, porch off
            for (uint8_t index = 0; index < SECTION_COUNT; index++) {
                // for each light, switch the mode to (LOW_CYCLE_STARTS_AT + 2) (colorProgress sudden)
                section[index].mode = (LOW_CYCLE_STARTS_AT + 2);
                section[index].RGBWM[4]->isOn = true;
                switchToRGB(index);
            }
    }
}
