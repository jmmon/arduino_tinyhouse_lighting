//**********************************************************************

void switchMode(uint8_t nn) {
    if (DEBUG) {
        Serial.print(F("Now in mode: ")); Serial.println(section[nn].mode);
    }

    section[nn].RGBWM[4]->isOn = true;

    switch (section[nn].mode) {
        case (0): // off
            switchToOff(nn);
            break;
        case (LOW_CYCLE_STARTS_AT): // white
            switchToWhite(nn);
            break;
        case (LOW_CYCLE_STARTS_AT + 1): // RGB smoothn
        case (LOW_CYCLE_STARTS_AT + 2): // RGB sudden
            switchToRGB(nn);
            break;
        case (LOW_CYCLE_STARTS_AT + 3):   //max brightness
            switchToMax(nn);
            break;
        case (HIGH_CYCLE_STARTS_AT):        // r
        case (HIGH_CYCLE_STARTS_AT + 1):    // g
        case (HIGH_CYCLE_STARTS_AT + 2):    // b
            switchToSingleColor(nn);
            break;
        case (HIGH_CYCLE_STARTS_AT + 3): // combined
            switchToCombined(nn);
            break;
    }
}

void switchToOff(uint8_t nn) { // 0
    section[nn].RGBWM[4]->isOn = false; // if "on," set to "off"
    section[nn].colorProgress = false;
    section[nn].extendedFade = false;

    for (uint8_t color = 0; color < 4; color++) {
        section[nn].RGBWM[color]->lvl = 0; // and set values to 0 for each color for that light
        section[nn].RGBWM[color]->isOn = false;
    }

    section[nn].RGBWM[4]->lastLvl = section[nn].RGBWM[4]->lvl;
    section[nn].RGBWM[4]->lvl = 0;
    
    updateLights(nn);
}

void switchToWhite(uint8_t nn) { // 1
    //wipe:
    section[nn].colorProgress = false;
    for (uint8_t color = 0; color < 4; color++) {
        
        section[nn].RGBWM[color]->lvl = 0;
        section[nn].RGBWM[color]->isOn = false;
    }
    
    section[nn].RGBWM[3]->lvl = TABLE_SIZE;
    section[nn].RGBWM[3]->isOn = true;
    section[nn].RGBWM[4]->lvl = uint16_t(TABLE_SIZE * (section[nn].LEVEL_FACTOR / 100.0) * (DEFAULT_LEVEL / 100.0));
    
    updateLights(nn);
}



void switchToRGB(uint8_t nn) { // 2, 3
    if (!(section[nn].colorProgress)) {
        //initialize colorProgress
        section[nn].colorProgress = true;
        section[nn].colorStartTime = currentTime;

        // section[nn].RGBWM[3]->lvl = 0;
        section[nn].RGBWM[3]->lvl = 0;
        section[nn].RGBWM[3]->isOn = false;
        for (uint8_t color = 0; color < 3; color++) {
            section[nn].RGBWM[color]->isOn = true;
        }
        section[nn].RGBWM[4]->lvl = uint16_t(TABLE_SIZE * (section[nn].LEVEL_FACTOR / 100.00) * (DEFAULT_LEVEL / 100.00));
    }

    section[nn].colorState = random(MAX_COLOR_STATE);

    section[nn].RGBWM[0]->lvl = uint16_t(TABLE_SIZE * (RED_LIST_NEW[section[nn].colorState]  / 100.00));
    section[nn].RGBWM[1]->lvl = uint16_t(TABLE_SIZE * (GREEN_LIST_NEW[section[nn].colorState]  / 100.00));
    section[nn].RGBWM[2]->lvl = uint16_t(TABLE_SIZE * (BLUE_LIST_NEW[section[nn].colorState]  / 100.00));

    section[nn].colorProgFadeFactor = COLOR_PROG_FADE_FACTOR_DEFAULT;
    // section[nn].colorProgFadeFactor = (section[nn].mode == (LOW_CYCLE_STARTS_AT + 1)) ? 
    //     COLOR_PROG_FADE_FACTOR_DEFAULT : 
    //     (COLOR_PROG_FADE_FACTOR_MAX - COLOR_PROG_FADE_FACTOR_DEFAULT) ;
}



void switchToMax(uint8_t nn) { // 4
    for (uint8_t color = 0; color < 4; color++) {
        section[nn].RGBWM[color]->lvl = TABLE_SIZE;
        section[nn].RGBWM[color]->isOn = true;
    }
        
    section[nn].RGBWM[4]->lvl = TABLE_SIZE;
    
    updateLights(nn);
}


void switchToSingleColor(uint8_t nn) {
    section[nn].RGBWM[4]->isOn = true;
    section[nn].RGBWM[3]->isOn = false; // turn off white
    section[nn].RGBWM[3]->lvl = 0;
    
    for (uint8_t color = 0; color < 3; color++) {
        section[nn].RGBWM[color]->isOn = false;
        
        if (section[nn].RGBWM[color]->lvl == 0) { // give em some value for combined from double bottom press from red
            section[nn].RGBWM[color]->lvl = uint16_t(1.0 * TABLE_SIZE * (section[nn].LEVEL_FACTOR / 100.0) * (DEFAULT_LEVEL / 100.0) * (2./3.));
        }
    }
    section[nn].RGBWM[section[nn].mode - HIGH_CYCLE_STARTS_AT]->isOn = true;
    section[nn].RGBWM[4]->lvl = TABLE_SIZE;
    
    updateLights(nn);
}

void switchToCombined(uint8_t nn) {
    // fade with RGBWM[4]->lvl, extend with white
    for (uint8_t color = 0; color < 3; color++) {
        section[nn].RGBWM[color]->isOn = true;
    }
        // normalize brightness to sometimes expand range
    if ((section[nn].RGBWM[0]->lvl >= section[nn].RGBWM[1]->lvl) && (section[nn].RGBWM[0]->lvl >= section[nn].RGBWM[2]->lvl)) {
        section[nn].RGBWM[4]->lvl = section[nn].RGBWM[0]->lvl;  //0 is highest
        section[nn].RGBWM[1]->lvl = uint16_t(TABLE_SIZE * (1.0 * section[nn].RGBWM[1]->lvl / section[nn].RGBWM[0]->lvl));
        section[nn].RGBWM[2]->lvl = uint16_t(TABLE_SIZE * (1.0 * section[nn].RGBWM[2]->lvl / section[nn].RGBWM[0]->lvl));
        section[nn].RGBWM[0]->lvl = TABLE_SIZE;
        
    } else if ((section[nn].RGBWM[1]->lvl >= section[nn].RGBWM[0]->lvl) && (section[nn].RGBWM[1]->lvl >= section[nn].RGBWM[2]->lvl)) {
        section[nn].RGBWM[4]->lvl = section[nn].RGBWM[1]->lvl;  //1 is highest
        section[nn].RGBWM[0]->lvl = uint16_t(TABLE_SIZE * (1.0 * section[nn].RGBWM[0]->lvl / section[nn].RGBWM[1]->lvl));
        section[nn].RGBWM[2]->lvl = uint16_t(TABLE_SIZE * (1.0 * section[nn].RGBWM[2]->lvl / section[nn].RGBWM[1]->lvl));
        section[nn].RGBWM[1]->lvl = TABLE_SIZE;
    
    } else if ((section[nn].RGBWM[2]->lvl >= section[nn].RGBWM[0]->lvl) && (section[nn].RGBWM[2]->lvl >= section[nn].RGBWM[1]->lvl)) {
        section[nn].RGBWM[4]->lvl = section[nn].RGBWM[2]->lvl;  //2 is highest
        section[nn].RGBWM[0]->lvl = uint16_t(TABLE_SIZE * (1.0 * section[nn].RGBWM[0]->lvl / section[nn].RGBWM[2]->lvl));
        section[nn].RGBWM[1]->lvl = uint16_t(TABLE_SIZE * (1.0 * section[nn].RGBWM[1]->lvl / section[nn].RGBWM[2]->lvl));
        section[nn].RGBWM[2]->lvl = TABLE_SIZE;
    }
    
    updateLights(nn);
}