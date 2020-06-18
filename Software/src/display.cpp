#include "display.h"

#include "Wire.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Full buffer
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Full buffer
// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Paged mode

#define MAX_LINE_CHARS 44   // a bit longer and it starts to overflow to the beginning of the line
#define LINE_HEIGHT 10
#define LINE_COUNT  (64 / LINE_HEIGHT)
String terminalLines[LINE_COUNT];
int lineIdx = 0;
bool newLine = true;

void dispDrawPrints() {
    u8g2.clearBuffer();
    for (int i = 0; i < LINE_COUNT; i++) {
        uint8_t length = u8g2.drawStr(0, LINE_HEIGHT * (i + 1), terminalLines[i].c_str());

        // do a premature newline if current line that gets written to overflows the display
        if (i == lineIdx && length > u8g2.getWidth()) { 
            lineIdx++;
            newLine = true;
        }
    }
    u8g2.sendBuffer();
}


void dispPrint(String str) {
    String line;
    if (newLine) {
        newLine = false;
        if (lineIdx == LINE_COUNT) {
            lineIdx = LINE_COUNT - 1;
            for (int i = 0; i < LINE_COUNT - 1; i++) { // yeah, this solution is shitty, but I couldn't be bothered to write a correct wrap-around
                terminalLines[i] = terminalLines[i + 1];
            }
        }
        terminalLines[lineIdx] = "";
    }
    line = terminalLines[lineIdx] += str;

    if (line.length() > MAX_LINE_CHARS) {
        line = line.substring(0, MAX_LINE_CHARS);
        line += "\n";
    }

    terminalLines[lineIdx] = line;

    if (line.indexOf("\n") > -1) {
        lineIdx++;
        if (lineIdx > LINE_COUNT) { // lineIdx == LineCount will be handled on next print
            lineIdx = LINE_COUNT; // make sure lineIdx doesn't go out of bounds
        }
        newLine = true;
    }
    dispDrawPrints();
}

void dispPrintln(String str) {
    dispPrint(str + "\n");
}

void displayInit() {
    u8g2.begin();
    u8g2.setI2CAddress(0x78);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_nerhoe_tr);
    u8g2.drawStr(0, 10, "Hello World!\nNEWLINE!");
    u8g2.sendBuffer();
}

void displayLoop();