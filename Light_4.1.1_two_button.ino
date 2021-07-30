#include <avr/pgmspace.h>
#include <DmxSimple.h>

const String VERSION = "Light_4.1.1: Classes";
const bool DEBUG = false;

const uint8_t DMX_PIN  =   3;
const uint8_t CHANNELS = 64;
const uint8_t SECTION_COUNT = 4;



// Buttons:

const uint16_t BTN_RESIST[2] = {488, 968};     // [0]bottom button returns ~3.3v  [1] top button returns ~5v
const uint8_t BTN_RESIST_TOLERANCE = 200; // BTN_RESIST +/- BTN_RESIST_TOLERANCE
const uint8_t MAX_PRESS_COUNT = 3;              // single, double, triple press

const uint8_t   KITCHEN_BTN_PIN  =   A3,
                ENTRY_BTN_PIN    =   A4,
                ENTRY2_BTN_PIN   =   A2,
                BATH_BTN_PIN     =   A5;
                //const uint8_t SCONCE_BUTTON_PIN = A2;
                //const uint8_t BACKWALL_BUTTON_PIN = A5;
                


// Timer stuff:

uint32_t loopStartTime = 0;
uint32_t currentTime = 0;
const uint8_t LOOP_DELAY_INTERVAL = 20;     // refresh speed in ms

const uint8_t BTN_HELD_DELAY = 230;         // time held before "held actions"
const uint16_t BTN_RELEASE_TIMER = 250;     // wait time before actions, to detect multi-press



// Modes Setup:

const uint8_t LOW_CYCLE_STARTS_AT = 1;      // (index) white
const uint8_t NUM_OF_MODES_IN_CYCLE_LOW = 3; // white==1, rgbSmooth==2, rgbSudden==3
const uint8_t MAX_BRIGHTNESS_MODE = 3 + LOW_CYCLE_STARTS_AT; // max==4

const uint8_t HIGH_CYCLE_STARTS_AT = 8;     // (index) red
const uint8_t NUM_OF_MODES_IN_CYCLE_HIGH = 4;  // r, g, b, combined
const uint8_t SINGLE_COLOR_MODE_OFFSET = HIGH_CYCLE_STARTS_AT; //num req to get to 0 from red mode number



// Brightness:

const float DEFAULT_BRIGHTNESS = 0.60; // 0-1 (percent) value for default brightness when turned on
const float FADE_AMOUNT = 0.005;      // base fade adjustment; modified by section[].BRIGHTNESS_FACTOR

const float COLOR_LOOP_FADE_AMOUNT = 0.001;  // brightness adjust amount per tick



// ColorProgress stuff:

const uint8_t COLOR_LOOP_SMOOTH_DELAY_INT = 20; //ms
const uint16_t COLOR_LOOP_SUDDEN_DELAY_INT = 3000; //ms
const uint8_t COLOR_LOOP_DELAY_CTR_INT = 5;    // 5 * 20ms (main loop time) per adjustment
uint16_t colorLoopDelayCtr = 0;

const uint8_t MAX_COLOR_STATE = 12;

const float _HIGH = 0.95,
             _MID =  0.85,
             _LOW = 0.60;
// const float RED_LIST[MAX_COLOR_STATE] =   {
//                 1., _HIGH,  _MID, _LOW,     
//                 0, 0,       0, 0,           
//                 0, _LOW,    _MID, _HIGH 
//             },
//             GREEN_LIST[MAX_COLOR_STATE] = {
//                 0, _LOW,    _MID, _HIGH,    
//                 1., _HIGH,  _MID, _LOW,     
//                 0, 0,       0, 0        
//             },
//             BLUE_LIST[MAX_COLOR_STATE] =  {
//                 0, 0,       0, 0,           
//                 0, _LOW,    _MID, _HIGH,    
//                 1., _HIGH,  _MID, _LOW
//             };
// const uint8_t MAX_COLOR_STATE = 6;
// const float RED_LIST[] = {1., _MID, 0, 0, 0, _MID};
// const float GREEN_LIST[] = {0, _MID, 1., _MID, 0, 0};
// const float BLUE_LIST[] = {0, 0, 0, _MID, 1., _MID};

//Sketch uses 13838 bytes (45%) of program storage space. Maximum is 30720 bytes.
//Global variables use 1172 bytes (57%) of dynamic memory, leaving 876 bytes for local variables. Maximum is 2048 bytes.
//144 saved from dynamic; 166 used on progmem
//Sketch uses 14004 bytes (45%) of program storage space. Maximum is 30720 bytes.
//Global variables use 1316 bytes (64%) of dynamic memory, leaving 732 bytes for local variables. Maximum is 2048 bytes.

// use progmem for lists

const float COLOR_PROGRESS_LIST[3][MAX_COLOR_STATE] PROGMEM = {
    //red
    {
        1., _HIGH,  _MID, _LOW,     
        0, 0,       0, 0,           
        0, _LOW,    _MID, _HIGH 
    },
    //green
    {
        0, _LOW,    _MID, _HIGH,    
        1., _HIGH,  _MID, _LOW,     
        0, 0,       0, 0        
    },
    //blue
    {
        0, 0,       0, 0,           
        0, _LOW,    _MID, _HIGH,    
        1., _HIGH,  _MID, _LOW
    }
};









// light values lookup table, built on an S curve, to make LED dimming feel linear. 1024 stages.
// Storing in PROGMEM allows for such a large array (1023 values). Without PROGMEM, an array of 512 values pushed the limit of dynamic memory but now it's ~51%.
    // Adjusted to remove skipped numbers (added in missing numbers, then removed one from the end of every second line and adjusted accordingly)
const uint8_t   WIDTH   = 32,
                HEIGHT  = 32;
const uint16_t TABLE_SIZE = 1023;
const uint8_t DIMMER_LOOKUP_TABLE[HEIGHT][WIDTH] PROGMEM = {

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
