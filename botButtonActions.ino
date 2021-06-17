void heldBottomButtonActions(uint8_t n, uint32_t cTime) { // fade down
    uint8_t b = 0;

    if (DEBUG == true) // {{ DEBUG }}
    {
        Serial.print(F("Fade Down"));
        Serial.print(section[n]._button[b]->pressedCount);
        Serial.println(F(" presses"));
    }

    //fade
    if (section[n]._button[b]->pressedTime == 0) // if button[n].pressedTime == 0 this is a NEW button press
    {
        section[n]._button[b]->pressedTime = cTime; // save the time to detect multipresses
        section[n]._button[b]->pressedCount++;            // add one press to its counter
    }
    else if (cTime >= section[n]._button[b]->pressedTime + BUTTON_FADE_DELAY) // if button has been pressed and held past BUTTON_FADE_DELAY, button is being held:
    {


        //HERE is after fade_delay, commence fading
        // 3+ presses, effects colormodes 1 and 2 (rgb smooth and rgb sudden)
        if (section[n].mode == (1 || 2) && section[n]._button[b]->pressedCount > 2) 
        { 
            // slow down colorProgress (increase delay)
            if (colorProgressDelayCounter == COLOR_PROGRESS_DELAY_COUNTER_INIT) {   
                //adjust speed
                if (section[n].colorProgressInterval < 20000) {
                    section[n].colorProgressInterval++;
                }
                colorProgressDelayCounter = 0;
            } else {
                colorProgressDelayCounter++;
            }

        } else {       //1 press or not colormode 1||2

            if (section[n]._button[b]->beingHeld == false) // if not yet held, initialize fading:
            {
                section[n]._button[b]->beingHeld = true;
            }

            //after QUICK_DELAY time, decrement an additional time per loop (double speed)
            float f = FADE_FACTOR;
            if (cTime >= section[n]._button[b]->pressedTime + BUTTON_FADE_DELAY_RAPID)   
            {
                f = FADE_FACTOR_RAPID;
            }

            masterFadeDecrement(n, f);       //regular decrement
            updateLights(n);
        }
    }
}





//***********************************************************************************

//                  BOTTOM PRESS ACTIONS

//***********************************************************************************


void botAction3p(uint8_t n, uint32_t cTime, uint8_t m) {
    //if not held past the fade point, we want all off. Otherwise, all we want is fade speed adjust, which happens way below.
    if (cTime < section[n]._button[m]->pressedTime + BUTTON_FADE_DELAY) {
        // TRIPLE PRESS BOTTOM from on: if lights are on, turn off all EXCEPT PORCH. (If porch isn't on, turn all lights off.)
        //                              if only PORCH is on, turn PORCH off.
        // if no light is on, disco mode.
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

void botAction2p(uint8_t n, uint32_t cTime) {
    // DOUBLE PRESS BOT: from on: reduce mode by 1
    // DOUBLE PRESS BOT: from off: do nothing
    if (DEBUG == true)
    {
        Serial.println(F(" BOT 2 "));
    }
    if (section[n].isOn == true)
    {
        section[n].mode--;
        if (section[n].mode < 0)
        {
            section[n].mode = NUM_OF_MODES_CYCLE - 1;
        }

        if (DEBUG == true)
        {
            Serial.print(F("Now in mode: "));
            Serial.println(section[n].mode);
        }

        switch (section[n].mode) // turn on the mode:
        {
            case (0): // white from RGB smooth
                section[n].colorProgress = false;
                for (uint8_t k = 0; k < 4; k++)
                {
                    section[n].RGBW[k] = 0;
                }

                section[n].RGBW[3] = 1;
                section[n].masterBrightness = DEFAULT_BRIGHTNESS;

                break;
            case (1): // RGB smooth from RGB sudden
                
                section[n].colorProgress = true;
                section[n].colorState = random(12); // get a random state to start at

                section[n].RGBW[3] = 0;
                // for (uint8_t k = 0; k < 4; k++)
                //         section[n].RGBW[k] = 0;
                    //initialize a state
                section[n].RGBW[0] = RED_LIST[section[n].colorState];
                section[n].RGBW[1] = GREEN_LIST[section[n].colorState];
                section[n].RGBW[2] = BLUE_LIST[section[n].colorState];
                updateLights(n);

                section[n].masterBrightness = section[n].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                section[n].colorProgressTimerStart = cTime;
                section[n].colorProgressInterval = COLOR_PROGRESS_SMOOTH_DELAY_INIT;

                break;
            case (2): // RGB sudden from W
                // start colorProgress

                section[n].colorProgress = true;
                section[n].colorState = random(12); // get a random state to start at

                section[n].RGBW[3] = 0;
                // for (uint8_t k = 0; k < 4; k++)
                //         section[n].RGBW[k] = 0;
                    //initialize a state
                section[n].RGBW[0] = RED_LIST[section[n].colorState];
                section[n].RGBW[1] = GREEN_LIST[section[n].colorState];
                section[n].RGBW[2] = BLUE_LIST[section[n].colorState];
                updateLights(n);

                section[n].masterBrightness = section[n].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                section[n].colorProgressTimerStart = cTime;
                section[n].colorProgressInterval = COLOR_PROGRESS_SUDDEN_DELAY_INIT;

                break;
        }
    }
}

void botAction1p(uint8_t n) {
    // PRESS BOT from off: turn on nightlight mode (red and/or ww)
    // PRESS BOT from on: turn off all lights, reset mode;

    if (section[n].isOn == false)
    {
        if (DEBUG == true)
        {
            Serial.println(F(" BOT 1: Nightlight "));
        }
        // turn on night light mode
        section[n].isOn = true;
        section[n].mode = 4;        // night light
        section[n].RGBW[3] = 1;     // turn on white
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
        section[n].colorProgress = false; // in case it was on, turn of colorProgress
        
        for (uint8_t k = 0; k < 4; k++)
        {
            section[n].RGBW[k] = 0;
        }
        section[n].masterBrightness = 0;
    }
}