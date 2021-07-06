#include <avr/pgmspace.h>
#include <DmxSimple.h>
const String VERSION = "Light_4.1.1: Structs and Light ID\n - SERIAL Enabled; DMX Disabled";
const bool DEBUG = false;
const uint8_t DMX_PIN  =   3;
const uint8_t CHANNELS = 64;

const uint16_t BTN_RESIST[2] = {488, 968};     // [0]bottom button returns ~3.3v  [1] top button returns ~5v
const uint8_t BTN_RESIST_TOLERANCE = 200; // BTN_RESIST +/- BTN_RESIST_TOLERANCE
const uint8_t SECTION_COUNT = 4;
const uint8_t   KITCHEN_BTN_PIN  =   A3,
                ENTRY_BTN_PIN    =   A4,
                ENTRY2_BTN_PIN   =   A2,
                BATH_BTN_PIN     =   A5;
                //const uint8_t SCONCE_BUTTON_PIN = A2;
                //const uint8_t BACKWALL_BUTTON_PIN = A5;

const uint8_t LOOP_DELAY_INTERVAL = 20;     // refresh speed in ms   {for ms=1: factor=0.00001; amount=0.018}
const uint8_t BTN_FADE_DELAY = 230;          // minimum time the button must be held to start "held" action;
const uint16_t BTN_RELEASE_TIMER = 250;      // time allowed between release and next press for multipresses
uint32_t loopStartTime = 0;
uint32_t currentTime = 0;


struct btn_t {
    uint32_t timeReleased; //when was this button released?
    uint32_t timePressed;  //when was this button pressed?
    uint8_t pressCt;  //If button is pressed before timeReleased ends, add one to count
    bool isHeld;        //is the button being held? (for longer than BTN_FADE_DELAY

} btn[] = {
    // Inside underloft
    {0, 0, 0, false}, //entry button up         
    {0, 0, 0, false}, //entry button down

    // Outside (Porch)
    {0, 0, 0, false}, //entry2 button up
    {0, 0, 0, false}, //entry2 button down

    // Kitchen underloft
    {0, 0, 0, false}, //kitchen left wall
    {0, 0, 0, false}, //kitchen

    // Bathroom (back wall right nook)
    {0, 0, 0, false}, //bath
    {0, 0, 0, false},

    //Back wall button
    // {0, 0, 0, 0, false}, //Back wall (left) / Greenhouse?
    // {0, 0, 0, 0, false},

    //sconce 1 button close button
};

struct section_t {
    btn_t *_btn[2];
    uint8_t PIN;
    uint8_t DMX_OUT;          // DMX OUT number (set of 4 channels) for this section
    float BRIGHTNESS_FACTOR; // affects [default brightness + fade speed], pref range [0-1]

    bool isOn;              // Are any levels > 0?
    bool colorProgress;     // while true, colors for this section will cycle
    bool extendedFade;      // enable extended fade

    float masterLevel; // master brightness level
    float lastMasterLevel; // level from last time the light was on
    int8_t mode; // 0-4: WW, colors, colors+ww, (All, Nightlight)
        // typical cycle is 0-1-2-0... modes 3 and 4 are hidden from the typical cycle.

    uint32_t colorStartTime;   // starting time of colorProgress
    uint16_t colorDelayInt;     // adjust this to change colorProgress speed
    float colorDelayCtr; // slows the color cycle, used to slow the "sudden" mode
    uint8_t colorState;        // next color state in the cycle

    bool RGBWon[4];     // is on? each color
    float RGBW[4];     // stores current RGBW color levels
    float lastRGBW[4]; // last color levels
    float nextRGB[3]; // next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.

} section[] = {
    //  ID 0    Living Room Lights
    {
        {&btn[0], &btn[1]},
        ENTRY_BTN_PIN, 4, 1.0, 
        false, false, false,
        1., 0., 0,
        0, 0, 0., 0, 
        {false, false, false, false},
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0., 0., 0.}, 
    },

    //  ID 1    Kitchen Lights
    {
        {&btn[4], &btn[5]},
        KITCHEN_BTN_PIN, 3, 1.35, 
        false, false, false,
        1., 0., 0,
        0, 0, 0., 0, 
        {false, false, false, false},
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0, 0, 0}, 
    },

    //  ID 2   Porch Lights
    {
        {&btn[2], &btn[3]},
        ENTRY2_BTN_PIN, 2, 1.6, 
        false, false, false,
        1., 0., 0, 
        0, 0, 0., 0, 
        {false, false, false, false},
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0, 0, 0}, 
    },

    //  ID 3   bath
    {
        {&btn[6], &btn[7]},
        BATH_BTN_PIN, 1, 1.0, 
        false, false, false,
        1., 0., 0,
        0, 0, 0., 0, 
        {false, false, false, false},
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0, 0, 0}, 
    },

    //  ID     Overhead Bedroom
    //  ID     Overhead Main
    //  ID     Overhead Small Loft
    //  ID     Greenhouse Lights
};


void setup() {
    for (uint8_t i = 0; i < SECTION_COUNT; i++) 
        pinMode(section[i].PIN, INPUT);
    
    randomSeed(analogRead(0)); //get random seed; used to start colorProgress state at a random color
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

            if (currentTime >= (section[i].colorStartTime + section[i].colorDelayInt)) {

                section[i].colorStartTime += section[i].colorDelayInt;

                if (section[i].mode == 2) 
                    progressColorSudden(i);

                else if (section[i].mode == 1) 
                    progressColorSmooth(i);
            }
        }
    }

    if (currentTime >= (loopStartTime + LOOP_DELAY_INTERVAL)) {

        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < SECTION_COUNT; i++) {

            uint16_t btnStatus = analogRead(section[i].PIN);

            for (uint8_t b = 0; b < 2; b++) {       // 2 buttons, bottom and top (0 and 1)s

                if (btnStatus <= 255) {      // if no button is pressed:

                    if (section[i]._btn[b]->timePressed > 0) {
                        // "register" a release of a button
                        section[i]._btn[b]->timePressed = 0; // reset
                        section[i]._btn[b]->timeReleased = currentTime; // save the time
                    }

                    else if ((section[i]._btn[b]->timeReleased != 0) && (currentTime >= (section[i]._btn[b]->timeReleased + BTN_RELEASE_TIMER))) 
                        btnActions(i, b); // after small wait
                    
                } else      // else btnStatus > 255: register press and/or do "held button" actions

                if ((btnStatus >= (BTN_RESIST[b] - BTN_RESIST_TOLERANCE)) && (btnStatus <= (BTN_RESIST[b] + BTN_RESIST_TOLERANCE))) {

                    if (DEBUG) {
                        DEBUG_heldActions(i, b, btnStatus);
                    }
                    
                    if (section[i]._btn[b]->timePressed == 0) {
                        // "register" a press of a button
                        section[i]._btn[b]->pressCt++;      // count the press
                        section[i]._btn[b]->timePressed = currentTime; // save the time
                    }

                    else if (currentTime >= (section[i]._btn[b]->timePressed + BTN_FADE_DELAY)) 
                        btnHeldActions(i, b);

                }
            } // button loop
        } // section loop
    } // timer loop
} // void loop
