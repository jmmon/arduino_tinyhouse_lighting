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

const uint8_t LOOP_DELAY_INTERVAL = 20;     // refresh speed in ms
const uint8_t BTN_FADE_DELAY = 230;         // time held before "held actions"
const uint16_t BTN_RELEASE_TIMER = 250;     // wait time before actions, to detect multi-press
uint32_t loopStartTime = 0;
uint32_t currentTime = 0;

const uint8_t MAX_PRESS_COUNT = 3;              // single, double, triple press

/**
 * @brief 
 * const uint8_t MODE_OFF = 0;
 * 
 * 
 */
const uint8_t LOW_CYCLE_STARTS_AT = 1;      // (index) white
const uint8_t NUM_OF_MODES_IN_CYCLE_LOW = 3; // white==1, rgbSmooth==2, rgbSudden==3
const uint8_t MAX_BRIGHTNESS_MODE = 3 + LOW_CYCLE_STARTS_AT; // max==4
const uint8_t HIGH_CYCLE_STARTS_AT = 252;     // (index) red
const uint8_t NUM_OF_MODES_IN_CYCLE_HIGH = 4;    // red==252, green==253, blue==254, TODO: combined==255;

const uint8_t SINGLE_COLOR_MODE_OFFSET = HIGH_CYCLE_STARTS_AT; //num req to get to 0 from red mode number

/*
 * BTN CLASS DEFINITION
 */
class Btn_C {
    //private:
    public:
        uint32_t timeReleased = 0;
        uint32_t timePressed = 0;

        uint8_t pressCt = 0;
        bool isHeld = false;

        //no constructor
        //functions/methods
        void registerPress() {
            pressCt++;  // add a press
            timePressed = currentTime; // save the time
        }

        void registerRelease() {
            timePressed = 0; // reset depressed timer
            timeReleased = currentTime; // save the time
        }

};

Btn_C btn[] = {
    Btn_C(), //entry button up  
    Btn_C(), //entry button down

    Btn_C(), //entry2 button up
    Btn_C(), //entry2 button down

    Btn_C(), //kitchen 
    Btn_C(),

    Btn_C(), //bath
    Btn_C(),
};

// struct btn_t {
//     uint32_t timeReleased; //when was this button released?
//     uint32_t timePressed;  //when was this button pressed?
//     uint8_t pressCt;  //If button is pressed before timeReleased ends, add one to count
//     bool isHeld;        //is the button being held? (for longer than BTN_FADE_DELAY

// } btn[] = {
//     // Inside underloft
//     {0, 0, 0, false}, //entry button up         
//     {0, 0, 0, false}, //entry button down

//     // Outside (Porch)
//     {0, 0, 0, false}, //entry2 button up
//     {0, 0, 0, false}, //entry2 button down

//     // Kitchen underloft
//     {0, 0, 0, false}, //kitchen left wall
//     {0, 0, 0, false}, //kitchen

//     // Bathroom (back wall right nook)
//     {0, 0, 0, false}, //bath
//     {0, 0, 0, false},

//     //Back wall button
//     // {0, 0, 0, 0, false}, //Back wall (left) / Greenhouse?
//     // {0, 0, 0, 0, false},

//     //sconce 1 button close button
// };




struct section_t {
    Btn_C *_btn[2];
    uint8_t PIN;
    uint8_t DMX_OUT;          // DMX OUT number (set of 4 channels) for this section
    float BRIGHTNESS_FACTOR; // affects [default brightness + fade speed], pref range [0-1]

    bool isOn;              // Are any levels > 0?
    bool colorProgress;     // while true, colors for this section will cycle
    bool extendedFade;      // enable extended fade

    float masterLevel; // master brightness level
    float lastMasterLevel; // level from last time the light was on
    uint8_t mode; // 0-4: WW, colors, colors+ww, (All, Nightlight)
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