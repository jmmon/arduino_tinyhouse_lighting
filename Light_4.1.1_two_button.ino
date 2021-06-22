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

const uint8_t CHANNELS = 64;
const uint16_t BUTTON_RES[2] = {488, 968};     // [0]bottom button returns ~3.3v    // [1]top button returns ~5v
const uint8_t BUTTON_RESISTANCE_TOLERANCE = 200; // +/-
const uint8_t LOOP_DELAY_INTERVAL = 20; // refresh speed in ms          {for ms=1: factor=0.00001; amount=0.018}

const uint8_t MAX_PRESS_COUNT = 6;
const uint16_t BUTTON_RELEASE_TIMER = 250;      // time allowed between release and next press for multipresses
const uint8_t BUTTON_FADE_DELAY = 230;          // minimum time the button must be held to start "held" action;

const uint16_t BUTTON_FADE_DELAY_RAPID = 410;       // after this time, double fading speed so it adjusts quicker

const float DEFAULT_BRIGHTNESS = 0.40; // 0-1 (percent) value for default brightness when turned on
const float FADE_FACTOR = 0.0050;      // base fade adjustment; modified by section[].BRIGHTNESS_FACTOR
const float FADE_FACTOR_RAPID = 0.01;    // after BUTTON_FADE_DELAY_RAPID ms, this is applied instead

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

const uint8_t COLOR_PROGRESS_DELAY_COUNTER_INIT = 5;    // 5 * 20ms (main loop time) per adjustment

uint16_t colorProgressDelayCounter = 0;
uint32_t loopStartTime = 0;

//*************************************************************************************************************************************

//                                              Structs

//*************************************************************************************************************************************

struct button_t
{
    uint32_t releaseTimer; //when was this button released?
    uint32_t pressedTime;  //when was this button pressed?
    uint16_t pressedDuration;  // time from press to release
    uint8_t pressedCount;  //If button is pressed before releaseTimer ends, add one to count
    bool beingHeld;        //is the button being held? (for longer than BUTTON_FADE_DELAY

} button[] = {
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
        // typical cycle is 0-1-2-0... modes 3 and 4 are hidden from the typical cycle.

    bool isOn;               // Are any levels > 0?
    float BRIGHTNESS_FACTOR; // affects [default brightness + fade speed], pref range [0-1]
    uint8_t DMXout;          // DMX OUT number (set of 4 channels) for this section

    float colorDelayCounter; // slows the color cycle, used to slow the "sudden" mode
    uint8_t colorState;        // next color state in the cycle
    bool colorProgress;        // while true, colors for this section will cycle

    uint32_t colorProgressTimerStart;   // starting time of colorProgress
    uint16_t colorProgressInterval;     // adjust this to change colorProgress speed

    float nextRGB[3]; // next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.
    float colorCycleFadeLevel;  //not used?
    int8_t colorCycleFadeDir;   //not used?

    uint8_t PIN;
    button_t *_button[2];

} section[] = {
    //  ID 0    Living Room Lights
    {
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        1., 0, 
        false, 0.8, 4, 
        0., 0, false, 
        0, 0, 
        {0., 0., 0.}, 
        1, 1, 
        ENTRY_BUTTON_PIN, 
        {&button[0], &button[1]}
    },

    //  ID 1    Kitchen Lights
    {
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        1., 0, 
        false, 1, 3, 
        0., 0, false, 
        0, 0, 
        {0, 0, 0}, 
        1, 1, 
        KITCHEN_BUTTON_PIN, 
        {&button[4], &button[5]}
    },

    //  ID 2   Porch Lights
    {
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        1., 0, 
        false, 0.9, 2, 
        0., 0, false, 
        0, 0, 
        {0, 0, 0}, 
        1, 1, 
        ENTRY2_BUTTON_PIN, 
        {&button[2], &button[3]}
    },

    //  ID 3   bath
    {
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        1., 0, 
        false, 1.0, 1, 
        0., 0, false, 
        0, 0, 
        {0, 0, 0}, 
        1, 1, 
        BATH_BUTTON_PIN, 
        {&button[6], &button[7]}
    },

    //  ID     Overhead Bedroom
    //  ID     Overhead Main
    //  ID     Overhead Small Loft
    //  ID     Greenhouse Lights
};


void setup()
{
    for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++)
    {
        pinMode(section[i].PIN, INPUT);
    }
    randomSeed(analogRead(0)); //get random seed; used to start colorProgress state at a random color
    loopStartTime = millis();

    if (DEBUG == true)
    {
        Serial.begin(9600); Serial.println(VERSION);
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
}


void loop()
{
    uint32_t currentTime = millis();
    for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) 
    {
        if (section[i].colorProgress == true) 
        {
            if ((currentTime - section[i].colorProgressTimerStart) >= section[i].colorProgressInterval) 
            {
                section[i].colorProgressTimerStart += section[i].colorProgressInterval;
                if (section[i].mode == 2) // sudden color changes
                {
                    progressColorSudden(i);
                }
                else if (section[i].mode == 1) 
                {
                    progressColorSmooth(i);
                }
            }
        }
    }

    if ((currentTime - loopStartTime) >= LOOP_DELAY_INTERVAL) // 20ms loop
    {
        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) 
        {
            uint16_t buttonStatus = analogRead(section[i].PIN);
            if (buttonStatus <= 256) // if no button is pressed check for Releases:
            {
                for (uint8_t b = 0; b < 2; b++) 
                {
                    // check if button[i] was just released
                    if (section[i]._button[b]->pressedTime > 0) 
                    {
                        section[i]._button[b]->pressedTime = 0; // so the next press is detected as a new press rather than a held press
                        section[i]._button[b]->releaseTimer = currentTime + BUTTON_RELEASE_TIMER; // start the releaseTimer
                    }
                    else if (currentTime >= section[i]._button[b]->releaseTimer)
                    // in case user is attempting a double or triple press wait for releaseTimer before commencing "release actions"
                    {
                        if (section[i]._button[b]->beingHeld == true) 
                        {
                            //mark as not held in case it was
                            section[i]._button[b]->beingHeld = false;
                        }
                        else    //if button is not held commence Release action
                        {
                            if (section[i]._button[b]->pressedCount > 0)
                            {
                                if (section[i]._button[b]->pressedCount > MAX_PRESS_COUNT)
                                {
                                    section[i]._button[b]->pressedCount = MAX_PRESS_COUNT;
                                }

                                switch (b)
                                {
                                    case (1): // top button action
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
                                        break;

                                    case (0): // bottom button action
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
                                        break;
                                }
                                updateLights(i);
                            }
                        }
                        // after RELEASE ACTIONS:
                        section[i]._button[b]->pressedCount = 0; 
                    } // release timer
                } // for each button
            }
            else // else buttonStatus > 255: register press / do "held button" actions
            {
                if (DEBUG == true)
                {
                    heldActionsDEBUG(i, buttonStatus);
                }

                uint8_t b = 1;
                if (buttonStatus >= (BUTTON_RES[b] - BUTTON_RESISTANCE_TOLERANCE) && 
                    buttonStatus <= (BUTTON_RES[b] + BUTTON_RESISTANCE_TOLERANCE)) 
                {
                    
                    if (DEBUG == true)
                    {
                        btnTopHeldActionsDEBUG(i, b);
                    }

                    // if NEW button press
                    if (section[i]._button[b]->pressedTime == 0) 
                    {
                        btnRegisterPress(i, b, currentTime);
                    }
                    else if (currentTime >= (section[i]._button[b]->pressedTime + BUTTON_FADE_DELAY)) // if button has been pressed and held past BUTTON_FADE_DELAY, button is being held:
                    {
                        btnTopHeld(i, b, currentTime);
                    }
                }
                else 
                {
                    b = 0;
                    if (buttonStatus >= (BUTTON_RES[b] - BUTTON_RESISTANCE_TOLERANCE) && 
                        buttonStatus <= (BUTTON_RES[b] + BUTTON_RESISTANCE_TOLERANCE))
                    {
                        
                        if (DEBUG == true)
                        {
                            btnBotHeldActionsDEBUG(i, b);
                        }

                        // if NEW button press
                        if (section[i]._button[b]->pressedTime == 0) 
                        {
                            btnRegisterPress(i, b, currentTime);
                        }
                        else if (currentTime >= (section[i]._button[b]->pressedTime + BUTTON_FADE_DELAY)) // if button has been pressed and held past BUTTON_FADE_DELAY, button is being held:
                        {
                            btnBotHeld(i, b, currentTime);
                        }
                    }
                }
            }
        } // end {check each section} loop
    } // timer
} // void loop