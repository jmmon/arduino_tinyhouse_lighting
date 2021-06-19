// fade down
void heldBotBtnActions(uint8_t n, uint32_t cTime) {
    uint8_t b = 0;
    if (DEBUG == true)
    {
        heldBotBtnActionsDEBUG(n, b);
    }

    // if NEW button press
    if (section[n]._button[b]->pressedTime == 0) 
    {
        section[n]._button[b]->pressedTime = cTime; // save the time to detect multipresses
        section[n]._button[b]->pressedCount++;            // add one press to its counter
    }
    else if (cTime >= section[n]._button[b]->pressedTime + BUTTON_FADE_DELAY) // if button has been pressed and held past BUTTON_FADE_DELAY, button is being held:
    {
        switch (section[n]._button[b]->pressedCount)
        {
            case (3):
                if (section[n].mode == 1 || section[n].mode == 2)
                {
                    // slow down colorProgress (increase delay)
                    if (colorProgressDelayCounter == COLOR_PROGRESS_DELAY_COUNTER_INIT) {   
                        if (section[n].colorProgressInterval < 20000)
                        {
                            section[n].colorProgressInterval++;
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
                // if not yet held, initialize fading:
                if (section[n]._button[b]->beingHeld == false)
                {
                    section[n]._button[b]->beingHeld = true;
                }
                
                float f = FADE_FACTOR;
                if (cTime >= section[n]._button[b]->pressedTime + BUTTON_FADE_DELAY_RAPID)   
                {
                    f = FADE_FACTOR_RAPID; // after QUICK_DELAY time, adjust double fast
                }

                masterFadeDecrement(n, f);       // regular decrement
                break;
        }
    }
}









//***********************************************************************************

//                  BOTTOM PRESS ACTIONS

//***********************************************************************************

//todo:
// TRIPLE PRESS BOTTOM from on: if lights are on, turn off all EXCEPT PORCH. (If porch isn't on, turn all lights off.)
// if only PORCH is on, turn PORCH off.
// if no light is on, disco mode.

void botAction3p(uint8_t n, uint32_t cTime, uint8_t m) {
    //if not held past the fade point, we want all off. Otherwise, all we want is fade speed adjust, which happens way below.
    if (cTime < section[n]._button[m]->pressedTime + BUTTON_FADE_DELAY) {
        
        if (DEBUG == true)
        {
            Serial.println(F(" BOT 3 "));
        }

        // if (section[2].isOn == true) {  //if porch is on
        // } else {
        // }

        // check if any lights are on (except porch)
        bool on = false;
        for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) 
        {
            if (section[k].isOn = true && k != 2) on = true;
        }
        if (on == true) // excluding porch, if any are on, turn them off.
        {
            if (DEBUG == true)
            {
                Serial.println(F("Light(s) are on, turn them off (except porch light)"));
            }
            
            for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) // turn off all but porch
            {
                if (section[k].isOn = true && k != 2) // if the light is on and it's not the porch light,
                {
                    section[k].isOn = false; // if "on," set to "off"
                    section[k].colorProgress = false;
                    for (uint8_t z = 0; z < 4; z++)
                        section[k].RGBW[z] = 0; // and set values to 0 for each color for that light
                    section[k].masterBrightness = 0;
                }
            }
        }
        else if (section[2].isOn == true) // else if porch is on:
        {
            if (DEBUG == true)
            {
                Serial.println(F("Only porch is on: turning off."));
            }

            section[2].isOn = false;
            section[2].colorProgress = false;
            for (uint8_t z = 0; z < 4; z++)
                section[2].RGBW[z] = 0; // and set values to 0 for each color for that light
            section[2].masterBrightness = 0;
        }
        else  // else no lights are on so do:
        {
            if (DEBUG == true)
            {
                Serial.println(F("Lights were off: disco mode!"));
            }

            // TODO: all is off, disco mode
        }
    }
}


// DOUBLE PRESS BOT: from on: reduce mode by 1
// DOUBLE PRESS BOT: from off: do nothing
void botAction2p(uint8_t n, uint32_t cTime) {
    if (DEBUG == true)
    {
        botAction2pDEBUG(n);
    }

    if (section[n].isOn == true)
    {
        section[n].mode--;
        if (section[n].mode < 0)
        {
            section[n].mode = NUM_OF_MODES_CYCLE - 1;
        }
        switchMode(n, cTime);
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
    }
    else // turn off all colors
    {
        if (DEBUG == true)
        {
            Serial.println(F(" BOT 1: Turn Off "));
        }
        section[n].mode = 0;
        section[n].isOn = false;
        section[n].colorProgress = false;
        section[n].masterBrightness = 0;
        
        for (uint8_t k = 0; k < 4; k++)
        {
            section[n].RGBW[k] = 0;
        }
    }
}