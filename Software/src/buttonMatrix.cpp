#include "buttonMatrix.h"
#include <Arduino.h>
#include "config.h"
#include "globals.h"

HardwareTimer btnMatrixTimer(TIM4); 

uint8_t btnCols[] = BTN_COLS;
uint8_t btnRows[] = BTN_ROWS;
uint8_t ledRows[] = BTN_LED_ROWS;

#define BTN_COL_NUM (sizeof(btnCols) / sizeof(btnCols[0]))
#define BTN_ROW_NUM (sizeof(btnRows) / sizeof(btnRows[0]))

// Button and LED indices are row-major
#define BTN_MAX_INDEX (BTN_COL_NUM * BTN_ROW_NUM)

uint8_t prevBtnState[BTN_MAX_INDEX] = {0}; // can probably expend the few extra bytes
uint8_t btnState[BTN_MAX_INDEX] = {0}; 
uint8_t ledState[BTN_MAX_INDEX] = {0};
uint8_t curColIdx = 0;
bool btnScanDone = false;

void btnScanLoop(HardwareTimer* ht);
void processButtons();

void btnInit() {
    for (int i = 0; i < BTN_COL_NUM; i++) {
        pinMode(btnCols[i], INPUT);
        digitalWrite(ledRows[i], LOW);
    }
    for (int i = 0; i < BTN_ROW_NUM; i++) {
        pinMode(btnRows[i], INPUT_PULLUP);

        pinMode(ledRows[i], INPUT);
        digitalWrite(ledRows[i], HIGH);
    }

    btnMatrixTimer.attachInterrupt(btnScanLoop);
    btnMatrixTimer.setOverflow(100 * BTN_COL_NUM, HERTZ_FORMAT);
    btnMatrixTimer.resume();
}

// TODO: do debouncing
// Function gets called in timer interrupt, no need to call manually
void btnScanLoop(HardwareTimer* ht) {
    // deactivate previous column
    pinMode(btnCols[(curColIdx + (BTN_COL_NUM - 1)) % BTN_COL_NUM], INPUT);

    for (int row = 0; row < BTN_ROW_NUM; row++) {
        // disable LEDs first to prevent ghosting
        pinMode(ledRows[row], INPUT);
    }

    // activate column to read
    pinMode(btnCols[curColIdx], OUTPUT);

    // loop through all rows
    for (int row = 0; row < BTN_ROW_NUM; row++) {
        uint16_t index = row * BTN_COL_NUM + curColIdx;
        // prevent losing a button transition if it was not processed yet (hopefully)
        if(!btnScanDone) {
            // read button state
            btnState[index] = digitalRead(btnRows[row]);
        }

        // set LED to the stored state
        pinMode(ledRows[row], ledState[index] ? OUTPUT : INPUT);
    }

    // // deactivate column
    // digitalWrite(btnCols[curColIdx], HIGH);

    // increment counter to next column
    curColIdx++;
    if(curColIdx >= BTN_COL_NUM) {
        curColIdx = 0;
        btnScanDone = true;
    }
}

void processButtons() {
    // loop through all buttons
    for (uint16_t i = 0; i < BTN_MAX_INDEX; i++) {
        if (prevBtnState[i] != btnState[i]) {
            if (!btnState[i]) { // button was pressed
                switch(i) {
                    case 0:
                        startup = true;
                        break;
                    default:
                        DEBUG.print(String(i)); // print number to debug
                }

                ledState[i] = !ledState[i]; // toggle the button LED
            }
            else { // button was released

            }
        }
        prevBtnState[i] = btnState[i]; // remember current button state as previous state
    }
}

void btnLoop() {
    if(btnScanDone) {
        btnScanDone = false;
        processButtons();
    }
}