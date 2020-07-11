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

void btnLoop(HardwareTimer* ht);
void processButtons();

void btnInit() {
    for (int i = 0; i < BTN_COL_NUM; i++) {
        pinMode(btnCols[i], OUTPUT);
        digitalWrite(ledRows[i], HIGH);
    }
    for (int i = 0; i < BTN_ROW_NUM; i++) {
        pinMode(btnRows[i], INPUT_PULLUP);

        pinMode(ledRows[i], INPUT);
        digitalWrite(ledRows[i], HIGH);
    }

    btnMatrixTimer.attachInterrupt(btnLoop);
    btnMatrixTimer.setOverflow(100 * BTN_COL_NUM, HERTZ_FORMAT);
    btnMatrixTimer.resume();
}

// TODO: do debouncing
// Function gets called in timer interrupt, no need to call manually
void btnLoop(HardwareTimer* ht) {

    // activate column to read
    digitalWrite(btnCols[curColIdx], LOW);

    // loop through all rows
    for (int row = 0; row < BTN_ROW_NUM; row++) {
        uint16_t index = curColIdx * BTN_ROW_NUM + row;
        // read button state
        btnState[index] = digitalRead(btnRows[row]);

        // set LED to the stored state
        pinMode(ledRows[row], ledState[index] ? OUTPUT : INPUT);
    }

    // deactivate column
    digitalWrite(btnCols[curColIdx], HIGH);

    // increment counter to next column
    curColIdx++;
    if(curColIdx >= BTN_COL_NUM) {
        curColIdx = 0;
        processButtons();
    }
}

void processButtons() {
    // loop through all buttons
    for (uint16_t i = 0; i < BTN_MAX_INDEX; i++) {
        if (prevBtnState[i] != btnState[i]) {
            if (!btnState[i]) { // button was pressed
                DEBUG.print(String(i)); // print number to debug
                ledState[i] = !ledState[i]; // toggle the button LED
            }
            else { // button was released

            }
        }
        prevBtnState[i] = btnState[i]; // remember current button state as previous state
    }
}