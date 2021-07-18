uint8_t lookupTable(uint8_t ii, uint8_t ccolor) {
    uint8_t temp = 0;

    uint8_t height = (uint16_t(sectionC[ii].RGBW[ccolor] * sectionC[ii].masterLevel * TABLE_SIZE) / HEIGHT);
    uint8_t width = (uint16_t(sectionC[ii].RGBW[ccolor] * sectionC[ii].masterLevel * TABLE_SIZE) % WIDTH);

    // look up brighness from table and saves as temp ( sizeof(temp) resolves to 1 [byte of data])
    memcpy_P(&temp, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(temp)); 

    if ((sectionC[ii].RGBW[ccolor] > 0) && (sectionC[ii].masterLevel > 0) && (temp == 0)) 
        temp = 1;
    
    if (DEBUG) {
        Serial.print(F("table_w:"));    Serial.print(width);
        Serial.print(F(", table_h:"));  Serial.print(height);
    }
    return temp;
}