void heldTopButtonActions(uint8_t n, uint32_t cTime) {  //fade up
    uint8_t b = 1;
    if (DEBUG == true) // {{ DEBUG }}
    {
        Serial.print(F("Fade Up: "));
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

        //HERE is after fade_delay
        if (section[n].mode == (1 || 2) && section[n]._button[b]->pressedCount > 1) { 
            // 2+ presses, effects colormodes 1 and 2 (rgb smooth and rgb sudden)

            // adjust speed of colorProgress
            // speed up (decrease delay)
            if (colorProgressDelayCounter == COLOR_PROGRESS_DELAY_COUNTER_INIT) {   
                //adjust speed
                
                if (section[n].colorProgressInterval > 1) {
                    section[n].colorProgressInterval--;
                }
                colorProgressDelayCounter = 0;
            } else {
                colorProgressDelayCounter++;
            }

        }
        else
        {       //1 press or not colormode 1||2 (rgb smooth / rgb sudden)
            if (section[n]._button[b]->beingHeld == false) // if not yet held, initialize fading:
            {
                section[n]._button[b]->beingHeld = true;
                section[n].isOn = true;
                if (section[n].mode == 0)
                {
                    section[n].RGBW[3] = 1;
                }
                
            }

            float f = FADE_FACTOR;
            if (cTime >= section[n]._button[b]->pressedTime + BUTTON_FADE_DELAY_RAPID)    //after QUICK_DELAY time, increment an additional time per loop (double speed)
            {
                f = FADE_FACTOR_RAPID;
            }
            masterFadeIncrement(n, f);       //regular increment
            updateLights(n);
        }

    } //end held (top button)
}











//***********************************************************************************

//                  TOP PRESS ACTIONS

//***********************************************************************************


void topAction3p(uint8_t n, uint32_t cTime, uint8_t m) {
    // if not held past the fade point, we want max brightness. Otherwise, all we want is fade speed adjust, which happens way below.
    if (cTime < section[n]._button[m]->pressedTime + BUTTON_FADE_DELAY) {

// NEED ANOTHER WAY TO CHECK THIS ABOVE, ALWAYS FALSE so NEVER LETS MAX BRIGHTNESS MODE ENGAGE

        //  TRIPLE PRESS TOP: MAX BRIGHTNESS
        if (DEBUG == true)
        {
            Serial.println(F(" TOP 3 "));
            Serial.println(F("Max Brightness {mode:3}"));
        }

        section[n].mode = 3; // set to mode 3

        for (uint8_t k = 0; k < 4; k++)
        {
            section[n].RGBW[k] = 1;
        }

        section[n].isOn = true;
        section[n].masterBrightness = 1;
    }
}


void topAction2p(uint8_t n, uint32_t cTime) {
    // DOUBLE PRESS TOP: turn on if off; and switch to next mode.
    // [white mode, color mode (cycle or no), white+color mode (white at 50% of color)] + extra hidden index 3:[all lights max, not included in cycle change, only for triple press]

    if (DEBUG == true)
    {
        Serial.println(F(" TOP 2 "));
    }
    
    if (section[n].isOn == false)
    {
        // change to next and turn on
        section[n].isOn = true;
    }
    
    section[n].mode++;  // if was off, we want value of 1 from 0. If it was on, we increment the value
    if (section[n].mode >= NUM_OF_MODES_CYCLE)
    {
        section[n].mode = 0;
    }
    if (DEBUG == true)
    {
        Serial.print(F("Now in mode: "));
        Serial.println(section[n].mode);
    }

    switch (section[n].mode) // turn on the mode:
    {
        case (0): // white mode from RGB sudden
            if (section[n].colorProgress == true)
            { // turn off colorProgress
                section[n].colorProgress = false;
                for (uint8_t k = 0; k < 4; k++)
                    section[n].RGBW[k] = 0;
            }

            section[n].RGBW[3] = 1;
            section[n].masterBrightness = section[n].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
            break;
        case (1): // RGB smooth from white
            // turn off other colors,
            // start colorProgress

            if (section[n].colorProgress == false)
            { //lastMode was not 2
                section[n].colorProgress = true;
                section[n].colorState = random(12); //get a random state to start at

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
                //progressColor(i);
            }

            break;
        case (2): // RGB sudden from RGB smooth
            // start colorProgress

            if (section[n].colorProgress == false)
            {
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
                //progressColor(i);
            }
            section[n].colorProgressTimerStart = cTime;
            section[n].colorProgressInterval = COLOR_PROGRESS_SUDDEN_DELAY_INIT;
            break;
    }
}

void topAction1p(uint8_t n) {
    if (DEBUG == true)
    {
        Serial.println(F(" TOP 1 "));
    }

    // PRESS TOP from off: turn on mode0 to default or to lastBrightness if available
    // PRESS TOP from on: white:  cycle light brightness steps (35, 70, 100)
    //                    RGB:    pause colorProgress
    if (section[n].isOn == false) // from off:
    {
        // TODO: turn on to last brightness if != 0
        // else turn on to default brightness
        section[n].isOn = true;
        section[n].mode = 0;
        section[n].RGBW[3] = 1;
        section[n].masterBrightness = section[n].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
    }
    else // from on:
    {
        switch (section[n].mode)
        {
            case (1): // RGB
            case (2): // RGBW
                // for RGB modes, 1 top press pauses or resumes the colorProgress
                section[n].colorProgress = !section[n].colorProgress;
                // if (section[n].colorProgress == true)
                // {
                //     section[n].colorProgress = false;
                // }
                // else
                // {
                //     section[n].colorProgress = true;
                // }
                break;

            default: // for other modes, cycle up brightness for colors that are on

                section[n].masterBrightness += 0.2;
                if (section[n].masterBrightness == 1)
                {
                    section[n].masterBrightness -= 1;
                }
                else if (section[n].masterBrightness > 1)
                {
                    section[n].masterBrightness = 1;
                }
        }
    }
}