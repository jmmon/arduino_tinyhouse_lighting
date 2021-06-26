void btnBotHeld(uint8_t ii, uint8_t bb) {
    // if not yet held, initialize fading:
    if (!(section[ii]._btn[bb]->beingHeld))
        section[ii]._btn[bb]->beingHeld = true;
    
    switch (section[ii]._btn[bb]->pressedCount) {
        case (3):
            if ((section[ii].mode == 1) || (section[ii].mode == 2)) {
                // slow down colorProgress (increase delay)
                if (colorProgressDelayCounter == COLOR_PROGRESS_DELAY_COUNTER_INIT) {   
                    colorProgressDelayCounter = 0;

                    if (section[ii].colorProgressInterval < 20000) 
                        section[ii].colorProgressInterval++;
                    
                } else 
                    colorProgressDelayCounter++;
                
            }
        break;

        case (2):
        break;

        case (1):
            float f = FADE_FACTOR;
            if (currentTime >= (section[ii]._btn[bb]->pressedTime + BUTTON_FADE_DELAY_RAPID)) 
                f = FADE_FACTOR_RAPID; // after QUICK_DELAY time, adjust double fast

            masterFadeDecrement(ii, f);       // regular decrement
        break;
    }
}





//implement feedback blink? On certain actions, call a function to blink a color on a certain lightsection really quickly, to let the user know something happened. For example: when turning the porch light on, blip the underloft light green really quickly.
//                      when turning the porch light off, blip the underloft light red really quickly. (Or different colors, but similar idea.)

//This might mean that the state must be "saved" and returned to, for the section that was used for the "blink action".



//***********************************************************************************

//                  BOTTOM PRESS ACTIONS
//These happen after BUTTON_RELEASE_TIMER time has passed after a button has been released.
//***********************************************************************************

// TRIPLE PRESS BOTTOM from on: 

//first triple press:
// if porch is off, turn all lights off. Otherwise, turn off all lights EXCEPT porch.

//Second triple press:
// if only PORCH is on, turn PORCH off.

//Third triple press:
// if no light is on, disco mode.

void botAction3presses(uint8_t n, uint8_t m) {
    if (DEBUG) {
        Serial.println(F(" BOT 3 "));
    }

    if (!(section[2].isOn)) {  //if porch is off
        //if other lights are on, turn all lights off.
        //else, if no other lights are on, then all lights are off so we go to disco mode!!!

        //so first, check if other lights are on
        bool somethingIsOn = false;
        for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) 
            if (section[k].isOn) 
                somethingIsOn = true;
            
        // if so, turn off all lights.
        if (somethingIsOn) {
            for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) 
                if (section[k].isOn)
                    turnOffSection(k);
        
        } else  // if no lights are on, go disco!
            for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) {
                // for each light, switch the mode to 2 (colorProgress sudden)

                section[k].mode = 2;
                switchMode(k);
                updateLights(k);
            }
        
    } else  // if porch is on, turn off all OTHER lights
        for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) // turn off all
            if (section[k].isOn && (k != 2)) // if it's not the porch light
                turnOffSection(k);
            
}


// DOUBLE PRESS BOT: from on: reduce mode by 1
// DOUBLE PRESS BOT: from off: do nothing
void botAction2presses(uint8_t ii) {
    if (DEBUG) {
        Serial.println(F(" BOT 2 "));
    }

    if (section[ii].isOn) { //double press from off does not switch to color mode 2 (Sudden)

        section[ii].mode--;

        if (section[ii].mode < 0)
            section[ii].mode = NUM_OF_MODES_CYCLE - 1;

        switchMode(ii);
        updateLights(ii);
    }
}


// PRESS BOT from off: turn on nightlight mode (red and/or ww)
// PRESS BOT from on: turn off all lights, reset mode;
void botAction1press(uint8_t n) {
    if (!(section[n].isOn)) { // turn on night light mode
        if (DEBUG) {
            Serial.println(F(" BOT 1: Nightlight "));
        }
        
        //section[n].RGBW[3] = 1;     // turn on white

        section[n].isOn = true;
        section[n].mode = 4;        // night light
        section[n].RGBW[0] = 1;     // turn on red
        section[n].masterBrightness = 0.1;  // set brightness to 10%

        updateLights(n);

    } else { // turn off light
        if (DEBUG) {
            Serial.println(F(" BOT 1: Turn Off "));
        }
        turnOffSection(n);
    }
}