#include "display.h"

#include "Wire.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Full buffer
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Full buffer
// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Paged mode

#define MAX_LINE_CHARS 44   // a bit longer and it starts to overflow to the beginning of the line
#define LINE_HEIGHT 10
#define STATUSBAR_HEIGHT 10
#define LINE_COUNT  ((64 - STATUSBAR_HEIGHT) / LINE_HEIGHT)
#define DEFAULT_FONT u8g2_font_nerhoe_tr
String terminalLines[LINE_COUNT];
int lineIdx = 0;
bool newLine = true;

void dispDrawPrints() {
    // clear text area
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, STATUSBAR_HEIGHT+1, u8g2.getWidth(), u8g2.getHeight());
    u8g2.setDrawColor(1);

    u8g2.setFont(DEFAULT_FONT);
    for (int i = 0; i < LINE_COUNT; i++) {
        uint8_t length = u8g2.drawStr(0, LINE_HEIGHT * (i + 1) + STATUSBAR_HEIGHT + 1, terminalLines[i].c_str());

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

// ATTENTION: non-null-terminated strings WILL cause a buffer overread
void dispWrite(const char *buffer, size_t size) {
    dispPrint(buffer);
}


void dispDrawStatusbar() {
    // clear statusbar area
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 0, u8g2.getWidth()-1, STATUSBAR_HEIGHT);
    u8g2.setDrawColor(1);
    u8g2.drawHLine(0, STATUSBAR_HEIGHT, u8g2.getWidth()); // draw line

    // mock data
    uint8_t batteryPercent = 42;
    bool connected = false;
    String titleText = "NRF24 Remote";

    char batteryStr[8];
    snprintf(batteryStr, 8, "%d%%", batteryPercent);

    u8g2.setFontMode(0);
    u8g2.setFont(DEFAULT_FONT);
    uint16_t batteryStrWidth = u8g2.getStrWidth(batteryStr);
    uint16_t batteryX = u8g2.getWidth() - batteryStrWidth - 11; // calculate battery symbol position
    uint16_t wirelessStatusX = (9 + 8); // end of wirelessStatus symbol
    uint16_t midpoint = (wirelessStatusX + batteryX) / 2;
    int titleWidth = u8g2.getUTF8Width(titleText.c_str());
    int titleX = midpoint - titleWidth / 2;
    titleX = max(titleX, wirelessStatusX + 4); // don't overdraw onto left icons

    u8g2.drawUTF8(titleX, STATUSBAR_HEIGHT - 2, titleText.c_str());

    u8g2.setDrawColor(0);
    u8g2.drawBox(batteryX - 3, 0, u8g2.getWidth() - batteryX + 3, STATUSBAR_HEIGHT);    // clear potentially overflowed title text to the right
    u8g2.setDrawColor(1);

    u8g2.drawStr(u8g2.getWidth() - batteryStrWidth, STATUSBAR_HEIGHT - 2, batteryStr);  // battery text

    u8g2.setFont(u8g2_font_open_iconic_embedded_1x_t);
    u8g2.drawGlyph(batteryX, STATUSBAR_HEIGHT - 2, batteryPercent <= 30 ? 0x40 : 0x49); // battery symbol
    u8g2.drawGlyph(0, STATUSBAR_HEIGHT - 2, 0x50); // wireless symbol

    u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
    u8g2.drawGlyph(9, STATUSBAR_HEIGHT - 2, connected ? 0x40 : 0x44); // wireless status



}

void displayInit() {
    u8g2.begin();
    u8g2.setI2CAddress(0x78);
    u8g2.clearBuffer();
    u8g2.setFont(DEFAULT_FONT);
    dispDrawStatusbar();
    // u8g2.drawStr(0, 10, "Hello World!\nNEWLINE!");
    u8g2.sendBuffer();
}

void displayLoop();