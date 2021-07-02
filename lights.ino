void turnOffSection(uint8_t ii) {
    section[ii].isOn = false; // if "on," set to "off"
    section[ii].mode = 0;
    section[ii].colorProgress = false;

    for (uint8_t z = 0; z < 4; z++) {
        section[ii].RGBW[z] = 0; // and set values to 0 for each color for that light
        section[ii].RGBWon[z] = false;
    }
    //save masterLevel so that next time it can be used as the starting point?
    section[ii].lastMasterLevel = section[ii].masterLevel;
    section[ii].masterLevel = 0;
    updateLights(ii);
}


void switchMode(uint8_t nn) {
    if (DEBUG) {
        Serial.print(F("Now in mode: "));
        Serial.println(section[nn].mode);
    }

    section[nn].isOn = true;

    switch (section[nn].mode) {
        case (0): // to: white,  from: RGB smooth
            section[nn].colorProgress = false;
            for (uint8_t z = 0; z < 4; z++) {
                section[nn].RGBW[z] = 0;
                section[nn].RGBWon[z] = false;
            }
            
            section[nn].RGBW[3] = 1;
                section[nn].RGBWon[3] = true;

            if (section[nn].lastMasterLevel >= 0.01)
                section[nn].masterLevel = section[nn].lastMasterLevel;
            else 
                section[nn].masterLevel = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
            
        break;

        case (1): // to: RGB smooth,  from: RGB sudden
        case (2): // to: RGB sudden,  from: white
            if (!(section[nn].colorProgress)) {
                for (uint8_t z = 0; z < 3; z++) {
                    section[nn].RGBWon[z] = true;
                }
                section[nn].colorProgress = true;
                section[nn].colorStartTime = currentTime;
                section[nn].masterLevel = section[nn].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                section[nn].RGBW[3] = 0;

                section[nn].colorState = random(MAX_COLOR_STATE);
                section[nn].RGBW[0] = RED_LIST[section[nn].colorState];
                section[nn].RGBW[1] = GREEN_LIST[section[nn].colorState];
                section[nn].RGBW[2] = BLUE_LIST[section[nn].colorState];

                if (section[nn].mode == 1) {
                    section[nn].colorDelayInt = COLOR_LOOP_SMOOTH_DELAY_INT;

                } else {
                    section[nn].colorDelayInt = COLOR_LOOP_SUDDEN_DELAY_INT;
                }
            }
        break;
        
        case (3):   //max brightness
            for (uint8_t z = 0; z < 4; z++) {
                section[nn].RGBW[z] = 1;
                section[nn].RGBWon[z] = true;
            }
                
            section[nn].masterLevel = 1;
        break;

        case (4):
        case (5):
        case (6):
            section[nn].singleColorMode = true;
            for (uint8_t z = 0; z < 4; z++) { //clear rgb
                // section[nn].RGBW[z] = 0;
                section[nn].RGBWon[z] = false;

            }
                
            section[nn].RGBW[section[nn].mode-4] = 1;     // turn on color
            section[nn].RGBWon[section[nn].mode-4] = true;
            section[nn].masterLevel = 0.1;  // set brightness to 10%
        break;

        // case (7):
        //     for (uint8_t z = 0; z < 3; z++) {
        //         section[nn].RGBWon[z] = true;
        //     }
        //     section[nn].masterLevel

        // break;
    }

    if ((section[nn].mode != 1) && (section[nn].mode != 2)) {
        updateLights(nn);
    }
    //updateLights(nn);
    
}

void updateLights(uint8_t i) { // updates a specific light section
    if (DEBUG) {
        updateLightsDEBUG(i);

    } else {
        uint8_t brightnessValue; // index for brightness lookup table

        for (uint8_t z = 0; z < 4; z++) {
            uint8_t height = (uint16_t(section[i].RGBW[z] * section[i].masterLevel * TABLE_SIZE) / HEIGHT);
            uint8_t width = (uint16_t(section[i].RGBW[z] * section[i].masterLevel * TABLE_SIZE) % WIDTH);

            // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
            memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

            if ((section[i].RGBW[z] > 0) && (section[i].masterLevel > 0) && (brightnessValue == 0)) 
                brightnessValue = 1;
                
            DmxSimple.write(section[i].DMX_OUT * 8 - 8 + (z * 2 + 1), brightnessValue);
            DmxSimple.write(section[i].DMX_OUT * 8 - 8 + (z * 2 + 1), brightnessValue);
            section[i].lastRGBW[z] = section[i].RGBW[z];
        }
    }
    
    for (uint8_t z = 0; z < 4; z++) { //clear rgb
        if (section[i].RGBW[z] <= 0) {
            section[i].RGBW[z] = 0;
            section[i].RGBWon[z] = false;
        }
    }
    
    if ( !(section[i].RGBW[0]) && !(section[i].RGBW[1]) && !(section[i].RGBW[2]) && !(section[i].RGBW[3]) ) {
        section[i].isOn = false;
        section[i].masterLevel = 0;

        if (DEBUG) {
            updateLightsOffDEBUG(i);
        }
    }
}




void masterFadeIncrement(uint8_t i, float f) {
    if (section[i].masterLevel < (1 - section[i].BRIGHTNESS_FACTOR * f))
        section[i].masterLevel += section[i].BRIGHTNESS_FACTOR * f;

    else
        section[i].masterLevel = 1; // max
    
    updateLights(i);
}

void masterFadeDecrement(uint8_t i, float f) {
    if (section[i].masterLevel > (section[i].BRIGHTNESS_FACTOR * f))
        section[i].masterLevel -= section[i].BRIGHTNESS_FACTOR * f;

    else
        section[i].masterLevel = 0; // min
    
    updateLights(i);
}



void fadeIncrement(uint8_t i, float f, uint8_t z) {
    if (section[i].RGBW[z] < (1 - section[i].BRIGHTNESS_FACTOR * f))
        section[i].RGBW[z] += section[i].BRIGHTNESS_FACTOR * f;

    else
        section[i].RGBW[z] = 1; // max
    
    updateLights(i);
}

void fadeDecrement(uint8_t i, float f, uint8_t z) {
    if (section[i].RGBW[z] > (section[i].BRIGHTNESS_FACTOR * f))
        section[i].RGBW[z] -= section[i].BRIGHTNESS_FACTOR * f;

    else
        section[i].RGBW[z] = 0; // min
    
    updateLights(i);
}



void progressColorSudden(uint8_t ii) {
    if (section[ii].colorDelayCtr < 1)
        section[ii].colorDelayCtr += (2 * COLOR_LOOP_FADE_AMOUNT);

    else {
        section[ii].colorDelayCtr = 0;

        section[ii].colorState += 1;
        if (section[ii].colorState == MAX_COLOR_STATE) 
            section[ii].colorState = 0;

        //section[ii].colorState = random(MAX_COLOR_STATE);

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
        if (section[ii].colorState == MAX_COLOR_STATE) 
            section[ii].colorState = 0;
        
        if (DEBUG) {
            Serial.print(F("color progress state: ")); 
            Serial.println(section[ii].colorState);
        }

    } else { // else change colors to get closer to current state
        for (uint8_t color = 0; color < 3; color++) {

            if (section[ii].RGBW[color] < section[ii].nextRGB[color]) {

                section[ii].RGBW[color] += COLOR_LOOP_FADE_AMOUNT;

                if (section[ii].RGBW[color] >= section[ii].nextRGB[color])
                    section[ii].RGBW[color] = section[ii].nextRGB[color];
                
            }
            else if (section[ii].RGBW[color] > section[ii].nextRGB[color]) {

                section[ii].RGBW[color] -= COLOR_LOOP_FADE_AMOUNT;

                if (section[ii].RGBW[color] <= section[ii].nextRGB[color]) 
                    section[ii].RGBW[color] = section[ii].nextRGB[color];
            }
        }
    }
    updateLights(ii);
}