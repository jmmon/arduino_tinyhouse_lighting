void updateLightsDEBUG(uint8_t ii) {
    uint8_t brightnessValue; //index for brightness lookup table
    uint8_t height = (uint16_t(section[ii].RGBW[3] * section[ii].masterBrightness * TABLE_SIZE) / HEIGHT);
    uint8_t width = (uint16_t(section[ii].RGBW[3] * section[ii].masterBrightness * TABLE_SIZE) % WIDTH);
    memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue));
    // looks up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
    if (section[ii].RGBW[3] > 0 && section[ii].masterBrightness > 0 && brightnessValue == 0)
    {
        brightnessValue = 1;
    }

    Serial.print(F("width:")); Serial.print(width);
    Serial.print(F(", height:")); Serial.print(height);
    Serial.print(F(" lvl:")); Serial.print(brightnessValue);
    Serial.print(F(" W: ")); Serial.print(section[ii].RGBW[3]);
    Serial.print(F(" R: ")); Serial.print(section[ii].RGBW[0]);
    Serial.print(F(" G: ")); Serial.print(section[ii].RGBW[1]);
    Serial.print(F(" B: ")); Serial.print(section[ii].RGBW[2]);
    Serial.print(F(" Master brightness: ")); Serial.print(section[ii].masterBrightness);
    Serial.print(F(" t:")); Serial.println(millis());
}