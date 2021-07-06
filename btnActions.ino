const uint8_t MAX_PRESS_COUNT = 3;              // single, double, triple press
const uint8_t MAX_MODE_SINGLE_COLOR = 7;    // red==4, green==5, blue==6, TODO: combined==7;
const uint8_t NUM_OF_MODES_CYCLE = 3;


void btnActions(uint8_t ii, uint8_t bb) {
    // happens after a short delay following a button release
    section[ii]._btn[bb]->timeReleased = 0;

    if (section[ii]._btn[bb]->isHeld) {  // runs once if the button was released from a hold
        section[ii]._btn[bb]->isHeld = false;
    
        // if max brightness was hit last button release, enable extended fade for next fade
        if (bb == 1) { // top (fade up)
            extendedFade(ii);
        }
    
    } else {                              // else this runs if it was not being held, regular button release!
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
        if (section[ii].mode == 1 || section[ii].mode == 2) { //rgb, rgb
            // pause colorProgress
            section[ii].colorProgress = !section[ii].colorProgress;
        } else {


            // need reworking? to use something other than masterLevel? Not sure. Working well for now!


            // for other modes, cycle up brightness for colors that are on
            section[ii].masterLevel += 0.2;

            if (section[ii].masterLevel > 1)
                section[ii].masterLevel = 1;

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
    if (section[ii].mode >= 4) {
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

    if (somethingElseIsOn) { // something else on, (porch on or off)
        for (uint8_t k = 0; k < SECTION_COUNT; k++) // turn off everything except porch
            if (section[k].isOn && (k != 2))
                turnOffSection(k);

    } else {
        if (section[2].isOn) // nothing else on, porch on
            turnOffSection(2);

        else // DISCO MODE // nothing else on, porch off
            for (uint8_t k = 0; k < SECTION_COUNT; k++) {
                // for each light, switch the mode to 2 (colorProgress sudden)
                section[k].mode = 2;
                switchMode(k);
            }
    }
}