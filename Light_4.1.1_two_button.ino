#include <avr/pgmspace.h>
#include <DmxSimple.h>
const String VERSION = "Light_4.0.0: Structs and Light ID\n - SERIAL Enabled; DMX Disabled";
const bool DEBUG = false;

const uint8_t   KITCHEN_BUTTON_PIN  =   A3,
                ENTRY_BUTTON_PIN    =   A4,
                ENTRY2_BUTTON_PIN   =   A2,
                BATH_BUTTON_PIN     =   A5,
                // SCONCE_BUTTON_PIN = A2;
                // BACKWALL_BUTTON_PIN = A5;

                        DMX_PIN     =   3;

//light values lookup table, built on an S curve, to make LED dimming feel linear. 1024 stages.
//Storing in PROGMEM allows for such a large array (1023 values). Without PROGMEM, an array of 512 values pushed the limit of dynamic memory but now it's ~51%.
const uint8_t   WIDTH   = 32,
                HEIGHT  = 32;
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
const uint16_t BUTTON_RES[2] = {488, 968};     // [0]bottom button returns ~3.3v    // [1]top button returns ~5v
const uint8_t BUTTON_RESISTANCE_TOLERANCE = 200; // +/-
const uint8_t LOOP_DELAY_INTERVAL = 20; // refresh speed in ms          {for ms=1: factor=0.00001; amount=0.018}


const uint8_t MAX_PRESS_COUNT = 6;
const uint16_t BUTTON_RELEASE_TIMER = 250;      // time before release action is processed; time allowed between release and next press for multipresses
const uint8_t BUTTON_FADE_DELAY = 230;          // minimum time the button must be held to start "held" action;

const uint16_t BUTTON_FADE_DELAY_RAPID = 410;       //after this time, accellerate fading speed (double) so it doesn't take so long to fade to max/min

//brightness value = index of large lookup table above
const float DEFAULT_BRIGHTNESS = 0.40; // 0-1 (percent) value for default brightness when turned on
const float FADE_FACTOR = 0.0050;      //base fade adjustment; modified by section[].BRIGHTNESS_FACTOR
const float FADE_FACTOR_RAPID = 0.01;    //after BUTTON_FADE_DELAY_RAPID ms, this is applied instead

uint32_t loopStartTime = 0;             // loop start time
const float _MID = 0.878;
const float _HIGH = 0.995;
const float _LOW = 0.6;
//const float RED_LIST[] = {1., _MID, 0, 0, 0, _MID}; // colorProgress cycle layout
//const float GREEN_LIST[] = {0, _MID, 1., _MID, 0, 0};
//const float BLUE_LIST[] = {0, 0, 0, _MID, 1., _MID};
const float RED_LIST[] = {1., _HIGH, _MID, _LOW, 0, 0, 0, 0, 0, _LOW, _MID, _HIGH}; // colorProgress cycle layout
const float GREEN_LIST[] = {0, _LOW, _MID, _HIGH, 1., _HIGH, _MID, _LOW, 0, 0, 0, 0};
const float BLUE_LIST[] = {0, 0, 0, 0, 0, _LOW, _MID, _HIGH, 1., _HIGH, _MID, _LOW};
const uint8_t COLOR_PROGRESS_DELAY_COUNT = 0;   // n delay cycles per progress cycle
const uint8_t COLOR_PROGRESS_DELAY_SUDDEN = 0;
const float COLOR_PROGRESS_FADE_AMOUNT = 0.001; //modified if faded so interval is the same if faded. fade not yet implemented.

const uint8_t NUM_OF_MODES_CYCLE = 3;
const uint8_t LIGHTSECTION_COUNT = 4;

const uint16_t COLOR_PROGRESS_SMOOTH_DELAY_INIT = 20; //ms
const uint16_t COLOR_PROGRESS_SUDDEN_DELAY_INIT = 3000; //ms

uint16_t colorProgressDelayCounter = 0;
const uint8_t COLOR_PROGRESS_DELAY_COUNTER_INIT = 5;    // 5 * 20ms (main loop time) per adjustment

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
    // two per section:
    // Inside underloft
    {0, 0, 0, 0, false}, //entry button up         
    {0, 0, 0, 0, false}, //entry button down

    // Outside (Porch)
    {0, 0, 0, 0, false}, //entry2 button up
    {0, 0, 0, 0, false}, //entry2 button down

    // Kitchen underloft
    {0, 0, 0, 0, false}, //kitchen left wall
    {0, 0, 0, 0, false}, //kitchen

    // Bathroom (back wall right nook)
    {0, 0, 0, 0, false}, //bath
    {0, 0, 0, 0, false},

    //Back wall button
    // {0, 0, 0, 0, false}, //Back wall (left) / Greenhouse?
    // {0, 0, 0, 0, false},

    //sconce 1 button close button
};


struct section_t
{
    float RGBW[4];     // stores current RGBW color levels
    float lastRGBW[4]; // last color levels
    float masterBrightness;
    int8_t mode; // 0-4: WW, colors, colors+ww, (All, Nightlight)
    uint8_t lastMode;         // not used
    //typical cycle is 0-1-2-0... modes 3 and 4 are hidden from the typical cycle.
    bool isOn;               // Are any levels > 0?
    float BRIGHTNESS_FACTOR; // affects [default brightness + fade speed], pref range [0-1]
    uint8_t DMXout;          // DMX OUT number (set of 4 channels) this section controls
    // colorProgress variables for this section
    uint8_t colorDelayCounter; // slows the color cycle
    uint8_t colorState;        // next color state in the cycle
    bool colorProgress;        // while true, colors for this section will cycle

    uint32_t colorProgressTimerStart;   // starting time of colorProgress
    uint16_t colorProgressInterval;     // adjust this to change colorProgress speed

    float nextRGB[3]; // next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.
    float colorCycleFadeLevel;
    int8_t colorCycleFadeDir;

    uint8_t PIN;
    button_t *_button[2];

} section[] = {
    //  ID 0    Living Room Lights
    {{0., 0., 0., 0.}, {0., 0., 0., 0.}, 1., 0, 0, false, 0.8, 4, 0, 0, false, 0, 0, {0, 0, 0}, 1, 1, ENTRY_BUTTON_PIN, {&button[0], &button[1]}},

    //  ID 1    Kitchen Lights
    {{0., 0., 0., 0.}, {0., 0., 0., 0.}, 1., 0, 0, false, 1, 3, 0, 0, false, 0, 0, {0, 0, 0}, 1, 1, KITCHEN_BUTTON_PIN, {&button[4], &button[5]}},

    //  ID 2   Porch Lights
    {{0., 0., 0., 0.}, {0., 0., 0., 0.}, 1., 0, 0, false, 0.9, 2, 0, 0, false, 0, 0, {0, 0, 0}, 1, 1, ENTRY2_BUTTON_PIN, {&button[2], &button[3]}},

    //  ID 3   bath
    {{0., 0., 0., 0.}, {0., 0., 0., 0.}, 1., 0, 0, false, 1.0, 1, 0, 0, false, 0, 0, {0, 0, 0}, 1, 1, BATH_BUTTON_PIN, {&button[6], &button[7]}},

    //  ID     Overhead Bedroom
    //  ID     Overhead Main
    //  ID     Overhead Small Loft
    //  ID     Greenhouse Lights
};

//******************************************************************************************************************************************************** 

//                                          End Variables

//******************************************************************************************************************************************************** 


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
        {
            DmxSimple.write(i, 0);
        }
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

    for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) 
    {
        if (section[i].colorProgress == true) 
        {
            if ((currentTime - section[i].colorProgressTimerStart) >= section[i].colorProgressInterval) 
            {
                section[i].colorProgressTimerStart += section[i].colorProgressInterval;
                progressColor(i);
            }
        }
    }


    if ((currentTime - loopStartTime) >= LOOP_DELAY_INTERVAL) // 20ms loop
    {
        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) 
        {
            uint16_t buttonStatus = analogRead(section[i].PIN); // check if any buttons are pressed
            
            // REGISTER RELEASES:
            if (buttonStatus <= 256) // if no button is pressed:
            {
                for (uint8_t b = 0; b < 2; b++) 
                {
                    // check if button[i] was pressed on the last loop by checking for non-zero pressedTime
                    if (section[i]._button[b]->pressedTime > 0) 
                    {
                        // if so, start the releaseTimer
                        // and reset pressedTime so the next press is detected as a new press rather than a held press.
                        section[i]._button[b]->releaseTimer = currentTime + BUTTON_RELEASE_TIMER;
                        section[i]._button[b]->pressedTime = 0;
                    }
                    else 
                    // else if pressedTime == 0 then button[i] is already "RELEASED":
                    {
                        // wait for releaseTimer before commencing "release actions," in case user is attempting a double or triple press.
                        if (currentTime >= section[i]._button[b]->releaseTimer) 
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
                                    {
                                        section[i]._button[b]->pressedCount = MAX_PRESS_COUNT;
                                    }

                                    if (b == 1) //top button action
                                    {
                                        switch(section[i]._button[b]->pressedCount)
                                        {
                                            case (3):
                                                topAction3p(i, currentTime, b);
                                                break;
                                            case (2):
                                                topAction2p(i, currentTime);
                                                break;
                                            case (1):
                                                topAction1p(i);
                                                break;
                                        }
                                    }
                                    else // bottom button action
                                    {
                                        switch (section[i]._button[b]->pressedCount)
                                        {
                                            case (3):
                                                botAction3p(i, currentTime, b);
                                                break;
                                            case (2):
                                                botAction2p(i, currentTime);
                                                break;
                                            case (1):
                                                botAction1p(i);
                                                break;
                                        }
                                    }
                                    updateLights(i);
                                }
                            }
                            // after RELEASE ACTIONS, reset pressedCount counter to 0.
                            section[i]._button[b]->pressedCount = 0; 
                        }                                            // END RELEASE ACTIONS (button press)
                    }
                }

            }
            else // else buttonStatus > 255; a button is being pressed.  // PRESSED ACTIONS (register a press, and do "held button" actions)
            {

                //this is where the pressed actions reside:

                if (DEBUG == true) // {{ DEBUG }}
                {
                    Serial.print(F(" section:")); Serial.print(i); // (print button reading)
                    Serial.print(F(" pin:")); Serial.print(section[i].PIN); // (print button reading)
                    Serial.print(F(" ")); Serial.print(buttonStatus); // (print button reading)
                    Serial.print(F(" | "));
                }

                if (buttonStatus >= (BUTTON_RES[1] - BUTTON_RESISTANCE_TOLERANCE) && buttonStatus <= (BUTTON_RES[1] + BUTTON_RESISTANCE_TOLERANCE)) 
                {

                    heldTopButtonActions(i, currentTime);

                } // end top button
                else if (buttonStatus >= (BUTTON_RES[0] - BUTTON_RESISTANCE_TOLERANCE) && buttonStatus <= (BUTTON_RES[0] + BUTTON_RESISTANCE_TOLERANCE))
                {

                    heldBottomButtonActions(i, currentTime);
                    
                } 
                

            } // end {button held} thread
        }     // end {check each section} loop
    }         // update timer
} // void loop