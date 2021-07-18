void setup() {
    for (uint8_t i = 0; i < SECTION_COUNT; i++) 
        pinMode(sectionC[i].PIN, INPUT);
    
    randomSeed(analogRead(0)); // used to start colorProgress state at a random color
    loopStartTime = millis();
    currentTime = loopStartTime;

    if (DEBUG) {
        Serial.begin(9600); 
        Serial.println(VERSION);

    } else {
        DmxSimple.maxChannel(CHANNELS);
        DmxSimple.usePin(DMX_PIN);

        for (uint8_t i = 1; i <= CHANNELS; i++)
            DmxSimple.write(i, 0); // turn off all light channels
    }
}


void loop() {
    currentTime = millis();

    for (uint8_t i = 0; i < SECTION_COUNT; i++) {
        if (sectionC[i].colorProgress) {
            if (currentTime >= (sectionC[i].colorStartTime + sectionC[i].colorDelayInt)) { //loop is separate for this, so speed change works well
                sectionC[i].colorStartTime += sectionC[i].colorDelayInt;
                if (sectionC[i].mode == (LOW_CYCLE_STARTS_AT + 2)) 
                    sectionC[i].progressColorSudden();
                else if (sectionC[i].mode == (LOW_CYCLE_STARTS_AT + 1)) 
                    sectionC[i].progressColorSmooth();
            }
        }
    }

    if (currentTime >= (loopStartTime + LOOP_DELAY_INTERVAL)) {
        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < SECTION_COUNT; i++) {
            uint16_t btnStatus = analogRead(sectionC[i].PIN);

            for (uint8_t b = 0; b < 2; b++) {       // 2 buttons, bottom and top (0 and 1)s

                if (btnStatus <= 255) { // if no button is pressed: (after press)
                    if (sectionC[i]._btnC[b]->timePressed > 0) {
                        // "register" a release of a button
                        // sectionC[i]._btnC[b]->timePressed = 0; // reset
                        // sectionC[i]._btnC[b]->timeReleased = currentTime; // save the time
                        sectionC[i]._btnC[b]->registerRelease();
                    }
                    else if ((sectionC[i]._btnC[b]->timeReleased != 0) && (currentTime >= (sectionC[i]._btnC[b]->timeReleased + BTN_RELEASE_TIMER))) 
                        btnRelease(i, b); // after small wait
                    
                } else if ((btnStatus >= (BTN_RESIST[b] - BTN_RESIST_TOLERANCE)) && (btnStatus <= (BTN_RESIST[b] + BTN_RESIST_TOLERANCE))) { // else btnStatus > 255: register press and/or do "held button" actions

                    if (DEBUG) {
                        Serial.print(F(" Section:")); Serial.print(i);
                        Serial.print(F(" Pin:")); Serial.print(sectionC[i].PIN);
                        Serial.print(F(" BTNread:")); Serial.print(btnStatus); // btnStatus
                        Serial.print(F(" | Fade "));

                        if (b == 0) 
                            Serial.print(F("Down"));
                        else 
                            Serial.print(F("Up"));

                        Serial.print(F(": ")); Serial.print(sectionC[i]._btnC[b]->pressCt); Serial.println(F(" presses"));
                    }

                    if (sectionC[i]._btnC[b]->timePressed == 0) {
                        sectionC[i]._btnC[b]->registerPress();
                        // "register" a press of a button
                        // sectionC[i]._btnC[b]->pressCt ++;      // count the press
                        // sectionC[i]._btnC[b]->timePressed = currentTime; // save the time
                    } 
                    else if (currentTime >= (sectionC[i]._btnC[b]->timePressed + BTN_FADE_DELAY)) 
                        btnHeldActions(i, b);

                }
            } // button loop
        } // section loop
    } // timer loop
} // void loop
