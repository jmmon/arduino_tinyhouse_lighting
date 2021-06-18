// fade up
void heldTopButtonActions(uint8_t n, uint32_t cTime) {
    uint8_t b = 1;
    if (DEBUG == true)
    {
        Serial.print(F("Fade Up: "));
        Serial.print(section[n]._button[b]->pressedCount);
        Serial.println(F(" presses"));
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
                if (section[n].mode == (1 || 2))
                {
                    // speed up colorProgress (decrease delay)
                    if (colorProgressDelayCounter == COLOR_PROGRESS_DELAY_COUNTER_INIT) {   
                        if (section[n].colorProgressInterval > 1) 
                        {
                            section[n].colorProgressInterval--;
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
                    section[n].isOn = true;
                    if (section[n].mode == 0)
                    {
                        section[n].RGBW[3] = 1;
                    }
                }

                //after QUICK_DELAY time, adjust double fast
                float f = FADE_FACTOR;
                if (cTime >= section[n]._button[b]->pressedTime + BUTTON_FADE_DELAY_RAPID)    
                {
                    f = FADE_FACTOR_RAPID;
                }
                masterFadeIncrement(n, f);       //increment
                updateLights(n);            
                break;
        }
    }
}











//***********************************************************************************

//                  TOP PRESS ACTIONS

//***********************************************************************************

//  TRIPLE PRESS TOP: MAX BRIGHTNESS
void topAction3p(uint8_t n, uint32_t cTime, uint8_t m) {
    // NEED ANOTHER WAY TO CHECK THIS ABOVE, ALWAYS FALSE so NEVER LETS MAX BRIGHTNESS MODE ENGAGE
    // maybe if is on, or something
    if (cTime < section[n]._button[m]->pressedTime + BUTTON_FADE_DELAY) 
    {
        if (DEBUG == true)
        {
            Serial.println(F(" TOP 3 "));
            Serial.println(F("Max Brightness {mode:3}"));
        }

        section[n].isOn = true;
        section[n].masterBrightness = 1;
        section[n].mode = 3; //max brightness mode

        for (uint8_t k = 0; k < 4; k++)
        {
            section[n].RGBW[k] = 1;
        }
    }
}


// DOUBLE PRESS TOP: turn on if off; and switch to next mode.
void topAction2p(uint8_t n, uint32_t cTime) {
    section[n].isOn = true;
    section[n].mode++;
    if (section[n].mode >= NUM_OF_MODES_CYCLE)
    {
        section[n].mode = 0;
    }

    if (DEBUG == true)
    {
        Serial.println(F(" TOP 2 "));
        Serial.print(F("Now in mode: "));
        Serial.println(section[n].mode);
    }
    switchMode(n, cTime);
    // switch (section[n].mode) // turn on the mode:
    // {
    //     case (0): // to: white,  from: RGB sudden
    //         section[n].colorProgress = false;
    //         for (uint8_t k = 0; k < 4; k++)
    //         {
    //             section[n].RGBW[k] = 0;
    //         }
                
    //         section[n].RGBW[3] = 1;
    //         section[n].masterBrightness = section[n].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;

    //         break;
    //     case (1): // to: RGB smooth,  from: white
    //     case (2): // to: RGB sudden,  from: RGB smooth
    //         if (section[n].colorProgress == false)
    //         {
    //             section[n].colorProgress = true;
    //             section[n].colorState = random(12);

    //             section[n].RGBW[0] = RED_LIST[section[n].colorState];
    //             section[n].RGBW[1] = GREEN_LIST[section[n].colorState];
    //             section[n].RGBW[2] = BLUE_LIST[section[n].colorState];
    //             section[n].RGBW[3] = 0;
    //             updateLights(n);

    //             section[n].masterBrightness = section[n].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
    //             section[n].colorProgressTimerStart = cTime;
    //             section[n].colorProgressInterval = COLOR_PROGRESS_SMOOTH_DELAY_INIT;
    //         }

    //         break;
    // }
}


// PRESS TOP from   off :   turn on mode0 to default (or to lastBrightness if available)
// PRESS TOP from white :   bump up brightness
// PRESS TOP from   RGB :   pause colorProgress
void topAction1p(uint8_t n) {
    if (DEBUG == true)
    {
        Serial.println(F(" TOP 1 "));
    }

    if (section[n].isOn == false) // from off:
    {
        // TODO: use last brightness if available, else default brightness
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
                // toggle colorProgress
                section[n].colorProgress = !section[n].colorProgress;
                break;

            default: // for other modes, cycle up brightness for colors that are on
                section[n].masterBrightness += 0.2;
                if (section[n].masterBrightness > 1)
                {
                    section[n].masterBrightness = 1;
                }
                break;
        }
    }
}