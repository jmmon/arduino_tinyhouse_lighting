void btnTopHeld(uint8_t ii, uint8_t bb, uint32_t cTime) {
    switch (section[ii]._button[bb]->pressedCount)
    {
        case (3):
            if (section[ii].mode == 1 || section[ii].mode == 2)
            {
                // speed up colorProgress (decrease delay)
                if (colorProgressDelayCounter == COLOR_PROGRESS_DELAY_COUNTER_INIT) {   
                    if (section[ii].colorProgressInterval > 1) 
                    {
                        section[ii].colorProgressInterval--;
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
            if (section[ii]._button[bb]->beingHeld == false) 
            {
                section[ii]._button[bb]->beingHeld = true;
                section[ii].isOn = true;
                if (section[ii].mode == 0)
                {
                    section[ii].RGBW[3] = 1;
                }
            }

            //after QUICK_DELAY time, adjust double fast
            float f = FADE_FACTOR;
            if (cTime >= section[ii]._button[bb]->pressedTime + BUTTON_FADE_DELAY_RAPID)    
            {
                f = FADE_FACTOR_RAPID;
            }
            masterFadeIncrement(ii, f);       //increment
            break;
    }
}

//??? should make function "applyMode3", "applyMode2", etc???


//***********************************************************************************

//                  TOP PRESS ACTIONS
//These happen after BUTTON_RELEASE_TIMER time has passed after a button has been released.
//***********************************************************************************

//  TRIPLE PRESS TOP: MAX BRIGHTNESS
//from off: turn on max brightness
//from on (any mode): turn on max brightness
void topAction3p(uint8_t ii, uint32_t cTime, uint8_t bb) {
    if (DEBUG == true)
    {
        Serial.println(F(" TOP 3 ")); Serial.println(F("Max Brightness {mode:3}"));
    }

    section[ii].isOn = true;
    section[ii].masterBrightness = 1;
    section[ii].mode = 3; //max brightness mode

    for (uint8_t k = 0; k < 4; k++)
    {
        section[ii].RGBW[k] = 1;
    }

    updateLights(i);
}


// DOUBLE PRESS TOP: turn on if off; and switch to next mode.
void topAction2p(uint8_t ii, uint32_t cTime) {
    section[ii].isOn = true;
    section[ii].mode++;
    if (section[ii].mode >= NUM_OF_MODES_CYCLE)
    {
        section[ii].mode = 0;
    }

    if (DEBUG == true)
    {
        Serial.println(F(" TOP 2 "));
        Serial.print(F("Now in mode: "));
        Serial.println(section[ii].mode);
    }
    switchMode(ii, cTime);

    updateLights(i);
}


// PRESS TOP from   off :   turn on mode0 to default (or to lastBrightness if available)
// PRESS TOP from white :   bump up brightness
// PRESS TOP from   RGB :   pause colorProgress
void topAction1p(uint8_t ii) {
    if (DEBUG == true)
    {
        Serial.println(F(" TOP 1 "));
    }

    if (section[ii].isOn == false) // from off:
    {
        // TODO: use last brightness if available, else default brightness
        section[ii].isOn = true;
        section[ii].mode = 0;
        section[ii].RGBW[3] = 1;
        section[ii].masterBrightness = section[ii].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
    }
    else // from on:
    {
        switch (section[ii].mode)
        {
            case (1): // RGB
            case (2): // RGBW
                // toggle colorProgress
                section[ii].colorProgress = !section[ii].colorProgress;
                break;

            default: // for other modes, cycle up brightness for colors that are on
                section[ii].masterBrightness += 0.2;
                if (section[ii].masterBrightness > 1)
                {
                    section[ii].masterBrightness = 1;
                }
                break;
        }
    }
    
    updateLights(i);
}