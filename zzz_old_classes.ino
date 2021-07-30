
// class RGBW_C {
//     private:
//     public:
//         bool on = false;
//         float level = 0;
//         float lastLevel = 0;
//         float nextLevel = 0;
        
//         RGBW_C() { //constructor

//         }


// };

// RGBW_C RGBW[][4] { // [section][color]
//     {RGBW_C(), RGBW_C(), RGBW_C(), RGBW_C(),},
//     {RGBW_C(), RGBW_C(), RGBW_C(), RGBW_C(),},
//     {RGBW_C(), RGBW_C(), RGBW_C(), RGBW_C(),},
//     {RGBW_C(), RGBW_C(), RGBW_C(), RGBW_C(),},
    
// };


// /*
//  * SECTION CLASS DEFINITION
//  */

// class Section {
//     private:
//     public:
//         //Btn *_btnC[2];
//         uint8_t DMX_OUT;
//         float BRIGHTNESS_FACTOR;
//         uint8_t PIN;

//         bool isOn;
//         bool colorProgress;
//         bool extendedFade;

//         float masterLevel;
//         float lastMasterLevel;
//         uint8_t mode;

//         uint32_t colorStartTime;
//         uint16_t colorDelayInt;
//         float colorDelayCtr;
//         uint8_t colorState;

//         bool RGBWon[4];
//         float RGBW[4];
//         float lastRGBW[4];
//         float nextRGB[3];

//         //public vars, constructor, methods
//         Section(uint8_t DMX_OUT, uint8_t pin) {   //, *Btn btn1, *Btn btn2) {
//             DMX_OUT = DMX_OUT;
//             PIN = pin;
//             pinMode(pin, INPUT);
//             // _btnC[0] = &btn1;
//             // _btnC[1] = &btn2;
//         }
//         //methods for lights:
//         //fade up/down
//         //switch mode
//         //updatelights
//         //colorProgress
//         //
//         void updateLights(uint8_t i) { //might not need to take i but will for now
//             if (DEBUG) {
//                 DEBUG_updateLights(i);

//             } else {
//                 uint8_t brightnessValue = 0; // index for brightness lookup table

//                 for (uint8_t color = 0; color < 4; color++) {
//                     if (RGBWon[color]) {

//                         //brightnessValue = lookupTable(i, color);
                        
//                         uint8_t height = (uint16_t(RGBW[color] * masterLevel * TABLE_SIZE) / HEIGHT);
//                         uint8_t width = (uint16_t(RGBW[color] * masterLevel * TABLE_SIZE) % WIDTH);

//                         // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
//                         memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 

//                         if ((RGBW[color] > 0) && (masterLevel > 0) && (brightnessValue == 0)) 
//                             brightnessValue = 1;
                        
//                     } else {
//                         //brightnessValue = 0;
//                         // light is off, so turn it off
//                     }

//                     DmxSimple.write((DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
//                     lastRGBW[color] = RGBW[color];
//                 }
//             }
                
//             if ( (RGBW[0] <= 0) && (RGBW[1] <= 0) && (RGBW[2] <= 0) && (RGBW[3] <= 0) ) {
//                 //switch to mode 0?

//                 isOn = false;
//                 masterLevel = 0;

//                 for (uint8_t color = 0; color < 4; color++) {  //clear rgbw
//                     RGBW[color] = 0;
//                     RGBWon[color] = false;
//                 }

//                 if (DEBUG) {
//                     DEBUG_updateLightsOff(i);
//                 }
//             }
//         }
// };

// Section sectionC[] = {
//     Section(4, ENTRY_BTN_PIN), //, &btnC[0], &btnC[1]),
//     Section(3, KITCHEN_BTN_PIN),
//     Section(2, ENTRY2_BTN_PIN),
//     Section(1, BATH_BTN_PIN),
// };