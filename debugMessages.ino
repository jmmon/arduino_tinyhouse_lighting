void DEBUG_updateLights(uint8_t ii) {
    //uint8_t brightnessValue = lookupTable(ii, 3); //index for brightness lookup table
    uint8_t brightnessValue = 0; //index for brightness lookup table

    uint8_t height = (uint16_t(sectionC[ii].RGBW[3] * sectionC[ii].masterLevel * TABLE_SIZE) / HEIGHT);
    uint8_t width = (uint16_t(sectionC[ii].RGBW[3] * sectionC[ii].masterLevel * TABLE_SIZE) % WIDTH);

    // look up brighness from table and saves as temp ( sizeof(temp) resolves to 1 [byte of data])
    memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

    if ((sectionC[ii].RGBW[3] > 0) && (sectionC[ii].masterLevel > 0) && (brightnessValue == 0)) 
        brightnessValue = 1;
    

    Serial.print(F("table_w:"));    Serial.print(width);
    Serial.print(F(", table_h:"));  Serial.print(height);
    
    Serial.print(F(" lvl:"));   Serial.print(brightnessValue);
    Serial.print(F(" W: "));    Serial.print(sectionC[ii].RGBW[3]);
    Serial.print(F(" R: "));    Serial.print(sectionC[ii].RGBW[0]);
    Serial.print(F(" G: "));    Serial.print(sectionC[ii].RGBW[1]);
    Serial.print(F(" B: "));    Serial.print(sectionC[ii].RGBW[2]);
    Serial.print(F(" Master level: ")); Serial.print(sectionC[ii].masterLevel);
    Serial.print(F(" cur_t:")); Serial.println(currentTime);
}

void DEBUG_updateLightsOff(uint8_t ii) {
    Serial.print(F("MasterBrightness: ")); Serial.println(sectionC[ii].masterLevel);
    Serial.print(F("IsOn:")); Serial.println(sectionC[ii].isOn);
}

void DEBUG_heldActions(uint8_t ii, uint8_t bb, uint16_t s) {
    Serial.print(F(" Section:")); Serial.print(ii);
    Serial.print(F(" Pin:")); Serial.print(sectionC[ii].PIN);
    Serial.print(F(" BTNread:")); Serial.print(s); // buttonStatus
    Serial.print(F(" | Fade "));

    if (bb == 0) 
        Serial.print(F("Down"));
    else 
        Serial.print(F("Up"));

    Serial.print(F(": ")); Serial.print(sectionC[ii]._btnC[bb]->pressCt); Serial.println(F(" presses"));
}