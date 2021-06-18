/*  Break up each different action into different functions? ideas:

fade increase, (done)
fade decrease, (done)

or even top button one press, (done)
top button two press,  (done)
top button three press,  (done)
etc,  (done)

Switch mode (done)
top hold, (done)
bottom hold (done)



progressSmooth, 
progressSudden, 

could move all the debug sections to thier own function on its own page?, 

*/

void switchMode(uint8_t nn, uint32_t ccTime) {
    switch (section[nn].mode) // turn on the mode:
    {
        case (0): // to: white,  from: RGB smooth
            section[nn].colorProgress = false;
            for (uint8_t k = 0; k < 4; k++)
            {
                section[nn].RGBW[k] = 0;
            }

            section[nn].RGBW[3] = 1;
            section[nn].masterBrightness = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;

            break;
        case (1): // to: RGB smooth,  from: RGB sudden
        case (2): // to: RGB sudden,  from: white
            if (section[nn].colorProgress == false)
            {
                section[nn].colorProgress = true;
                section[nn].colorState = random(12);

                section[nn].RGBW[0] = RED_LIST[section[nn].colorState];
                section[nn].RGBW[1] = GREEN_LIST[section[nn].colorState];
                section[nn].RGBW[2] = BLUE_LIST[section[nn].colorState];
                section[nn].RGBW[3] = 0;
                updateLights(nn);

                section[nn].masterBrightness = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                section[nn].colorProgressTimerStart = ccTime;
                section[nn].colorProgressInterval = COLOR_PROGRESS_SMOOTH_DELAY_INIT;
            }
            break;
    }
}

void updateLights(uint8_t i)
{                            //updates a specific light section
    if (DEBUG == true)
    {
        updateLightsDEBUG(i);
    }
    else
    {
        uint8_t brightnessValue; //index for brightness lookup table
        for (uint8_t k = 0; k < 4; k++)
        {
            uint8_t height = (uint16_t(section[i].RGBW[k] * section[i].masterBrightness * TABLE_SIZE) / HEIGHT);
            uint8_t width = (uint16_t(section[i].RGBW[k] * section[i].masterBrightness * TABLE_SIZE) % WIDTH);
            // looks up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
            memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

            if (section[i].RGBW[k] > 0 && section[i].masterBrightness > 0 && brightnessValue == 0)
            {
                brightnessValue = 1;
            }
                
            DmxSimple.write(section[i].DMXout * 8 - 8 + (k * 2 + 1), brightnessValue); //main underloft lights
            DmxSimple.write(section[i].DMXout * 8 - 8 + (k * 2 + 1), brightnessValue); //main underloft lights
            section[i].lastRGBW[k] = section[i].RGBW[k];
        }
    }
    
    if (section[i].RGBW[3] <= 0 && section[i].RGBW[0] <= 0 && section[i].RGBW[1] <= 0 && section[i].RGBW[2] <= 0)
    {
        section[i].isOn = false;
        section[i].masterBrightness = 0;
        if (DEBUG == true)
        {
            Serial.print(F("MasterBrightness: ")); Serial.println(section[i].masterBrightness);
            Serial.print(F("IsOn:")); Serial.println(section[i].isOn);
        }
    }
}

void masterFadeIncrement(uint8_t i, float f)
{
    if (section[i].masterBrightness < (1 - section[i].BRIGHTNESS_FACTOR * f))
    {
        section[i].masterBrightness += section[i].BRIGHTNESS_FACTOR * f;
    }
    else
    {
        section[i].masterBrightness = 1; // max
    }
}

void masterFadeDecrement(uint8_t i, float f)
{
    if (section[i].masterBrightness > (section[i].BRIGHTNESS_FACTOR * f))
    {
        section[i].masterBrightness -= section[i].BRIGHTNESS_FACTOR * f;
    }
    else
    {
        section[i].masterBrightness = 0; // min
    }
}

void progressColor(uint8_t i)
{
    if (section[i].mode == 2) 
    {
        //sudden color changes

        section[i].colorState += 1;
        if (section[i].colorState == 12)
        {
            section[i].colorState = 0;
        }

        if (DEBUG == true)
        {
            Serial.print(F("color progress state: ")); Serial.println(section[i].colorState);
        }

        //set new light color
        section[i].RGBW[0] = RED_LIST[section[i].colorState];
        section[i].RGBW[1] = GREEN_LIST[section[i].colorState];
        section[i].RGBW[2] = BLUE_LIST[section[i].colorState];

    } 
    else if (section[i].mode == 1) 
    {
        //smooth color changes

        section[i].nextRGB[0] = RED_LIST[section[i].colorState]; // target levels for the current state
        section[i].nextRGB[1] = GREEN_LIST[section[i].colorState];
        section[i].nextRGB[2] = BLUE_LIST[section[i].colorState];

        if ((section[i].nextRGB[0] == section[i].RGBW[0]) && (section[i].nextRGB[1] == section[i].RGBW[1]) && (section[i].nextRGB[2] == section[i].RGBW[2])) // Go to next state
        {
            section[i].colorState += 1;
            if (section[i].colorState == 12)
            {
                section[i].colorState = 0;
            }
                
            if (DEBUG == true)
            {
                Serial.print(F("color progress state: ")); Serial.println(section[i].colorState);
            }
        }
        else // else change colors to get closer to current state
        {
            for (uint8_t k = 0; k < 3; k++) //rgb
            {
                if (section[i].RGBW[k] < section[i].nextRGB[k])
                {
                    section[i].RGBW[k] += COLOR_PROGRESS_FADE_AMOUNT;
                    if (section[i].RGBW[k] >= section[i].nextRGB[k])
                    {
                        section[i].RGBW[k] = section[i].nextRGB[k];
                    }
                        
                }
                else if (section[i].RGBW[k] > section[i].nextRGB[k])
                {
                    section[i].RGBW[k] -= COLOR_PROGRESS_FADE_AMOUNT;
                    if (section[i].RGBW[k] <= section[i].nextRGB[k])
                    {
                        section[i].RGBW[k] = section[i].nextRGB[k];
                    }
                        
                }
            }
        }
    }
    updateLights(i);
}
