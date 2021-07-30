/*
 * BTN CLASS DEFINITION
 */
class Btn_C {
    private:
    public:
        uint32_t timeReleased = 0;
        uint32_t timePressed = 0;
        uint8_t pressCtr = 0;
        bool isHeld = false;
        //isHeld if time passes heldLength req and timePressed != 0

        // bool isHeld() {
        //     return ((timePressed != 0) && (currentTime > (timePressed + BTN_HELD_DELAY)));
        // }
        
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

// /*
//  * SECTION CLASS DEFINITION
//  */

// /*
class section_c { //rebuild of other class? Not sure why class isn't working
    public:
        Btn_C btn[2];
        uint8_t PIN;
        uint8_t DMX_OUT;          // DMX OUT number (set of 4 channels) for this section
        float BRIGHTNESS_FACTOR; // affects [default brightness + fade speed], pref range [0-1]
    
        bool isOn = false;              // Are any levels > 0?
        bool colorProgress = false;     // while true, colors for this section will cycle
        bool extendedFade = false;      // enable extended fade
    
        float masterLevel = 1.0; // master brightness level
        float lastMasterLevel = 0.0; // level from last time the light was on
        uint8_t mode = 0; // 0-4: WW, colors, colors+ww, (All, Nightlight)
            // typical cycle is 0-1-2-0... modes 3 and 4 are hidden from the typical cycle.
    
        uint32_t colorStartTime = 0;   // starting time of colorProgress
        uint16_t colorDelayInt = 0;     // adjust this to change colorProgress speed
        float colorDelayCtr = 0; // slows the color cycle, used to slow the "sudden" mode
        uint8_t colorState = 0;        // next color state in the cycle
    
        bool RGBWon[4] = {false, false, false, false};     // is on? each color
        float RGBW[4] = {0., 0., 0., 0.};     // stores current RGBW color levels
        float lastRGBW[4] = {0., 0., 0., 0.}; // last color levels
        float nextRGB[3] = {0., 0., 0.}; // next state of RGB for colorProgress cycle. Stores index of lookup table. Could be modified by colorFadeLevel to change the max level for the colorProgress.
        
        section_c(uint8_t pin, uint8_t dmx_out, float bf) {
            btn[0] = Btn_C();
            btn[1] = Btn_C();
            PIN = pin;
            pinMode(pin, INPUT);
            DMX_OUT = dmx_out;
            BRIGHTNESS_FACTOR = bf;
        };

        //methods for lights:
        //fade up/down
        //switch mode
        //updatelights
        //colorProgress
        
} section[] = {
    section_c(ENTRY_BTN_PIN, 4, 1.0), // under main loft
    section_c(KITCHEN_BTN_PIN, 3,  1.32), // kitchen
    section_c(ENTRY2_BTN_PIN, 2,  1.6), // porch
    section_c(BATH_BTN_PIN, 1, 1.0), // bath TODO

    //  ID     Overhead Bedroom
    //  ID     Overhead Main
    //  ID     Overhead Small Loft
    //  ID     Greenhouse Lights
};




























  ///*/

 /*

struct section_t {
    Btn_C btn[2];
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
        {Btn_C(), Btn_C()},
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
        {Btn_C(), Btn_C()},
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
        {Btn_C(), Btn_C()},
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
        {Btn_C(), Btn_C()},
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


//  */
