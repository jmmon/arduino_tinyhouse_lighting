void setup() {
    // for (uint8_t i = 0; i < SECTION_COUNT; i++) 
    //     pinMode(section[i].PIN, INPUT);
    
    randomSeed(analogRead(0)); // used to start colorProgress state at a random color

    loopStartTime = millis();
    currentTime = loopStartTime;

    if (DEBUG) {
        Serial.begin(9600); 
        Serial.println(VERSION);

    } else {
        DmxSimple.maxChannel(CHANNELS);
        DmxSimple.usePin(DMX_PIN);

        for (uint8_t i = 1; i <= SECTION_COUNT; i++)
            for (uint8_t color = 0; color < 4; color++)
                DmxSimple.write((section[i].DMX_OUT * 8) - 8 + (color * 2 + 1), 0); // turn off all light channels
            
    }
}


void loop() {
    currentTime = millis();

    for (uint8_t i = 0; i < SECTION_COUNT; i++) {
        if (section[i].colorProgress) {
            if (currentTime >= (section[i].colorStartTime + section[i].colorDelayInt)) { //loop is separate for this, so speed change works well
                section[i].colorStartTime += section[i].colorDelayInt;
                if (section[i].mode == (LOW_CYCLE_STARTS_AT + 2)) 
                    section[i].progressColorSudden();
                else if (section[i].mode == (LOW_CYCLE_STARTS_AT + 1)) 
                    section[i].progressColorSmooth();
            }
        }
    }

    if (currentTime >= (loopStartTime + LOOP_DELAY_INTERVAL)) {
        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < SECTION_COUNT; i++) {
            uint16_t btnStatus = analogRead(section[i].PIN);

            for (uint8_t b = 0; b < 2; b++) {       // 2 buttons, bottom and top (0 and 1)s

                if (btnStatus <= 255) { // if no button is pressed: (after press)
                    if (section[i].btn[b].timePressed > 0) {
                        // "register" a release of a button
                        section[i].btn[b].registerRelease();
                        // section[i].btn[b].timePressed = 0; // reset
                        // section[i].btn[b].timeReleased = currentTime; // save the time
                    }
                    else if ((section[i].btn[b].timeReleased != 0) && (currentTime >= (section[i].btn[b].timeReleased + BTN_RELEASE_TIMER))) 
                        btnRelease(i, b); // after small wait
                    
                } else if ((btnStatus >= (BTN_RESIST[b] - BTN_RESIST_TOLERANCE)) && (btnStatus <= (BTN_RESIST[b] + BTN_RESIST_TOLERANCE))) { // else btnStatus > 255: register press and/or do "held button" actions

                    if (DEBUG) {
                        Serial.print(F(" Section:")); Serial.print(i);
                        Serial.print(F(" Pin:")); Serial.print(section[i].PIN);
                        Serial.print(F(" BTNread:")); Serial.print(btnStatus); // buttonStatus
                        Serial.print(F(" | Fade "));

                        if (b == 0) 
                            Serial.print(F("Down"));
                        else 
                            Serial.print(F("Up"));

                        Serial.print(F(": ")); Serial.print(section[i].btn[b].pressCtr); Serial.println(F(" presses"));
                    }

                    if (section[i].btn[b].timePressed == 0) {
                        section[i].btn[b].registerPress(); // "register" a press of a button
                    } 
                    else if (currentTime >= (section[i].btn[b].timePressed + BTN_HELD_DELAY)) 
                        btnHeldActions(i, b);

                }
            } // button loop
        } // section loop
    } // timer loop
} // void loop
