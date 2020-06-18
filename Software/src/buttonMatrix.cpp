#include "buttonMatrix.h"
#include <Arduino.h>
#include "config.h"
#include "globals.h"

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
}

// TODO: convert for loop to index++ and call by timer and do debouncing
void btnLoop() {
    for (int col = 0; col < BTN_COL_NUM; col++) {
        digitalWrite(btnCols[col], LOW);

        for (int row = 0; row < BTN_ROW_NUM; row++) {
            uint16_t index = col * BTN_ROW_NUM + row;
            btnState[index] = digitalRead(btnRows[row]);

            pinMode(ledRows[row], ledState[index] ? OUTPUT : INPUT);
        }

        digitalWrite(btnCols[col], HIGH);
    }

    for (uint16_t i = 0; i < BTN_MAX_INDEX; i++) {
        if (prevBtnState[i] != btnState[i]) {
            if (!btnState[i]) { // button was pressed
                DEBUG.print(String(i));
                ledState[i] = !ledState[i];
            }
            else { // button was released

            }
        }
        prevBtnState[i] = btnState[i];
    }
}