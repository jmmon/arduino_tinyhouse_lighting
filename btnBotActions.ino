void btnBotHeld(uint8_t ii, uint8_t bb, uint32_t cTime) {
    // if not yet held, initialize fading:
    if (section[ii]._button[bb]->beingHeld == false)
    {
        section[ii]._button[bb]->beingHeld = true;
    }
    switch (section[ii]._button[bb]->pressedCount)
    {
        case (3):
            if (section[ii].mode == 1 || section[ii].mode == 2)
            {
                // slow down colorProgress (increase delay)
                if (colorProgressDelayCounter == COLOR_PROGRESS_DELAY_COUNTER_INIT) {   
                    if (section[ii].colorProgressInterval < 20000)
                    {
                        section[ii].colorProgressInterval++;
                    }
                    colorProgressDelayCounter = 0;
                } else {
                    colorProgressDelayCounter++;
                }
            }
            break;
        case (2):
            break;
        case (1):
            float f = FADE_FACTOR;
            if (cTime >= section[ii]._button[bb]->pressedTime + BUTTON_FADE_DELAY_RAPID)   
            {
                f = FADE_FACTOR_RAPID; // after QUICK_DELAY time, adjust double fast
            }

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

//todo:
// TRIPLE PRESS BOTTOM from on: 

//first triple press:
// if porch is off, turn all lights off. Otherwise, turn off all lights EXCEPT porch.

//Second triple press:
// if only PORCH is on, turn PORCH off.

//Third triple press:
// if no light is on, disco mode.

void botAction3p(uint8_t n, uint32_t cTime, uint8_t m) {
    if (DEBUG == true)
    {
        Serial.println(F(" BOT 3 "));
    }

    if (section[2].isOn == false) {  //if porch is off
        //if other lights are on, turn all lights off.
        //else, if no other lights are on, then all lights are off so we go to disco mode!!!

        //so first, check if other lights are on with a loop and boolean.
        bool somethingIsOn = false;
        for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++)
        {
            if (section[k].isOn == true)
            {
                somethingIsOn = true;
            }
        }

        //if so, turn off all lights.
        if (somethingIsOn == true)
        {
            //turn off ALL lights
            for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++)
            {
                if (section[k].isOn == true) // if the light is on
                {
                    turnOffSection(k);
                }
            }
        }
        else // if no lights are on, go disco!
        {
            //activate disco mode!
            //for each light, switch the mode to 2 (colorProgress sudden)
            for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++)
            {
                section[k].mode = 2;
                switchMode(k, cTime);
                updateLights(k);
            }
        }

    } 
    else  //if porch is on, turn off all OTHER lights
    {
        for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) // turn off all
        {
            if (section[k].isOn == true && k != 2) // if it's not the porch light
            {
                turnOffSection(k);
            }
        }
    }
}


// DOUBLE PRESS BOT: from on: reduce mode by 1
// DOUBLE PRESS BOT: from off: do nothing
void botAction2p(uint8_t ii, uint32_t cTime) {
    if (DEBUG == true)
    {
        Serial.println(F(" BOT 2 "));
    }

    if (section[ii].isOn == true)    //double press from off does not switch to color mode 2 (Sudden)
    {
        section[ii].mode--;
        if (section[ii].mode < 0)
        {
            section[ii].mode = NUM_OF_MODES_CYCLE - 1;
        }

        switchMode(ii, cTime);
    }
}


// PRESS BOT from off: turn on nightlight mode (red and/or ww)
// PRESS BOT from on: turn off all lights, reset mode;
void botAction1p(uint8_t n) {
    if (section[n].isOn == false)
    {
        // turn on night light mode
        if (DEBUG == true)
        {
            Serial.println(F(" BOT 1: Nightlight "));
        }
        
        section[n].isOn = true;
        section[n].mode = 4;        // night light
        //section[n].RGBW[3] = 1;     // turn on white
        section[n].RGBW[0] = 1;     // turn on red
        section[n].masterBrightness = 0.1;  // set brightness to 10%

        updateLights(n);
    }
    else // turn off light
    {
        if (DEBUG == true)
        {
            Serial.println(F(" BOT 1: Turn Off "));
        }
        turnOffSection(n);
    }
}