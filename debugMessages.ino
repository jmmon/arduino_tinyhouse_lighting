void DEBUG_updateLights(uint8_t ii) {
    uint8_t brightnessValue; //index for brightness lookup table
    uint8_t height = (uint16_t(section[ii].RGBW[3] * section[ii].masterLevel * TABLE_SIZE) / HEIGHT);
    uint8_t width = (uint16_t(section[ii].RGBW[3] * section[ii].masterLevel * TABLE_SIZE) % WIDTH);

    // looks up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
    memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue));
    
    if ((section[ii].RGBW[3] > 0) && (section[ii].masterLevel > 0) && (brightnessValue == 0))
        brightnessValue = 1;

    Serial.print(F("table_w:"));    Serial.print(width);
    Serial.print(F(", table_h:"));  Serial.print(height);
    Serial.print(F(" lvl:"));   Serial.print(brightnessValue);
    Serial.print(F(" W: "));    Serial.print(section[ii].RGBW[3]);
    Serial.print(F(" R: "));    Serial.print(section[ii].RGBW[0]);
    Serial.print(F(" G: "));    Serial.print(section[ii].RGBW[1]);
    Serial.print(F(" B: "));    Serial.print(section[ii].RGBW[2]);
    Serial.print(F(" Master level: ")); Serial.print(section[ii].masterLevel);
    Serial.print(F(" cur_t:")); Serial.println(currentTime);
}

void DEBUG_updateLightsOff(uint8_t ii) {
    Serial.print(F("MasterBrightness: ")); Serial.println(section[ii].masterLevel);
    Serial.print(F("IsOn:")); Serial.println(section[ii].isOn);
}

void DEBUG_heldActions(uint8_t ii, uint8_t bb, uint16_t s) {
    Serial.print(F(" Section:")); Serial.print(ii);
    Serial.print(F(" Pin:")); Serial.print(section[ii].PIN);
    Serial.print(F(" BTNread:")); Serial.print(s); // buttonStatus
    Serial.print(F(" | Fade "));

    if (bb == 0) 
        Serial.print(F("Down"));
    else 
        Serial.print(F("Up"));

    Serial.print(F(": ")); Serial.print(section[ii]._btn[bb]->pressCt); Serial.println(F(" presses"));
}