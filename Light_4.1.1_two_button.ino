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
class Btn {
    private:
        uint8_t pinBtn;

        uint32_t timeReleased = 0;
        uint32_t timePressed = 0;

        uint8_t pressCt = 0;

        bool isHeld = false;

    public:
        //constructor
        Btn(uint8_t pinBtn) {
            this->pinBtn = pinBtn;

            pinMode(pinBtn, INPUT);
        }

        //functions/methods
        void registerPress(uint32_t t) { // takes the currentTime
            pressCt++;  // add a press
            timePressed = t; // save the time
            // section[i]._btn[b]->pressCt ++;
            // section[i]._btn[b]->timePressed = currentTime;
        }

        void registerRelease(uint32_t t) { // takes the currentTime
            timePressed = 0; // reset
            timeReleased = t; // save the time
            // section[i]._btn[b]->timePressed = 0;
            // section[i]._btn[b]->timeReleased = currentTime;
        }

};

Btn btnC[] = {
    Btn(ENTRY_BTN_PIN),
    Btn(ENTRY_BTN_PIN),

    Btn(KITCHEN_BTN_PIN),
    Btn(KITCHEN_BTN_PIN),

    Btn(ENTRY2_BTN_PIN),
    Btn(ENTRY2_BTN_PIN),

    Btn(BATH_BTN_PIN),
    Btn(BATH_BTN_PIN),
};

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


/*
 * SECTION CLASS DEFINITION
 */

class Section {
    private:
        //Btn *_btnC[2];
        uint8_t DMX_OUT;
        float BRIGHTNESS_FACTOR;

        bool isOn;
        bool colorProgress;
        bool extendedFade;

        float masterLevel;
        float lastMasterLevel;
        uint8_t mode;

        uint32_t colorStartTime;
        uint16_t colorDelayInt;
        float colorDelayCtr;
        uint8_t colorState;

        bool RGBwon[4];
        float RGBW[4];
        float lastRGBW[4];
        float nextRGB[3];

    public:
        //public vars, constructor, methods
        Section(uint8_t DMX_OUT) {
            this->DMX_OUT = DMX_OUT;

        }
        //methods for lights:
        //fade up/down
        //switch mode
        //updatelights
        //colorProgress
        //
        void updateLights(uint8_t i) { //might not need to take i but will for now
            if (DEBUG) {
                DEBUG_updateLights(i);

            } else {
                uint8_t brightnessValue = 0; // index for brightness lookup table

                for (uint8_t color = 0; color < 4; color++) {
                    if (RGBWon[color]) {

                        //brightnessValue = lookupTable(i, color);
                        
                        uint8_t height = (uint16_t(RGBW[color] * masterLevel * TABLE_SIZE) / HEIGHT);
                        uint8_t width = (uint16_t(RGBW[color] * masterLevel * TABLE_SIZE) % WIDTH);

                        // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
                        memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

                        if ((RGBW[color] > 0) && (masterLevel > 0) && (brightnessValue == 0)) 
                            brightnessValue = 1;
                        
                    } else {
                        //brightnessValue = 0;
                        // light is off, so turn it off
                    }

                    DmxSimple.write((DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
                    lastRGBW[color] = RGBW[color];
                }
            }
                
            if ( (RGBW[0] <= 0) && (RGBW[1] <= 0) && (RGBW[2] <= 0) && (RGBW[3] <= 0) ) {
                //switch to mode 0?

                isOn = false;
                masterLevel = 0;

                for (uint8_t color = 0; color < 4; color++) {  //clear rgbw
                    RGBW[color] = 0;
                    RGBWon[color] = false;
                }

                if (DEBUG) {
                    DEBUG_updateLightsOff(i);
                }
            }
        }
};

Section sectionC[] = {
    Section(4),
    Section(3),
    Section(2),
    Section(1),
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
    
    randomSeed(analogRead(0)); // used to start colorProgress state at a random color
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
            if (currentTime >= (section[i].colorStartTime + section[i].colorDelayInt)) { //loop is separate for this, so speed change works well
                section[i].colorStartTime += section[i].colorDelayInt;
                if (section[i].mode == (LOW_CYCLE_STARTS_AT + 2)) 
                    progressColorSudden(i);
                else if (section[i].mode == (LOW_CYCLE_STARTS_AT + 1)) 
                    progressColorSmooth(i);
            }
        }
    }

    if (currentTime >= (loopStartTime + LOOP_DELAY_INTERVAL)) {
        loopStartTime += LOOP_DELAY_INTERVAL; // set time for next timer

        for (uint8_t i = 0; i < SECTION_COUNT; i++) {
            uint16_t btnStatus = analogRead(section[i].PIN);

            for (uint8_t b = 0; b < 2; b++) {       // 2 buttons, bottom and top (0 and 1)s

                if (btnStatus <= 255) { // if no button is pressed: (after press)
                    if (section[i]._btn[b]->timePressed > 0) {
                        // "register" a release of a button
                        section[i]._btn[b]->timePressed = 0; // reset
                        section[i]._btn[b]->timeReleased = currentTime; // save the time
                    }
                    else if ((section[i]._btn[b]->timeReleased != 0) && (currentTime >= (section[i]._btn[b]->timeReleased + BTN_RELEASE_TIMER))) 
                        btnRelease(i, b); // after small wait
                    
                } else if ((btnStatus >= (BTN_RESIST[b] - BTN_RESIST_TOLERANCE)) && (btnStatus <= (BTN_RESIST[b] + BTN_RESIST_TOLERANCE))) { // else btnStatus > 255: register press and/or do "held button" actions

                    if (DEBUG) {
                        DEBUG_heldActions(i, b, btnStatus);
                    }
                    if (section[i]._btn[b]->timePressed == 0) {
                        // "register" a press of a button
                        section[i]._btn[b]->pressCt ++;      // count the press
                        section[i]._btn[b]->timePressed = currentTime; // save the time
                    } 
                    else if (currentTime >= (section[i]._btn[b]->timePressed + BTN_FADE_DELAY)) 
                        btnHeldActions(i, b);

                }
            } // button loop
        } // section loop
    } // timer loop
} // void loop
