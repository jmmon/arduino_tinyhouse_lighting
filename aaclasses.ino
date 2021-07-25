/*
 * BTN CLASS DEFINITION
 */
class Btn_C {
    //private:
    public:
        uint32_t timeReleased = 0;
        uint32_t timePressed = 0;

        uint8_t pressCtr = 0;
        bool isHeld = false;

        //no constructor
        //functions/methods
        void registerPress() {
            pressCtr++;  // add a press
            timePressed = currentTime; // save the time
        }

        void registerRelease() {
            timePressed = 0; // reset depressed timer
            timeReleased = currentTime; // save the time
        }

};

Btn_C btn[] = {
    // left buttons (inside under loft)
    Btn_C(), // entry button up  
    Btn_C(), // entry button down

    // porch
    Btn_C(), // entry2 button up
    Btn_C(), // entry2 button down

    // kitchen (under sm loft)
    Btn_C(), // kitchen 
    Btn_C(),

    // TODO
    Btn_C(), // bath
    Btn_C(),

    // TODO:
    // sconce 1 button close button
};

// /*
//  * SECTION CLASS DEFINITION
//  */

class Section_C {
    private:
    public:
        // public vars, constructor, methods
        Btn_C *_btn[2];
        uint8_t PIN;
        uint8_t DMX_OUT;
        float BRIGHTNESS_FACTOR = 1;

        bool isOn = false;
        bool colorProgress = false;
        bool extendedFade = false;

        float masterLevel = 1;
        float lastMasterLevel = 0;
        uint8_t mode = 0;

        uint32_t colorStartTime = 0;
        uint16_t colorDelayInt = 0;
        float colorDelayCtr = 0;
        uint8_t colorState = 0;

        bool RGBWon[4] = {false, false, false, false};
        float RGBW[4] =  {0., 0., 0., 0.};
        float lastRGBW[4] =  {0., 0., 0., 0.};
        float nextRGB[3] =  {0., 0., 0.};

        Section_C(uint8_t pin, uint8_t DMX_OUT, Btn_C* btn0, Btn_C* btn1, float BF) {
            PIN = pin;
            DMX_OUT = DMX_OUT;
            pinMode(pin, INPUT);
            _btn[0] = btn0;
            _btn[1] = btn1;
            BRIGHTNESS_FACTOR = BF;
        };

        //methods for lights:
        //fade up/down
        //switch mode
        //updatelights
        //colorProgress
        //
};

Section_C sectionC[] = {
    Section_C(ENTRY_BTN_PIN, 4, &btn[0], &btn[1], 1.0), // under main loft
    Section_C(KITCHEN_BTN_PIN, 3, &btn[4], &btn[5], 1.32), // kitchen
    Section_C(ENTRY2_BTN_PIN, 2, &btn[2], &btn[3], 1.6), // porch
    Section_C(BATH_BTN_PIN, 1, &btn[6], &btn[7], 1.0), // bath TODO

   //  ID     Overhead Bedroom
   //  ID     Overhead Main
   //  ID     Overhead Small Loft
   //  ID     Greenhouse Lights
};


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