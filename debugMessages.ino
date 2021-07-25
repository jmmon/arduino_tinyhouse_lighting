void DEBUG_updateLights(uint8_t ii) {

}

void DEBUG_updateLightsOff(uint8_t ii) {

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

    Serial.print(F(": ")); Serial.print(section[ii]._btn[bb]->pressCtr); Serial.println(F(" presses"));
}