void setup() {
    for (uint8_t i = 0; i < SECTION_COUNT; i++) 
        pinMode(section[i].PIN_IN, INPUT);
    
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
/// colorProg loop is looping every n ms, where n = colorProgLoopInt * COLOR_PROG_DELAY_FACTOR_S.....
////// colorProgLoopInt is limited to COLOR_PROG_LOOP_INT_M.. (1, 2000)
//// This means loop is limited to ( (1 * 1) ~ (2000 * 1) ms for SMOOTH ) || ( (1 * 100) ~ (2000 * 100) ms for SUDDEN )

void loop() {
    currentTime = millis();

    for (uint8_t i = 0; i < SECTION_COUNT; i++) {
        if (section[i].colorProgress) {
            /**
             * @brief 

             so:
             if smooth mode,
                loop time should be 20ms, and call the smooth function

            if sudden mode,
                loop time should be 20 * factor ms, and call the sudden function
             * 
             */
            if (section[i].mode == (LOW_CYCLE_STARTS_AT + 1)) {
                if (currentTime >= (section[i].colorStartTime + COLOR_PROG_CTR_DFLT)) {
                    section[i].colorStartTime += COLOR_PROG_CTR_DFLT;
                    progressColorSmooth(i);
                }

            } else  if (section[i].mode == (LOW_CYCLE_STARTS_AT + 2)) {
                uint16_t interval = COLOR_PROG_CTR_DFLT * ((COLOR_PROG_FADE_FACTOR_MAX + 1 - (section[i].colorProgFadeFactor * COLOR_PROG_FADE_FACTOR_DEFAULT)) + 1);
                

                if (currentTime >= (section[i].colorStartTime + interval)) {
                    section[i].colorStartTime += interval; //higher factor should reduce interval
                    progressColorSudden(i);
                }
            }
        }
    }

    if (currentTime >= (loopStartTime + MAIN_LOOP_DELAY_INTERVAL)) {
        loopStartTime += MAIN_LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < SECTION_COUNT; i++) {
            uint16_t btnStatus = analogRead(section[i].PIN_IN);

            for (uint8_t b = 0; b < 2; b++) {       // 2 buttons, bottom and top (0 and 1)

                if (btnStatus <= 255) { // if no button is pressed: (after press)
                    if (section[i].btn[b].timePressed > 0) {
                        section[i].btn[b].registerRelease();
                    }
                    else if ((section[i].btn[b].timeReleased != 0) && (currentTime >= (section[i].btn[b].timeReleased + BTN_RELEASE_TIMER))) {
                        btnRelease(i, b); // after small wait
                    }
                    
                } else if ( (btnStatus >= (section[i].btn[b].RESISTANCE - BTN_RESIST_TOLERANCE))
                        && (btnStatus <= (section[i].btn[b].RESISTANCE + BTN_RESIST_TOLERANCE))) { // else btnStatus > 255: register press and/or do "held button" actions

                    if (DEBUG) {
                        Serial.print(F(" Section:")); Serial.print(i);    Serial.print(F(" Pin:")); Serial.print(section[i].PIN_IN);
                        Serial.print(F(" BTNread:")); Serial.print(btnStatus);    Serial.print(F(" | Fade ")); Serial.print((b == 0) ? F("Down") : F("Up"));
                        Serial.print(F(": ")); Serial.print(section[i].btn[b].pressCt);   Serial.println(F(" presses"));
                    }

                    if (section[i].btn[b].timePressed == 0) {
                        section[i].btn[b].registerPress();
                    } 
                    else if (currentTime >= (section[i].btn[b].timePressed + BTN_FADE_DELAY)) {
                        btnHeld(i, b);
                    }

                }
            } // button loop
        } // section loop
    } // timer loop
} // void loop
