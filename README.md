# arduino_lighting_two_button
Arduino lighting using one input pin per pair of buttons (up/down) and DMX output for RGBW LED strips




Layout idea slightly different:

each section has its buttons

timer loop {
    for each section {
        if progressColor {
            progressColor()

        }
        if this section's buttonSignal is LOW {
            for each button {   //gotta process both buttons
                checkIfTimerIsUpToProcessTheActionFromThePress() -> depending on #ofPresses, do action
                checkIfRecentlyReleased()
            }

        } else if this section's buttonSignal is MEDIUM {   //processing the buttons for pressed actions
            //buttonBotBeingPressed
            checkIfTimerIsDoneToInitiateFade()

        } else is HIGH {
            //buttonTopBeingPressed
            checkIfTimerIsDoneToInitiateFade()

        }
    }
}