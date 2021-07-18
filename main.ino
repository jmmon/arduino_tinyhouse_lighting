void setup() {
    for (uint8_t i = 0; i < SECTION_COUNT; i++) 
        pinMode(section[i].PIN, INPUT);
    
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
        if (section[i].colorProgress) {
            if (currentTime >= (section[i].colorStartTime + section[i].colorDelayInt)) { //loop is separate for this, so speed change works well
                section[i].colorStartTime += section[i].colorDelayInt;
                if (section[i].mode == (LOW_CYCLE_STARTS_AT + 2)) 
                    progressColorSudden(i);
                else if (section[i].mode == (LOW_CYCLE_STARTS_AT + 1)) 
                    progressColorSmooth(i);
            }
        }
    }

    if (currentTime >= (loopStartTime + LOOP_DELAY_INTERVAL)) {
        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < SECTION_COUNT; i++) {
            uint16_t btnStatus = analogRead(section[i].PIN);

            for (uint8_t b = 0; b < 2; b++) {       // 2 buttons, bottom and top (0 and 1)s

                if (btnStatus <= 255) { // if no button is pressed: (after press)
                    if (section[i]._btn[b]->timePressed > 0) {
                        // "register" a release of a button
                        section[i]._btn[b]->timePressed = 0; // reset
                        section[i]._btn[b]->timeReleased = currentTime; // save the time
                    }
                    else if ((section[i]._btn[b]->timeReleased != 0) && (currentTime >= (section[i]._btn[b]->timeReleased + BTN_RELEASE_TIMER))) 
                        btnRelease(i, b); // after small wait
                    
                } else if ((btnStatus >= (BTN_RESIST[b] - BTN_RESIST_TOLERANCE)) && (btnStatus <= (BTN_RESIST[b] + BTN_RESIST_TOLERANCE))) { // else btnStatus > 255: register press and/or do "held button" actions

                    if (DEBUG) {
                        DEBUG_heldActions(i, b, btnStatus);
                    }
                    if (section[i]._btn[b]->timePressed == 0) {
                        // "register" a press of a button
                        section[i]._btn[b]->pressCt ++;      // count the press
                        section[i]._btn[b]->timePressed = currentTime; // save the time
                    } 
                    else if (currentTime >= (section[i]._btn[b]->timePressed + BTN_FADE_DELAY)) 
                        btnHeldActions(i, b);

                }
            } // button loop
        } // section loop
    } // timer loop
} // void loop
