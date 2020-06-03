#pragma once

#include "config.h"
#include "display.h"

#ifdef DEBUG_SERIAL
class DebugPrints {
    public:
    void begin(uint32_t baud) {DEBUG_SERIAL.begin(baud);}
    void print(String str) {DEBUG_SERIAL.print(str); dispPrint(str);}
    void println(String str) {DEBUG_SERIAL.println(str); dispPrintln(str);}
};
extern DebugPrints DEBUG;
#endif
