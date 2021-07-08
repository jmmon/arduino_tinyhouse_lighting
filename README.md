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





/* The color fading pattern https://github.com/mathertel/DMXSerial/blob/master/examples/DmxSerialSend/DmxSerialSend.ino ******* COLOR ChANGING THRU DMX

   WORKING 6/24 basic functionality
   WORKS AS EXPECTED except extra color changing functionality
   WORKS WELL




  TWO BUTTON SYSTEM (up and down):
  
  TRIPLE PRESS TOP from on or off: MAX BRIGHTNESS on this section

  TRIPLE HOLD TOP from on or off:

  TRIPLE PRESS BOTTOM from off: if no lightsections are on, start disco mode.
  TRIPLE PRESS BOTTOM from on: if only PORCH is on, turn PORCH off.
                                if lights are on, turn all EXCEPT PORCH off. (If porch isn't on, turn all lights off.)
  TRIPLE HOLD BOT from on or off:

  DOUBLE PRESS TOP from off:  change to next mode and turn on
  DOUBLE PRESS TOP from on: CHANGE TO NEXT MODE. [white mode, color mode (cycle or no), white+color mode (white at 50% of color)] + extra hidden index 3:[all lights max, not included in cycle change, only for triple press]
  DOUBLE HOLD TOP from off:
  DOUBLE HOLD TOP from on:

  DOUBLE PRESS BOT from off: turn on last mode.
  DOUBLE PRESS BOT from on: change back a mode?
  DOUBLE HOLD BOT from off:
  DOUBLE HOLD BOT from on:

  
  PRESS TOP from off: turn on light to default or to lastBrightness if available
  PRESS TOP from on: cycle light brightness steps (35, 70, 100)
  HOLD TOP from off: fade up white light
  HOLD TOP from on: fade up current mode.

  PRESS BOT from off: turn on nightlight mode (red and/or ww)
  PRESS BOT from on: turn off.
  HOLD BOT from off: turn on nightlight mode and fade down
  HOLD BOT from on: fade down current mode.



  Default mode (WW):  [0]
  tap top: if wasOff turn on LAST BRIGHTNESS LEVEL (or default if it was 0). if wasOn, cycle through [low, med, bright] starting from where the brightness was
  hold top: increase brightness
  hold bottom: decrease brightness
  tap bottom: turn off

  color change mode   [1]
  up press: pause, resume color change mode
  up hold: fade up
  down hold: fade down
  down press: cancel color change mode, set at equivalent brightness to colorChangeMode.brightness

  color change (sudden rather than smooth)   [2]?
  tap top: pause, resume color change mode
  up hold: fade up
  down hold: fade down
  down press: cancel color change mode, set at equivalent brightness to colorChangeMode.brightness

  Max brightness (RGBW):  [3]   //not included in normal cycle; "hidden"
    tap top: (if on) {if isMax {go default brightness} else {go max brightness}}  //toggle between max and medium, maybe make 4 steps?
    hold top: increase brightness
    hold bottom: decrease brightness
    tap bottom: turn off

  Nightlight (R): [4]    //special case, tap bottom from [off]
    tap top: cycle up brightness (low, medium, high) OR (medium, high, low)
    hold top: fade
    hold bottom: fade
    tap bottom: turn off

  (G): [5]
  (B): [6]


  so, section needs to save a mode for it's current mode, and using that mode along with whether the color is on or off, decide what to do.
  still only need to deal with single, double, triple presses, and hold functions for single press.


   On press, add 1 to press counter   //check if buttons have been released (releaseTimer)??      (Start of buttonSequence
      start heldDelayTimer //delay before it starts to adjust
   On release
      start release timer    //max time between release and new press

   if heldDelayTImer is ended, check state of button.   //held long enough for fade up / down ( end of buttonSequence )
    if held, (if releaseTimer is 0 (button has not been released lately) == double check)
      then held, do held action (based on pressCounter will do held action for 1, 2, or 3 presses)
    if not held,    //no action

   after end of releaseTimer, check button?       //quarter second after release of first, second, third press
    if button == 0 && releaseTimer == 0     //no additional button was pressed
      end of this buttonSequence
      start action based on buttonCount
      reset button count
    if button was pressed
      do nothing
    (register an additional buttonpress with "On Press" so postpone action until decision is finalized)
*/


/*
Notes: current system:
(Loops every 20 ms to register signal of button 0, 1, or no signal)
Button is pressed, timer starts for that button
  While button is pressed {
    (check for release)
    if held, check if held long enough to start fading the light
    if release {
      reset start timer, initiate new timer for release
      if was fading, stop fading
    Once new timer is up {
      reset timer (stop counting multipresses)
      if not fading (indicates a button tap), turn on light and/or go brighter one level
button cycle now complete



How to add colorProgress speed adjustment? currently refreshes every cycle (20ms), but if I want it to go faster it needs to refresh faster
    so, put colorProgesss on its own separate timer.

    So in the loop, I'll have a colorProgress timer, and for smooth it will refresh every (n) ms (initializing at 20ms)
        and for sudden it should refresh every (n) ms (initializing at maybe 3000ms)

    double-press and hold will increase or decrease n to a max of x or a min of 1ms

*/



// [white mode, color mode (cycle or no), white+color mode (white at 50% of color)] + extra hidden index 3:[all lights max, not included in cycle change, only for triple press]



//implement feedback blink? On certain actions, call a function to blink a color on a certain lightsection really quickly, to let the user know something happened. For example: when turning the porch light on, blip the underloft light green really quickly.
//                      when turning the porch light off, blip the underloft light red really quickly. (Or different colors, but similar idea.)

//This might mean that the state must be "saved" and returned to, for the section that was used for the "blink action".





in nightlight mode, double tap to switch to blue then green? (done)









TODO
when in nightlight mode, fading to off should switch out of nightlight mode 
?? or not? stay in "red" mode?

mode 7: 
Then all the previous levels combined? haha

Same with white: after hitting max brightness, fading up again turns on rgb as a white, then fading down will fade rgb then w
colorProgress: after hitting max brightness, next time it fades it adds white? then fading down first fades down white then fades color.

double press and hold on white fades ALL sets with white? or all?
(Double press and hold on color fades ALL sets with color) or all?
    - Don't adjust the original white level - instead use a second variable so they can stay in sync with each other in terms of starting difference in brightness level






Brightness variables:

Master level - controls all light  (set to 1 by default so basically is ignored)

    White level - controls W

    Color level - controls RGB
        R level
        G level
        B level

So:
    masterLevel
    RGBL[4] => r, g, b, colorLevel
    W => white

Or:
    masterLevel
    colorLevel
    RGBW[4]


Add mode 0 for off? (use turn off light section function)

Mode:   required:        fade options:  
    0       ww      Reg: white || master                        
            RGBWW:  Ext: color || RGB

    1/2     RGB     Reg: color || master
            RGBWW   Ext: white

    (3       N/A)

    4/5/6   RGB     Reg: RGB || color || master
            RGBWW   Ext: white

    7       RGB     Reg: color || master
            RGBWW   Ext: white

master as main fade ctrl:       
    modes: ALL

White as extension:             color || RGB (individuals) as extension:
    modes: 1,2,4,5,6,7              mode: 0

So no need for "color" variable!

*************************************************************************************



Revised:
    Mode:   required:        fade options:  
        0:       
                w       Reg: master
                RGBW:   Ext: RGB
        1/2:
                RGB     Reg: master
                RGBW    Ext: white
        4/5/6
                RGB     Reg: RGB (save the levels)   // don't wipe rgb levels when in CYCLE_HIGH
                RGBW    Ext: white

        7: (on activation, take RGB values and normalize them to max and set the initial master brightness down to tune it to a level comparable to the previous RGB values)
        
                RGB     Reg: master
                RGBW    Ext: white

                        normalize:
                            [highest, middle, low] = getHighest(r, g, b)
                            masterLevel = highest
                            low = low / highest     // if low was 0.01 and high was 0.05, low == 1/5 == 0.2
                            middle = middle / highest // if middle was 0.02, middle == 2/5 == 0.4
                            highest = 1         // highest now set to max

                            // now all are set to use master as fade

        (3:      N/A )




*************************************************************************************

Modes using masterLevel as main fade ctrl:      ALL!

White as extension:             color || RGB (individuals) as extension:
    modes: 1,2,4-7              mode: 0

Mode without extension:     3