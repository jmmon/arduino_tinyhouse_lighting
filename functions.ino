void btnRegisterNewPress(uint8_t ii, uint8_t bb) {
    section[ii]._btn[bb]->pressedCount++;      // add one press to its counter
    section[ii]._btn[bb]->pressedTime = currentTime; // save the time to detect multipresses
}

void btnRegisterNewRelease(uint8_t ii, uint8_t bb) {
    section[ii]._btn[bb]->pressedTime = 0; // so the next press is new rather than a held press
    section[ii]._btn[bb]->releaseTime = currentTime; // set the releaseTime
}

void turnOffSection(uint8_t index) {
    section[index].isOn = false; // if "on," set to "off"
    section[index].mode = 0;
    section[index].colorProgress = false;

    for (uint8_t z = 0; z < 4; z++) 
        section[index].RGBW[z] = 0; // and set values to 0 for each color for that light
    
    section[index].masterBrightness = 0;
    updateLights(index);
}



void switchMode(uint8_t nn) {
    if (DEBUG) {
        Serial.print(F("Now in mode: "));
        Serial.println(section[nn].mode);
    }

    switch (section[nn].mode) {
        case (0): // to: white,  from: RGB smooth
            section[nn].colorProgress = false;

            for (uint8_t k = 0; k < 4; k++) {
                section[nn].RGBW[k] = 0;
            }

            section[nn].RGBW[3] = 1;
            section[nn].masterBrightness = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
        break;

        case (1): // to: RGB smooth,  from: RGB sudden
        case (2): // to: RGB sudden,  from: white
            if (!(section[nn].colorProgress)) {
                section[nn].colorProgress = true;
                section[nn].colorState = random(12);

                section[nn].RGBW[0] = RED_LIST[section[nn].colorState];
                section[nn].RGBW[1] = GREEN_LIST[section[nn].colorState];
                section[nn].RGBW[2] = BLUE_LIST[section[nn].colorState];
                section[nn].RGBW[3] = 0;

                section[nn].masterBrightness = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                section[nn].colorProgressTimerStart = currentTime;
                section[nn].colorProgressInterval = COLOR_PROGRESS_SMOOTH_DELAY_INIT;
            }
        break;
    }
}


void updateLights(uint8_t i) { //updates a specific light section
    if (DEBUG) {
        updateLightsDEBUG(i);
    } else {
        uint8_t brightnessValue; //index for brightness lookup table

        for (uint8_t k = 0; k < 4; k++) {
            uint8_t height = (uint16_t(section[i].RGBW[k] * section[i].masterBrightness * TABLE_SIZE) / HEIGHT);
            uint8_t width = (uint16_t(section[i].RGBW[k] * section[i].masterBrightness * TABLE_SIZE) % WIDTH);

            // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
            memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

            if ((section[i].RGBW[k] > 0) && (section[i].masterBrightness > 0) && (brightnessValue == 0)) 
                brightnessValue = 1;
                
            DmxSimple.write(section[i].DMX_OUT * 8 - 8 + (k * 2 + 1), brightnessValue);
            DmxSimple.write(section[i].DMX_OUT * 8 - 8 + (k * 2 + 1), brightnessValue);
            section[i].lastRGBW[k] = section[i].RGBW[k];
        }
    }
    
    if ( (section[i].RGBW[0] <= 0) && (section[i].RGBW[1] <= 0) && (section[i].RGBW[2] <= 0) && (section[i].RGBW[3] <= 0) ) {
        section[i].isOn = false;
        section[i].masterBrightness = 0;

        if (DEBUG) {
            updateLightsOffDEBUG(i);
        }
    }
}


void masterFadeIncrement(uint8_t i, float f) {
    if (section[i].masterBrightness < (1 - section[i].BRIGHTNESS_FACTOR * f))
        section[i].masterBrightness += section[i].BRIGHTNESS_FACTOR * f;

    else
        section[i].masterBrightness = 1; // max
    
    updateLights(i);
}

void masterFadeDecrement(uint8_t i, float f) {
    if (section[i].masterBrightness > (section[i].BRIGHTNESS_FACTOR * f))
        section[i].masterBrightness -= section[i].BRIGHTNESS_FACTOR * f;

    else
        section[i].masterBrightness = 0; // min
    
    updateLights(i);
}



void progressColorSudden(uint8_t ii) {
    if (section[ii].colorDelayCounter < 1)
        section[ii].colorDelayCounter += (2 * COLOR_PROGRESS_FADE_AMOUNT);

    else {
        section[ii].colorDelayCounter = 0;
        section[ii].colorState = random(12);

        //set new light color
        section[ii].RGBW[0] = RED_LIST[section[ii].colorState];
        section[ii].RGBW[1] = GREEN_LIST[section[ii].colorState];
        section[ii].RGBW[2] = BLUE_LIST[section[ii].colorState];

        updateLights(ii);

        if (DEBUG) {
            Serial.print(F("color progress state: ")); 
            Serial.println(section[ii].colorState);
        }
    }
}


void progressColorSmooth(uint8_t ii)
{
    section[ii].nextRGB[0] = RED_LIST[section[ii].colorState]; // target levels for the current state
    section[ii].nextRGB[1] = GREEN_LIST[section[ii].colorState];
    section[ii].nextRGB[2] = BLUE_LIST[section[ii].colorState];

    if ((section[ii].RGBW[0] == section[ii].nextRGB[0]) && (section[ii].RGBW[1] == section[ii].nextRGB[1]) && (section[ii].RGBW[2] == section[ii].nextRGB[2])) {

        section[ii].colorState += 1;

        if (section[ii].colorState == 12) 
            section[ii].colorState = 0;
        
        if (DEBUG) {
            Serial.print(F("color progress state: ")); 
            Serial.println(section[ii].colorState);
        }

    } else { // else change colors to get closer to current state

        for (uint8_t color = 0; color < 3; color++) {

            if (section[ii].RGBW[color] < section[ii].nextRGB[color]) {

                section[ii].RGBW[color] += COLOR_PROGRESS_FADE_AMOUNT;

                if (section[ii].RGBW[color] >= section[ii].nextRGB[color])
                    section[ii].RGBW[color] = section[ii].nextRGB[color];
                
            }
            else if (section[ii].RGBW[color] > section[ii].nextRGB[color]) {

                section[ii].RGBW[color] -= COLOR_PROGRESS_FADE_AMOUNT;

                if (section[ii].RGBW[color] <= section[ii].nextRGB[color]) 
                    section[ii].RGBW[color] = section[ii].nextRGB[color];
            }
        }
    }
    updateLights(ii);
}

void btnPressedActions(uint8_t ii, uint8_t bb) {
    section[ii]._btn[bb]->releaseTime = 0;

    if (section[ii]._btn[bb]->pressedCount > 0) {

        if (section[ii]._btn[bb]->beingHeld)
            section[ii]._btn[bb]->beingHeld = false;
        
        else {
            if (section[ii]._btn[bb]->pressedCount > MAX_PRESS_COUNT) 
                section[ii]._btn[bb]->pressedCount = MAX_PRESS_COUNT;

            switch (bb) {
                case (1): // top button actions
                    switch(section[ii]._btn[bb]->pressedCount) {
                        case (3):
                            topAction3presses(ii, bb);
                        break;

                        case (2):
                            topAction2presses(ii);
                        break;

                        case (1):
                            topAction1press(ii);
                        break;
                    }
                break;

                case (0): // bottom button actions
                    switch (section[ii]._btn[bb]->pressedCount) {
                        case (3):
                            botAction3presses(ii, bb);
                        break;

                        case (2):
                            botAction2presses(ii);
                        break;

                        case (1):
                            botAction1press(ii);
                        break;
                    }
                break;
            }
        }
        
        section[ii]._btn[bb]->pressedCount = 0;
    }
}