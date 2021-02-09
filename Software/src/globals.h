#pragma once

#include "config.h"
#include "display.h"

extern bool startup;

#ifdef DEBUG_SERIAL
class DebugPrints {
    public:
    void begin(uint32_t baud) {DEBUG_SERIAL.begin(baud);}
    void print(String str) {DEBUG_SERIAL.print(str); dispPrint(str);}
    void println(String str) {DEBUG_SERIAL.println(str); dispPrintln(str);}
    void write(const char *buffer, size_t size) {DEBUG_SERIAL.write(buffer, size); dispWrite(buffer, size);}
};
extern DebugPrints DEBUG;
#endif
