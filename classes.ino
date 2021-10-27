/*
 * BTN CLASS DEFINITION
 */
//class Btn {
//    private:
//        uint8_t pinBtn;
//
//        uint32_t timeReleased = 0;
//        uint32_t timePressed = 0;
//
//        uint8_t pressCt = 0;
//
//        bool isHeld = false;
//
//    public:
//        //constructor
//        Btn(uint8_t pinBtn) {
//            this->pinBtn = pinBtn;
//
//            pinMode(pinBtn, INPUT);
//        }
//
//        //functions/methods
//        void registerPress(uint32_t t) { // takes the currentTime
//            pressCt++;  // add a press
//            timePressed = t; // save the time
//            // section[i].btn[b].pressCt ++;
//            // section[i].btn[b].timePressed = currentTime;
//        }
//
//        void registerRelease(uint32_t t) { // takes the currentTime
//            timePressed = 0; // reset
//            timeReleased = t; // save the time
//            // section[i].btn[b].timePressed = 0;
//            // section[i].btn[b].timeReleased = currentTime;
//        }
//
//};
//
//Btn btnC[] = {
//    Btn(ENTRY_BTN_PIN),
//    Btn(ENTRY_BTN_PIN),
//
//    Btn(KITCHEN_BTN_PIN),
//    Btn(KITCHEN_BTN_PIN),
//
//    Btn(ENTRY2_BTN_PIN),
//    Btn(ENTRY2_BTN_PIN),
//
//    Btn(BATH_BTN_PIN),
//    Btn(BATH_BTN_PIN),
//};















//
//
///*
// * SECTION CLASS DEFINITION
// */
//
//class Section {
//    private:
//        //Btn *_btnC[2];
//        uint8_t DMX_OUT;
//        float BRIGHTNESS_FACTOR;
//
//        bool RGBWM[4]->isOn;
//        bool colorProgress;
//        bool extendedFade;
//
//        float RGBWM[4]->lvl;
//        float lastMasterLevel;
//        uint8_t mode;
//
//        uint32_t colorStartTime;
//        uint16_t colorProgDelay;
//        float colorProgSuddenDelayCtr;
//        uint8_t colorState;
//
//        bool RGBWM[4]->isOn;
//        float RGBWM[4]->lvl;
//        float RGBWM[4]->lastLvl;
//        float nextRGB[3];
//
//    public:
//        //public vars, constructor, methods
//        Section(uint8_t DMX_OUT) {   //, *Btn btn1, *Btn btn2) {
//            DMX_OUT = DMX_OUT;
//            // _btnC[0] = &btn1;
//            // _btnC[1] = &btn2;
//        }
//        //methods for lights:
//        //fade up/down
//        //switch mode
//        //updatelights
//        //colorProgress
//        //
//        void updateLights(uint8_t i) { //might not need to take i but will for now
//            if (DEBUG) {
//                DEBUG_updateLights(i);
//
//            } else {
//                uint8_t brightnessValue = 0; // index for brightness lookup table
//
//                for (uint8_t color = 0; color < 4; color++) {
//                    if (RGBWM[color]->isOn) {
//
//                        //brightnessValue = lookupTable(i, color);
//                        
//                        uint8_t height = (uint16_t(RGBWM[color]->lvl * RGBWM[4]->lvl * TABLE_SIZE) / HEIGHT);
//                        uint8_t width = (uint16_t(RGBWM[color]->lvl * RGBWM[4]->lvl * TABLE_SIZE) % WIDTH);
//
//                        // look up brighness from table and saves as uint8_t brightness ( sizeof(brightness) resolves to 1 [byte of data])
//                        memcpy_P(&brightnessValue, &(DIMMER_LOOKUP_TABLE[height][width]), sizeof(brightnessValue)); 
//
//                        if ((RGBWM[color]->lvl > 0) && (RGBWM[4]->lvl > 0) && (brightnessValue == 0)) 
//                            brightnessValue = 1;
//                        
//                    } else {
//                        //brightnessValue = 0;
//                        // light is off, so turn it off
//                    }
//
//                    DmxSimple.write((DMX_OUT * 8) - 8 + (color * 2 + 1), brightnessValue);
//                    RGBWM[color]->lastLvl = RGBWM[color]->lvl;
//                }
//            }
//                
//            if ( (RGBWM[0]->lvl <= 0) && (RGBWM[1]->lvl <= 0) && (RGBWM[2]->lvl <= 0) && (RGBWM[3]->lvl <= 0) ) {
//                //switch to mode 0?
//
//                RGBWM[4]->isOn = false;
//                RGBWM[4]->lvl = 0;
//
//                for (uint8_t color = 0; color < 4; color++) {  //clear rgbw
//                    RGBWM[color]->lvl = 0;
//                    RGBWM[color]->isOn = false;
//                }
//
//                if (DEBUG) {
//                    DEBUG_updateLightsOff(i);
//                }
//            }
//        }
//};
//
//Section sectionC[] = {
//    Section(4), //, &btnC[0], &btnC[1]),
//    Section(3),
//    Section(2),
//    Section(1),
//};
