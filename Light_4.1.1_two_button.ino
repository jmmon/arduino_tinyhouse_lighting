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

  hidden[...2](
  Max brightness (RGBW):  [3]   //not included in normal cycle; "hidden"
    tap top: (if on) {if isMax {go default brightness} else {go max brightness}}  //toggle between max and medium, maybe make 4 steps?
    hold top: increase brightness
    hold bottom: decrease brightness
    tap bottom: turn off

  Nightlight (RW): [4]    //special case, tap bottom from [off]
    tap top: cycle up brightness (low, medium, high) OR (medium, high, low)
    hold top: fade
    hold bottom: fade
    tap bottom: turn off
  )

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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
#include <avr/pgmspace.h>
#include <DmxSimple.h>
const String VERSION = "Light_4.0.0: Structs and Light ID\n - SERIAL Enabled; DMX Disabled";
const bool DEBUG = false; // DEBUG }}

const uint8_t DMX_PIN = 3;

const uint8_t KITCHEN_BUTTON_PIN = A3;
const uint8_t ENTRY_BUTTON_PIN = A4;
const uint8_t ENTRY2_BUTTON_PIN = A2;
const uint8_t BATH_BUTTON_PIN = A5;
//const uint8_t SCONCE_BUTTON_PIN = A2;
//const uint8_t BACKWALL_BUTTON_PIN = A5;

//light values lookup table, built on an S curve, to make LED dimming feel linear. 1024 stages.
//Storing in PROGMEM allows for such a large array (1023 values). Without PROGMEM, an array of 512 values pushed the limit of dynamic memory but now it's ~51%.
const uint8_t WIDTH = 32;
const uint8_t HEIGHT = 32;
const uint16_t TABLE_SIZE = 1023;
const uint8_t DIMMER_LOOKUP_TABLE[HEIGHT][WIDTH] PROGMEM = {
{ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, },
{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, },

{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, },
{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, },
{ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, },
{ 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, },

{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, },
{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, },
{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, },
{ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, },

{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, },
{ 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, },
{ 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, },
{ 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, },

{ 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, },
{ 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, },
{ 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, },
{ 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 32, 32, 32, },

{ 32, 32, 32, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 38, 38, },
{ 38, 38, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 41, 41, 41, 41, 42, 42, 42, 42, 42, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, },
{ 45, 46, 46, 46, 46, 47, 47, 47, 47, 48, 48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 52, 52, 52, 52, 53, 53, 53, 54, },
{ 54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, },

{ 64, 64, 65, 65, 65, 66, 66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 71, 71, 71, 72, 72, 73, 73, 73, 74, 74, 75, 75, 75, 76, },
{ 76, 77, 77, 77, 78, 78, 79, 79, 80, 80, 80, 81, 81, 82, 82, 83, 83, 84, 84, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, },
{ 91, 91, 92, 92, 93, 93, 94, 94, 95, 95, 96, 96, 97, 97, 98, 98, 99, 99, 100, 100, 101, 102, 102, 103, 103, 104, 104, 105, 105, 106, 107, 107, },
{ 108, 108, 109, 110, 110, 111, 111, 112, 113, 113, 114, 114, 115, 116, 116, 117, 118, 118, 119, 119, 120, 121, 121, 122, 123, 123, 124, 125, 125, 126, 127, 127, },

{ 128, 129, 130, 130, 131, 132, 132, 133, 134, 135, 135, 136, 137, 138, 138, 139, 140, 141, 141, 142, 143, 144, 144, 145, 146, 147, 148, 148, 149, 150, 151, 152, },
{ 152, 153, 154, 155, 156, 157, 157, 158, 159, 160, 161, 162, 163, 164, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 175, 176, 177, 178, 179, 180, },
{ 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 208, 209, 210, 211, 212, 213, 214, },
{ 216, 217, 218, 219, 220, 222, 223, 224, 225, 226, 228, 229, 230, 231, 233, 234, 235, 236, 238, 239, 240, 242, 243, 244, 246, 247, 248, 250, 251, 252, 254, 255, },
};


const uint8_t CHANNELS = 64;
const uint8_t MAX_PRESS_COUNT = 6;
const uint16_t BUTTON_RES[2] = {566, 1023};     // [0]bottom button returns ~3.3v    // [1]top button returns ~5v
const uint8_t BUTTON_RESISTANCE_TOLERANCE = 30; // +/-
const uint16_t BUTTON_RELEASE_TIMER = 250;      // time before release action is processed; time allowed between release and next press for multipresses
const uint8_t BUTTON_FADE_DELAY = 230;          // minimum time the button must be held to start "held" action;

const uint16_t BUTTON_FADE_DELAY_RAPID = 410;       //after this time, accellerate fading speed (double) so it doesn't take so long to fade to max/min

//brightness value = index of large lookup table above
const float DEFAULT_BRIGHTNESS = 0.40; // 0-1 (percent) value for default brightness when turned on
const float FADE_FACTOR = 0.0050;      //base fade adjustment; modified by section[].BRIGHTNESS_FACTOR

const uint8_t LOOP_DELAY_INTERVAL = 20; // refresh speed in ms          {for ms=1: factor=0.00001; amount=0.018}
uint32_t loopStartTime = 0;             // loop start time
const float MIDDLE = 0.878;
const float _HIGH = 0.995;
const float _LOW = 0.6;
//const float RED_LIST[] = {1., MIDDLE, 0, 0, 0, MIDDLE}; // colorProgress cycle layout
//const float GREEN_LIST[] = {0, MIDDLE, 1., MIDDLE, 0, 0};
//const float BLUE_LIST[] = {0, 0, 0, MIDDLE, 1., MIDDLE};
const float RED_LIST[] = {1., _HIGH, MIDDLE, _LOW, 0, 0, 0, 0, 0, _LOW, MIDDLE, _HIGH}; // colorProgress cycle layout
const float GREEN_LIST[] = {0, _LOW, MIDDLE, _HIGH, 1., _HIGH, MIDDLE, _LOW, 0, 0, 0, 0};
const float BLUE_LIST[] = {0, 0, 0, 0, 0, _LOW, MIDDLE, _HIGH, 1., _HIGH, MIDDLE, _LOW};
const uint8_t COLOR_PROGRESS_DELAY_COUNT = 0;   // n delay cycles per progress cycle
const uint8_t COLOR_PROGRESS_DELAY_SUDDEN = 0;
const float COLOR_PROGRESS_FADE_AMOUNT = 0.001; //modified if faded so interval is the same if faded. fade not yet implemented.

const uint8_t NUM_OF_MODES_CYCLE = 3;
const uint8_t LIGHTSECTION_COUNT = 4;

const uint16_t COLOR_PROGRESS_SMOOTH_DELAY_INIT = 20; //ms
const uint16_t COLOR_PROGRESS_SUDDEN_DELAY_INIT = 3000; //ms

//********************************************************************************************************************************************************

//                                              Structs

//********************************************************************************************************************************************************

struct button_t
{

    uint32_t releaseTimer; //when was this button released?
    uint32_t pressedTime;  //when was this button pressed?
    uint16_t pressedDuration;  // time from press to release
    uint8_t pressedCount;  //If button is pressed before releaseTimer ends, add one to count
    bool beingHeld;        //is the button being held? (for longer than BUTTON_FADE_DELAY

} button[] = {

    {0, 0, 0, 0, false}, //entry button up         inside underloft
    {0, 0, 0, 0, false}, //entry button down

    {0, 0, 0, 0, false}, //entry2 button up       outside
    {0, 0, 0, 0, false}, //entry2 button down

    {0, 0, 0, 0, false}, //kitchen left wall
    {0, 0, 0, 0, false}, //kitchen

    {0, 0, 0, 0, false}, //bath right nook
    {0, 0, 0, 0, false},
    //Back wall button
    //sconce 1 button close button
};


struct section_t
{

    float RGBW[4];     //stores current RGBW color levels
    float lastRGBW[4]; //last color levels
    float masterBrightness;
    int8_t mode; // 0-4: WW, colors, colors+ww, (All, Nightlight)
    uint8_t lastMode;
    //typical cycle is 0-1-2-0... modes 3 and 4 are hidden from the typical cycle.
    bool isOn;               // Are any levels > 0?
    float BRIGHTNESS_FACTOR; // affects [default brightness + fade speed], pref range [0-1]
    uint8_t DMXout;          // DMX OUT number (set of 4 channels) this section controls
    //colorProgress variables for this section
    uint8_t colorDelayCounter; //slows the color cycle
    uint8_t colorState;        //next color state in the cycle
    bool colorProgress;        //while true, colors for this section will cycle

    uint32_t colorProgressTimerStart;
    uint16_t colorProgressInterval;

    float nextRGB[3]; //next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.
    float colorCycleFadeLevel;
    int8_t colorCycleFadeDir;

    uint8_t PIN;
    button_t *_button[2];

} section[] = {
    {{0., 0., 0., 0.}, {0., 0., 0., 0.}, 1., 0, 0, false, 0.8, 4, 0, 0, false, 0, 0, {0, 0, 0}, 1, 1, ENTRY_BUTTON_PIN, {&button[0], &button[1]}}, //  ID 0    Living Room Lights

    {{0., 0., 0., 0.}, {0., 0., 0., 0.}, 1., 0, 0, false, 1, 3, 0, 0, false, 0, 0, {0, 0, 0}, 1, 1, KITCHEN_BUTTON_PIN, {&button[4], &button[5]}}, //  ID 1    Kitchen Lights

    {{0., 0., 0., 0.}, {0., 0., 0., 0.}, 1., 0, 0, false, 0.9, 2, 0, 0, false, 0, 0, {0, 0, 0}, 1, 1, ENTRY2_BUTTON_PIN, {&button[2], &button[3]}}, //  ID 2   Porch Lights

    {{0., 0., 0., 0.}, {0., 0., 0., 0.}, 1., 0, 0, false, 1.0, 1, 0, 0, false, 0, 0, {0, 0, 0}, 1, 1, BATH_BUTTON_PIN, {&button[6], &button[7]}}, //  ID 3   bath
    //  ID 3    Overhead Bedroom
    //  ID 4    Overhead Main
    //  ID 5    Overhead Small Loft
    //  ID 7    Greenhouse Lights
};

//******************************************************************************************************************************************************** 

//                                          End Variables

//******************************************************************************************************************************************************** 

void updateLights(uint8_t i)
{                            //updates a specific light section
    uint8_t brightnessValue; //index for brightness lookup table

    if (DEBUG == true)
    { // DEBUG }}
        uint8_t height = (uint16_t(section[i].RGBW[3] * section[i].masterBrightness * TABLE_SIZE) / HEIGHT);
        uint8_t width = (uint16_t(section[i].RGBW[3] * section[i].masterBrightness * TABLE_SIZE) % WIDTH);
        memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); //  looks up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
        if (section[i].RGBW[3] > 0 && section[i].masterBrightness > 0 && brightnessValue == 0)
            brightnessValue = 1;
        Serial.print(F("width:"));
        Serial.print(width);
        Serial.print(F(", height:"));
        Serial.print(height);
        Serial.print(F(" lvl:"));
        Serial.print(brightnessValue);
        Serial.print(F(" W: "));
        Serial.print(section[i].RGBW[3]);
        Serial.print(F(" R: "));
        Serial.print(section[i].RGBW[0]);
        Serial.print(F(" G: "));
        Serial.print(section[i].RGBW[1]);
        Serial.print(F(" B: "));
        Serial.print(section[i].RGBW[2]);
        Serial.print(F(" Master brightness: "));
        Serial.print(section[i].masterBrightness);
        Serial.print(F(" t:"));
        Serial.println(millis());
    }
    else
    {
        for (uint8_t k = 0; k < 4; k++)
        {
            //if (section[i].lastRGBW[k] != section[i].RGBW[k])
            //{ //update changed numbers
            uint8_t height = (uint16_t(section[i].RGBW[k] * section[i].masterBrightness * TABLE_SIZE) / HEIGHT);
            uint8_t width = (uint16_t(section[i].RGBW[k] * section[i].masterBrightness * TABLE_SIZE) % WIDTH);
            memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); //  looks up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
            if (section[i].RGBW[k] > 0 && section[i].masterBrightness > 0 && brightnessValue == 0)
                brightnessValue = 1;
            DmxSimple.write(section[i].DMXout * 8 - 8 + (k * 2 + 1), brightnessValue); //main underloft lights
            DmxSimple.write(section[i].DMXout * 8 - 8 + (k * 2 + 1), brightnessValue); //main underloft lights
            section[i].lastRGBW[k] = section[i].RGBW[k];
            //}
        }
    }
    if (section[i].RGBW[3] <= 0 && section[i].RGBW[0] <= 0 && section[i].RGBW[1] <= 0 && section[i].RGBW[2] <= 0)
    {
        section[i].isOn = false;
        section[i].masterBrightness = 0;
        if (DEBUG == true)
        {
            Serial.print(F("MasterBrightness: "));
            Serial.println(section[i].masterBrightness);
            Serial.print(F("IsOn:"));
            Serial.println(section[i].isOn);
        }
    }
}

void masterFadeIncrement(uint8_t i)
{
    if (section[i].masterBrightness < (1 - section[i].BRIGHTNESS_FACTOR * FADE_FACTOR))
        section[i].masterBrightness += section[i].BRIGHTNESS_FACTOR * FADE_FACTOR;
    else
        section[i].masterBrightness = 1; // max
}

void masterFadeDecrement(uint8_t i)
{
    if (section[i].masterBrightness > (section[i].BRIGHTNESS_FACTOR * FADE_FACTOR))
        section[i].masterBrightness -= section[i].BRIGHTNESS_FACTOR * FADE_FACTOR;
    else
        section[i].masterBrightness = 0; // min
}

void progressColor(uint8_t i)
{
    if (section[i].mode == 2) {
        //sudden color changes

        section[i].colorState += 1;
        if (section[i].colorState == 12)
            section[i].colorState = 0;
        if (DEBUG == true)
        {
            Serial.print(F("color progress state: "));
            Serial.println(section[i].colorState);
        }

        //set new light color
        section[i].RGBW[0] = RED_LIST[section[i].colorState];
        section[i].RGBW[1] = GREEN_LIST[section[i].colorState];
        section[i].RGBW[2] = BLUE_LIST[section[i].colorState];

    } else if (section[i].mode == 1) {
        //smooth color changes



        section[i].nextRGB[0] = RED_LIST[section[i].colorState]; // target levels for the current state
        section[i].nextRGB[1] = GREEN_LIST[section[i].colorState];
        section[i].nextRGB[2] = BLUE_LIST[section[i].colorState];

        if ((section[i].nextRGB[0] == section[i].RGBW[0]) && (section[i].nextRGB[1] == section[i].RGBW[1]) && (section[i].nextRGB[2] == section[i].RGBW[2])) // Go to next state
        {
            section[i].colorState += 1;
            if (section[i].colorState == 12)
                section[i].colorState = 0;
            if (DEBUG == true)
            {
                Serial.print(F("color progress state: "));
                Serial.println(section[i].colorState);
            }
        }
        else // else change colors to get closer to current state
        {
            for (uint8_t k = 0; k < 3; k++) //rgb
            {
                if (section[i].RGBW[k] < section[i].nextRGB[k])
                {
                    section[i].RGBW[k] += COLOR_PROGRESS_FADE_AMOUNT;
                    if (section[i].RGBW[k] >= section[i].nextRGB[k])
                        section[i].RGBW[k] = section[i].nextRGB[k];
                }
                else if (section[i].RGBW[k] > section[i].nextRGB[k])
                {
                    section[i].RGBW[k] -= COLOR_PROGRESS_FADE_AMOUNT;
                    if (section[i].RGBW[k] <= section[i].nextRGB[k])
                        section[i].RGBW[k] = section[i].nextRGB[k];
                }
            }
        }

    }
    updateLights(i);
}

void setup() //****************************************************************************************************************************** SETUP
{
    if (DEBUG == true)
    { // DEBUG }}
        Serial.begin(9600);
        Serial.println(VERSION);
    }
    else
    {
        DmxSimple.maxChannel(CHANNELS);
        DmxSimple.usePin(DMX_PIN);
        for (uint8_t i = 1; i <= CHANNELS; i++) //turn off all light channels
            DmxSimple.write(i, 0);
    }
    randomSeed(analogRead(0)); //get random seed; used to start colorProgress state at a random color
    for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++)
    {
        pinMode(section[i].PIN, INPUT);
    }
    loopStartTime = millis();
}

void loop() //********************************************************************************************************************************* LOOP
{
    uint32_t currentTime = millis();

    //colorProgress loop
    //for each section,
        //if colorProgress
            //loop every n ms
                //progressColor

    for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) {
        if (section[i].colorProgress == true) {
            if ((currentTime - section[i].colorProgressTimerStart) >= section[i].colorProgressInterval) {
                section[i].colorProgressTimerStart += section[i].colorProgressInterval;
                progressColor(i);

            }
        }
    }






    if ((currentTime - loopStartTime) >= LOOP_DELAY_INTERVAL) // 20ms loop
    {
        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) // cycle through each section
        {
            // if (section[i].colorProgress == true) {
            //     progressColor(i);
            // }
            

            uint16_t buttonStatus = analogRead(section[i].PIN); // read the button pin to check if any buttons are pressed
            
            // REGISTER RELEASES:
            if (buttonStatus <= 256) // if no button is pressed:
            {
                for (uint8_t b = 0; b < 2; b++) {
                    if (section[i]._button[b]->pressedTime > 0) //  check if button[i] was pressed on the last loop through by checking for non-zero pressedTime
                    {
                        section[i]._button[b]->releaseTimer = currentTime + BUTTON_RELEASE_TIMER; // if so, since it is no longer pressed we start the releaseTimer
                        //section[i]._button[b]->pressedDuration = currentTime - pressedTime;
                        section[i]._button[b]->pressedTime = 0;                                   // and reset pressedTime to 0 so the next press is detected as a new press rather than a held press.
                    }
                    else // else if pressedTime == 0 then button[i] is already "RELEASED":
                    {
                        // if ( currentTime >= section[i]._button[b]->pressedTime + section[i]._button[b]->pressedDuration + BUTTON_RELEASE_TIMER )
                        if (currentTime >= section[i]._button[b]->releaseTimer) // In that case, wait for releaseTimer before commencing "release actions," in case user is attempting a double or triple press. Commmence actions:
                        {
                            // this is where we process the "button was pressed x times" action
                            if (section[i]._button[b]->beingHeld == true) //mark as not held in case it was
                            {
                                section[i]._button[b]->beingHeld = false;
                            }
                            else    //if button is not held commence Release action
                            {
                                uint8_t brightness = 0;
                                if (section[i]._button[b]->pressedCount > 0)
                                {
                                    if (section[i]._button[b]->pressedCount > MAX_PRESS_COUNT)
                                        section[i]._button[b]->pressedCount = MAX_PRESS_COUNT;

                                    switch (section[i]._button[b]->pressedCount) // handle button presses
                                    {
                                    case (3):       // 3 presses
                                        if (b == 1) // top button  action
                                        {
                                            //  TRIPLE PRESS TOP: MAX BRIGHTNESS
                                            if (DEBUG == true)
                                            {
                                                Serial.println(F(" TOP 3 "));
                                                Serial.println(F("Max Brightness {mode:3}"));
                                            }
                                            section[i].mode = 3; // set to mode 3

                                            for (uint8_t k = 0; k < 4; k++)
                                                section[i].RGBW[k] = 1;
                                            section[i].isOn = true;
                                            section[i].masterBrightness = 1;
                                        }
                                        else // bottom button action
                                        {
                                            // TRIPLE PRESS BOTTOM from on: if lights are on, turn off all EXCEPT PORCH. (If porch isn't on, turn all lights off.)
                                            //                              if only PORCH is on, turn PORCH off.
                                            // if no light is on, disco mode.
                                            if (DEBUG == true)
                                            {
                                                Serial.println(F(" BOT 3 "));
                                            }

                                            // if (section[2].isOn == true) {  //if porch is on

                                            // } else {

                                            // }


                                            bool on = false;
                                            for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) // check if any lights are on (except porch)
                                            {
                                                if (section[k].isOn = true && k != 2)
                                                    on = true;
                                            }
                                            if (on == true) // excluding porch, if any are on, turn them off.
                                            {
                                                if (DEBUG == true)
                                                {
                                                    Serial.println(F("Light(s) are on, turn them off (except porch light)"));
                                                }
                                                
                                                for (uint8_t k = 0; k < LIGHTSECTION_COUNT; k++) // turn off all but porch
                                                {
                                                    if (section[k].isOn = true && k != 2) // if the light is on and it's not the porch light,
                                                    {
                                                        section[k].isOn = false; // if "on," set to "off"
                                                        section[k].colorProgress = false;
                                                        for (uint8_t z = 0; z < 4; z++)
                                                            section[k].RGBW[z] = 0; // and set values to 0 for each color for that light
                                                        section[k].masterBrightness = 0;
                                                    }
                                                }
                                            }
                                            else if (section[2].isOn == true) // else if porch is on:
                                            {
                                                if (DEBUG == true)
                                                {
                                                    Serial.println(F("Only porch is on: turning off."));
                                                }

                                                section[2].isOn = false;
                                                section[2].colorProgress = false;
                                                for (uint8_t z = 0; z < 4; z++)
                                                    section[2].RGBW[z] = 0; // and set values to 0 for each color for that light
                                                section[2].masterBrightness = 0;
                                            }
                                            else  // else no lights are on so do:
                                            {
                                                if (DEBUG == true)
                                                {
                                                    Serial.println(F("Lights were off: disco mode!"));
                                                }

                                                // TODO: all is off, disco mode
                                            }
                                        }
                                        break;

                                    case (2):       // 2 presses
                                        if (b == 1) // top button action 
                                        {
                                            // DOUBLE PRESS TOP: turn on if off; and switch to next mode.
                                            // [white mode, color mode (cycle or no), white+color mode (white at 50% of color)] + extra hidden index 3:[all lights max, not included in cycle change, only for triple press]

                                            if (DEBUG == true)
                                            {
                                                Serial.println(F(" TOP 2 "));
                                            }
                                            
                                            if (section[i].isOn == false)
                                            {
                                                // change to next and turn on
                                                section[i].isOn = true;
                                            }
                                            
                                            section[i].mode++;  // if was off, we want value of 1 from 0. If it was on, we increment the value
                                            if (section[i].mode >= NUM_OF_MODES_CYCLE)
                                            {
                                                section[i].mode = 0;
                                            }
                                            if (DEBUG == true)
                                            {
                                                Serial.print(F("Now in mode: "));
                                                Serial.println(section[i].mode);
                                            }

                                            switch (section[i].mode) // turn on the mode:
                                            {
                                            case (0): // white mode from RGB sudden
                                                if (section[i].colorProgress == true)
                                                { // turn off colorProgress
                                                    section[i].colorProgress = false;
                                                    for (uint8_t k = 0; k < 4; k++)
                                                        section[i].RGBW[k] = 0;
                                                }

                                                section[i].RGBW[3] = 1;
                                                section[i].masterBrightness = section[i].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                                                break;
                                            case (1): // RGB smooth from white
                                                // turn off other colors,
                                                // start colorProgress

                                                if (section[i].colorProgress == false)
                                                { //lastMode was not 2
                                                    section[i].colorProgress = true;
                                                    section[i].colorState = random(12); //get a random state to start at

                                                    section[i].RGBW[3] = 0;
                                                    // for (uint8_t k = 0; k < 4; k++)
                                                    //         section[i].RGBW[k] = 0;
                                                        //initialize a state
                                                    section[i].RGBW[0] = RED_LIST[section[i].colorState];
                                                    section[i].RGBW[1] = GREEN_LIST[section[i].colorState];
                                                    section[i].RGBW[2] = BLUE_LIST[section[i].colorState];
                                                    updateLights(i);

                                                    section[i].masterBrightness = section[i].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                                                    section[i].colorProgressTimerStart = currentTime;
                                                    section[i].colorProgressInterval = COLOR_PROGRESS_SMOOTH_DELAY_INIT;
                                                    //progressColor(i);
                                                }

                                                break;
                                            case (2): // RGB sudden from RGB smooth
                                                // start colorProgress

                                                if (section[i].colorProgress == false)
                                                {
                                                    section[i].colorProgress = true;
                                                    section[i].colorState = random(12); // get a random state to start at

                                                    section[i].RGBW[3] = 0;
                                                    // for (uint8_t k = 0; k < 4; k++)
                                                    //         section[i].RGBW[k] = 0;
                                                        //initialize a state
                                                    section[i].RGBW[0] = RED_LIST[section[i].colorState];
                                                    section[i].RGBW[1] = GREEN_LIST[section[i].colorState];
                                                    section[i].RGBW[2] = BLUE_LIST[section[i].colorState];
                                                    updateLights(i);
                                                    
                                                    section[i].masterBrightness = section[i].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                                                    //progressColor(i);
                                                }
                                                section[i].colorProgressTimerStart = currentTime;
                                                section[i].colorProgressInterval = COLOR_PROGRESS_SUDDEN_DELAY_INIT;
                                                break;
                                            }
                                            
                                            
                                        }
                                        else // bottom double press action
                                        {
                                            // DOUBLE PRESS BOT: from on: reduce mode by 1
                                            // DOUBLE PRESS BOT: from off: do nothing
                                            if (DEBUG == true)
                                            {
                                                Serial.println(F(" BOT 2 "));
                                            }
                                            if (section[i].isOn == true)
                                            {
                                                section[i].mode--;
                                                if (section[i].mode < 0)
                                                {
                                                    section[i].mode = NUM_OF_MODES_CYCLE - 1;
                                                }

                                                if (DEBUG == true)
                                                {
                                                    Serial.print(F("Now in mode: "));
                                                    Serial.println(section[i].mode);
                                                }

                                                switch (section[i].mode) // turn on the mode:
                                                {
                                                case (0): // white from RGB smooth
                                                    section[i].colorProgress = false;
                                                    for (uint8_t k = 0; k < 4; k++)
                                                        section[i].RGBW[k] = 0;

                                                    section[i].RGBW[3] = 1;
                                                    section[i].masterBrightness = DEFAULT_BRIGHTNESS;
                                                    break;
                                                case (1): // RGB smooth from RGB sudden
                                                    
                                                    section[i].colorProgress = true;
                                                    section[i].colorState = random(12); // get a random state to start at

                                                    section[i].RGBW[3] = 0;
                                                    // for (uint8_t k = 0; k < 4; k++)
                                                    //         section[i].RGBW[k] = 0;
                                                        //initialize a state
                                                    section[i].RGBW[0] = RED_LIST[section[i].colorState];
                                                    section[i].RGBW[1] = GREEN_LIST[section[i].colorState];
                                                    section[i].RGBW[2] = BLUE_LIST[section[i].colorState];
                                                    updateLights(i);

                                                    section[i].masterBrightness = section[i].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                                                    section[i].colorProgressTimerStart = currentTime;
                                                    section[i].colorProgressInterval = COLOR_PROGRESS_SMOOTH_DELAY_INIT;
                                                    //progressColor(i);

                                                    break;
                                                case (2): // RGB sudden from W
                                                    // start colorProgress

                                                    section[i].colorProgress = true;
                                                    section[i].colorState = random(12); // get a random state to start at

                                                    section[i].RGBW[3] = 0;
                                                    // for (uint8_t k = 0; k < 4; k++)
                                                    //         section[i].RGBW[k] = 0;
                                                        //initialize a state
                                                    section[i].RGBW[0] = RED_LIST[section[i].colorState];
                                                    section[i].RGBW[1] = GREEN_LIST[section[i].colorState];
                                                    section[i].RGBW[2] = BLUE_LIST[section[i].colorState];
                                                    updateLights(i);
                                                    
                                                    section[i].masterBrightness = section[i].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                                                    section[i].colorProgressTimerStart = currentTime;
                                                    section[i].colorProgressInterval = COLOR_PROGRESS_SUDDEN_DELAY_INIT;
                                                    //progressColor(i);

                                                    break;
                                                }
                                            }
                                        }
                                        break;

                                    case (1):        //  1 press
                                        if (b == 1) // top  single press action
                                        {
                                            if (DEBUG == true)
                                            {
                                                Serial.println(F(" TOP 1 "));
                                            }

                                            // PRESS TOP from off: turn on mode0 to default or to lastBrightness if available
                                            // PRESS TOP from on: white:  cycle light brightness steps (35, 70, 100)
                                            //                    RGB:    pause colorProgress
                                            if (section[i].isOn == false) // from off:
                                            {
                                                // TODO: turn on to last brightness if != 0
                                                // else turn on to default brightness
                                                section[i].isOn = true;
                                                section[i].mode = 0;
                                                section[i].RGBW[3] = 1;
                                                section[i].masterBrightness = section[i].BRIGHTNESS_FACTOR * DEFAULT_BRIGHTNESS;
                                            }
                                            else // from on:
                                            {
                                                switch (section[i].mode)
                                                {
                                                case (1): // RGB
                                                case (2): // RGBW
                                                    // for RGB modes, 1 top press pauses or resumes the colorProgress
                                                    if (section[i].colorProgress == true)
                                                    {
                                                        section[i].colorProgress = false;
                                                    }
                                                    else
                                                    {
                                                        section[i].colorProgress = true;
                                                    }
                                                    break;

                                                default: // for other modes, cycle up brightness for colors that are on

                                                    section[i].masterBrightness += 0.2;
                                                    if (section[i].masterBrightness == 1)
                                                    {
                                                        section[i].masterBrightness -= 1;
                                                    }
                                                    else if (section[i].masterBrightness > 1)
                                                    {
                                                        section[i].masterBrightness = 1;
                                                    }
                                                }
                                            }
                                        }
                                        else //  bot single press action
                                        {
                                            // PRESS BOT from off: turn on nightlight mode (red and/or ww)
                                            // PRESS BOT from on: turn off all lights, reset mode;

                                            if (section[i].isOn == false)
                                            {
                                                if (DEBUG == true)
                                                {
                                                    Serial.println(F(" BOT 1: Nightlight "));
                                                }
                                                // turn on night light mode
                                                section[i].isOn = true;
                                                section[i].mode = 4;        //night light
                                                section[i].RGBW[3] = 1;     //turn on white
                                                section[i].RGBW[0] = 1;     //turn on red
                                                section[i].masterBrightness = 0.1;  //set brightness to 10%
                                            }
                                            else // turn off all colors
                                            {
                                                if (DEBUG == true)
                                                {
                                                    Serial.println(F(" BOT 1: Turn Off "));
                                                }
                                                section[i].mode = 0;
                                                section[i].isOn = false;
                                                section[i].colorProgress = false;       //in case it was on, turn of colorProgress
                                                for (uint8_t k = 0; k < 4; k++)
                                                {
                                                    section[i].RGBW[k] = 0;
                                                }
                                                section[i].masterBrightness = 0;
                                            }
                                        }
                                    }
                                    updateLights(i);
                                }
                            }
                            
                            section[i]._button[b]->pressedCount = 0; // after RELEASE ACTIONS, reset pressedCount counter to 0.
                        }                                            // END RELEASE ACTIONS (button press)
                    }
                }
                
            }
            else // else buttonStatus > 255; a button is being pressed.  // PRESSED ACTIONS (register a press, and do "held button" actions)
            {
                if (DEBUG == true) // {{ DEBUG }}
                {
                    Serial.print(F(" section:"));
                    Serial.print(i); // (print button reading)
                    Serial.print(F(" pin:"));
                    Serial.print(section[i].PIN); // (print button reading)
                    Serial.print(F(" "));
                    Serial.print(buttonStatus); // (print button reading)
                    Serial.print(F(" | "));
                }
                if (buttonStatus >= (BUTTON_RES[0] - BUTTON_RESISTANCE_TOLERANCE) && buttonStatus <= (BUTTON_RES[0] + BUTTON_RESISTANCE_TOLERANCE))
                {
                    uint8_t b = 0;
                    // bot button held actions, fade down
                    if (DEBUG == true) // {{ DEBUG }}
                    {
                        Serial.println(F("Fade Down"));
                    }
                    

                    if (section[i]._button[b]->pressedTime == 0) // if button[i].pressedTime == 0 this is a NEW button press
                        {
                            section[i]._button[b]->pressedTime = currentTime; // save the time to detect multipresses
                            section[i]._button[b]->pressedCount++;            // add one press to its counter
                        }
                        else if (currentTime >= section[i]._button[b]->pressedTime + BUTTON_FADE_DELAY) // if button has been pressed and held past BUTTON_FADE_DELAY, button is being held:
                        {
                            if (section[i]._button[b]->beingHeld == false) // if not yet held, initialize fading:
                            {
                                section[i]._button[b]->beingHeld = true;
                            }

                            if (currentTime >= section[i]._button[b]->pressedTime + BUTTON_FADE_DELAY_RAPID)    //after QUICK_DELAY time, decrement an additional time per loop (double speed)
                            {
                                masterFadeDecrement(i);
                            }
                            masterFadeDecrement(i);       //regular decrement
                            updateLights(i);


                        }
                    


                } else if (buttonStatus >= (BUTTON_RES[1] - BUTTON_RESISTANCE_TOLERANCE) && buttonStatus <= (BUTTON_RES[1] + BUTTON_RESISTANCE_TOLERANCE)) {
                    uint8_t b = 1;
                    // top button held actions, fade up
                    if (DEBUG == true) // {{ DEBUG }}
                    {
                        Serial.println(F("Fade Up"));
                    }

                    if (section[i]._button[b]->pressedTime == 0) // if button[i].pressedTime == 0 this is a NEW button press
                        {
                            section[i]._button[b]->pressedTime = currentTime; // save the time to detect multipresses
                            section[i]._button[b]->pressedCount++;            // add one press to its counter
                        }
                        else if (currentTime >= section[i]._button[b]->pressedTime + BUTTON_FADE_DELAY) // if button has been pressed and held past BUTTON_FADE_DELAY, button is being held:
                        {
                            if (section[i]._button[b]->beingHeld == false) // if not yet held, initialize fading:
                            {
                                section[i]._button[b]->beingHeld = true;
                                section[i].isOn = true;
                                if (section[i].mode == 0)
                                {
                                    section[i].RGBW[3] = 1;
                                }
                                
                            }

                            if (currentTime >= section[i]._button[b]->pressedTime + BUTTON_FADE_DELAY_RAPID)    //after QUICK_DELAY time, increment an additional time per loop (double speed)
                            {
                                masterFadeIncrement(i);
                            }
                            masterFadeIncrement(i);       //regular increment
                            updateLights(i);



                        }


                    
                }
                
            } // end {button held} thread
        }     // end {check each section} loop
    }         // update timer
} // void loop
