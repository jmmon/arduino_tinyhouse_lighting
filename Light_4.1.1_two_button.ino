#include <avr/pgmspace.h>
#include <DmxSimple.h>
const String VERSION = "Light_4.1.1: Structs and Light ID\n - SERIAL Enabled; DMX Disabled";
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

uint32_t currentTime = 0;


struct btn_t {

    uint32_t releaseTime; //when was this button released?
    uint32_t pressedTime;  //when was this button pressed?
    uint8_t pressedCount;  //If button is pressed before releaseTime ends, add one to count
    bool beingHeld;        //is the button being held? (for longer than BUTTON_FADE_DELAY

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

    float RGBW[4];     // stores current RGBW color levels

    float lastRGBW[4]; // last color levels

    float masterBrightness;
    int8_t mode; // 0-4: WW, colors, colors+ww, (All, Nightlight)
        // typical cycle is 0-1-2-0... modes 3 and 4 are hidden from the typical cycle.

    bool isOn;               // Are any levels > 0?
    float BRIGHTNESS_FACTOR; // affects [default brightness + fade speed], pref range [0-1]
    uint8_t DMX_OUT;          // DMX OUT number (set of 4 channels) for this section

    uint32_t colorProgressTimerStart;   // starting time of colorProgress
    uint16_t colorProgressInterval;     // adjust this to change colorProgress speed
    float colorDelayCounter; // slows the color cycle, used to slow the "sudden" mode

    uint8_t colorState;        // next color state in the cycle
    bool colorProgress;        // while true, colors for this section will cycle

    float nextRGB[3]; // next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.

    uint8_t PIN;
    btn_t *_btn[2];

} section[] = {
    //  ID 0    Living Room Lights
    {
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        1., 0, 
        false, 0.8, 4, 
        0, 0, 0., 
        0, false, 
        {0., 0., 0.}, 
        ENTRY_BUTTON_PIN, 
        {&btn[0], &btn[1]}
    },

    //  ID 1    Kitchen Lights
    {
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        1., 0, 
        false, 1, 3, 
        0, 0, 0., 
        0, false, 
        {0, 0, 0}, 
        KITCHEN_BUTTON_PIN, 
        {&btn[4], &btn[5]}
    },

    //  ID 2   Porch Lights
    {
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        1., 0, 
        false, 0.9, 2, 
        0, 0, 0., 
        0, false, 
        {0, 0, 0}, 
        ENTRY2_BUTTON_PIN, 
        {&btn[2], &btn[3]}
    },

    //  ID 3   bath
    {
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        1., 0, 
        false, 1.0, 1, 
        0, 0, 0., 
        0, false, 
        {0, 0, 0}, 
        BATH_BUTTON_PIN, 
        {&btn[6], &btn[7]}
    },

    //  ID     Overhead Bedroom
    //  ID     Overhead Main
    //  ID     Overhead Small Loft
    //  ID     Greenhouse Lights
};


void setup() {
    for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) 
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

    for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) {
        
        if (section[i].colorProgress == true) {

            if ((currentTime - section[i].colorProgressTimerStart) >= section[i].colorProgressInterval) {

                section[i].colorProgressTimerStart += section[i].colorProgressInterval;

                if (section[i].mode == 2) 
                    progressColorSudden(i);

                else if (section[i].mode == 1) 
                    progressColorSmooth(i);
            }
        }
    }

    if ((currentTime - loopStartTime) >= LOOP_DELAY_INTERVAL) {

        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < LIGHTSECTION_COUNT; i++) {

            uint16_t btnStatus = analogRead(section[i].PIN);

            for (uint8_t b = 0; b < 2; b++) {

                if (btnStatus <= 255) {      // if no button is pressed:

                    if (section[i]._btn[b]->pressedTime > 0) 
                        btnRegisterNewRelease(i, b);

                    
                    else if ((section[i]._btn[b]->releaseTime != 0) && (currentTime >= (section[i]._btn[b]->releaseTime + BUTTON_RELEASE_TIMER))) 
                        // after small wait, commmence actions for the pressed button
                        btnPressedActions(i, b);
                    
                } else if ((btnStatus >= (BUTTON_RES[b] - BUTTON_RESISTANCE_TOLERANCE)) && (btnStatus <= (BUTTON_RES[b] + BUTTON_RESISTANCE_TOLERANCE))) {
                    // else btnStatus > 255: register press and/or do "held button" actions

                    if (DEBUG)
                        heldActionsDEBUG(i, b, btnStatus);

                    if (section[i]._btn[b]->pressedTime == 0) 
                        btnRegisterNewPress(i, b);

                    else if (currentTime >= (section[i]._btn[b]->pressedTime + BUTTON_FADE_DELAY)) {

                        if (b == 0) 
                            btnBotHeld(i, b);

                        else 
                            btnTopHeld(i, b);
                    }
                }
            } // button loop
        } // section loop
    } // timer loop
} // void loop
