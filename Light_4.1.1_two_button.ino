#include <avr/pgmspace.h>
#include <DmxSimple.h>
const String VERSION = "Light_4.1.1: Structs and Light ID\n - SERIAL Enabled; DMX Disabled";
const bool DEBUG = false;
const uint8_t DMX_PIN = 3;
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


const uint8_t LOW_CYCLE_STARTS_AT = 1;      // (index) white
const uint8_t NUM_OF_MODES_IN_CYCLE_LOW = 3; // white==1, rgbSmooth==2, rgbSudden==3
const uint8_t MAX_BRIGHTNESS_MODE = 3 + LOW_CYCLE_STARTS_AT; // max==4
const uint8_t HIGH_CYCLE_STARTS_AT = 11;     // (index) red (11, 12, 13, 14)
const uint8_t NUM_OF_MODES_IN_CYCLE_HIGH = 4;    // r, g, b, comb

// 12214 / 30720 bytes
// 1354 of 2048 (694) bytes
//
///uint16_t TESTING_SIZEOF_UINT16_T[12] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,}; 
//// 12256 / 30720 bytes    42 diff
//// 1378 of 2048 (670) bytes   24 diff
//
float TESTING_SIZEOF_FLOAT[12] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00,};
//// 12316 / 30720 bytes    102 diff
//// 1402 of 2048 (646) bytes   48 diff
//

const float DEFAULT_BRIGHTNESS = 0.60; // 0-1 (percent) value for default brightness when turned on
const uint8_t DEFAULT_BRIGHTNESS_NEW = 60; // 0-100 (percent) value for default brightness when turned on
const uint8_t COLOR_LOOP_SMOOTH_DELAY_INT = 20; //ms
const uint16_t COLOR_LOOP_SUDDEN_DELAY_INT = 3000; //ms
const float COLOR_LOOP_FADE_AMOUNT = 0.001;  // brightness adjust amount per tick
const uint8_t COLOR_LOOP_FADE_AMOUNT_NEW = 33;  // brightness adjust amount per tick

const float _HIGH = 0.95,
             _MID =  0.85,
             _LOW = 0.60;

const uint8_t _HIGH_NEW = 95,
             _MID_NEW =  85,
             _LOW_NEW = 60;
const uint8_t MAX_COLOR_STATE = 12;
const float RED_LIST[MAX_COLOR_STATE] =   {1., _HIGH,  _MID, _LOW,     0, 0,       0, 0,           0, _LOW,    _MID, _HIGH },
            GREEN_LIST[MAX_COLOR_STATE] = {0, _LOW,    _MID, _HIGH,    1., _HIGH,  _MID, _LOW,     0, 0,       0, 0        },
            BLUE_LIST[MAX_COLOR_STATE] =  {0, 0,       0, 0,           0, _LOW,    _MID, _HIGH,    1., _HIGH,  _MID, _LOW};
const uint8_t RED_LIST_NEW[MAX_COLOR_STATE] =   {100, _HIGH_NEW,  _MID_NEW, _LOW_NEW,     0, 0,       0, 0,           0, _LOW_NEW,    _MID_NEW, _HIGH_NEW },
            GREEN_LIST_NEW[MAX_COLOR_STATE] = {0, _LOW_NEW,    _MID_NEW, _HIGH_NEW,    100, _HIGH_NEW,  _MID_NEW, _LOW_NEW,     0, 0,       0, 0        },
            BLUE_LIST_NEW[MAX_COLOR_STATE] =  {0, 0,       0, 0,           0, _LOW_NEW,    _MID_NEW, _HIGH_NEW,    100, _HIGH_NEW,  _MID_NEW, _LOW_NEW};
// const uint8_t MAX_COLOR_STATE = 6;
// const float RED_LIST[] = {1., _MID, 0, 0, 0, _MID};
// const float GREEN_LIST[] = {0, _MID, 1., _MID, 0, 0};
// const float BLUE_LIST[] = {0, 0, 0, _MID, 1., _MID};






const float FADE_AMOUNT = 0.005;      // base fade adjustment; modified by section[].BRIGHTNESS_FACTOR
const uint8_t FADE_AMOUNT_NEW = 16;      // base fade adjustment; modified by section[].BRIGHTNESS_FACTOR
const uint8_t COLOR_LOOP_DELAY_CTR_INT = 5;    // 5 * 20ms (main loop time) per adjustment
uint16_t colorLoopDelayCtr = 0;



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
    
    uint8_t BRIGHTNESS_FACTOR_NEW; // affects [default brightness + fade speed], pref range [0-1]
    float masterLevel_NEW; // master brightness level
    float lastMasterLevel_NEW; // level from last time the light was on
    uint16_t RGBW_NEW[4];     // stores current RGBW color levels
    uint16_t lastRGBW_NEW[4]; // last color levels
    uint16_t nextRGB_NEW[3]; // next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.

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
        
        100, 100, 0,
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0, 0, 0}, 
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
        
        135, 100, 0,
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
        
        160, 100, 0,
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
        
        100, 100, 0,
        {0., 0., 0., 0.}, 
        {0., 0., 0., 0.}, 
        {0, 0, 0}, 
    },

    //  ID     Overhead Bedroom
    //  ID     Overhead Main
    //  ID     Overhead Small Loft
    //  ID     Greenhouse Lights
};



//light values lookup table, built on an S curve, to make LED dimming feel linear. 1024 stages.
//Storing in PROGMEM allows for such a large array (1023 values). Without PROGMEM, an array of 512 values pushed the limit of dynamic memory but now it's ~51%.
const uint8_t   WIDTH   = 128,
                HEIGHT  = 256;
const uint16_t TABLE_SIZE = 1023;
const uint8_t DIMMER_LOOKUP_TABLE[HEIGHT][WIDTH] PROGMEM = {
    //Adjusted to remove skipped numbers (added in missing numbers, then removed one from the end of every second line and adjusted accordingly)
{ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, },
{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, },

{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, },
{ 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, },
{ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, },
{ 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, },

{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, },
{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, },
{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, },
{ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, },

{ 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, },
{ 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, },
{ 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, },
{ 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, },

{ 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, },
{ 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 24, },
{ 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, },
{ 28, 28, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 34, },
 
{ 34, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 38, 38, 38, 38, 39, 39, 39, 39, 39, 40, 40, 40, },
{ 40, 40, 41, 41, 41, 41, 42, 42, 42, 42, 42, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 47, 47, 47, 47, 48, },
{ 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 52, 52, 52, 52, 53, 53, 53, 54, 54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, },
{ 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 65, 65, 65, 66, 66, 67, 67, 67, 68, },
 
{ 68, 68, 69, 69, 69, 70, 70, 71, 71, 71, 72, 72, 73, 73, 73, 74, 74, 75, 75, 75, 76, 76, 77, 77, 77, 78, 78, 79, 79, 80, 80, 80, },
{ 81, 81, 82, 82, 83, 83, 84, 84, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 91, 91, 92, 92, 93, 93, 94, 94, 95, 95, 96, },
{ 96, 97, 97, 98, 98, 99, 99, 100, 100, 101, 102, 102, 103, 103, 104, 104, 105, 105, 106, 107, 107, 108, 108, 109, 110, 110, 111, 111, 112, 113, 113, 114, },
{ 114, 115, 116, 116, 117, 118, 118, 119, 119, 120, 121, 121, 122, 123, 123, 124, 125, 125, 126, 127, 127, 128, 129, 130, 130, 131, 132, 132, 133, 134, 135, 135, },

{ 136, 137, 138, 138, 139, 140, 141, 141, 142, 143, 144, 144, 145, 146, 147, 148, 148, 149, 150, 151, 152, 152, 153, 154, 155, 156, 157, 157, 158, 159, 160, 161, },
{ 162, 163, 164, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, },
{ 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, },
{ 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, },
};


// uint8_t lookupTable(uint8_t ii, uint8_t ccolor) {
//     uint8_t temp = 0;

//     uint8_t height = (uint16_t(section[ii].RGBW[ccolor] * section[ii].masterLevel * TABLE_SIZE) / HEIGHT);
//     uint8_t width = (uint16_t(section[ii].RGBW[ccolor] * section[ii].masterLevel * TABLE_SIZE) % WIDTH);

//     // look up brighness from table and saves as temp ( sizeof(temp) resolves to 1 [byte of data])
//     memcpy_P(&temp, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(temp)); 

//     if ((section[ii].RGBW[ccolor] > 0) && (section[ii].masterLevel > 0) && (temp == 0)) 
//         temp = 1;
    
//     if (DEBUG) {
//         Serial.print(F("table_w:"));    Serial.print(width);
//         Serial.print(F(", table_h:"));  Serial.print(height);
//     }
//     return temp;
// }
