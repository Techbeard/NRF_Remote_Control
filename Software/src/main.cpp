#include <Arduino.h>
#include "nrf.h"
#include "config.h"
#include "globals.h"
#include "display.h"
#include "buttonMatrix.h"

#ifdef DEBUG_SERIAL
DebugPrints DEBUG;
#endif

bool startup;

void handleData(uint8_t* buf, uint16_t len) {
    DEBUG.println((char*)buf);
}

void setup() {
    DEBUG.begin(115200);
    // pinMode(PC13, OUTPUT);
    // digitalWrite(PC13, HIGH);
    btnInit();
    startup = false;
    // while(!startup) {
    //     btnLoop();
    // }
    displayInit();
    DEBUG.println("NRF24 Remote");
    nrfInit();
    setNRFCallback(handleData);
}

uint32_t lastSend = 0;

void loop() {
    nrfLoop();
    btnLoop();
    #ifdef SENDER
    if(millis() - lastSend > 500) {
        lastSend = millis();
        nrfSendUDP(IPAddress((uint32_t)0x01020304), 12345, "Hello World " + String(millis()));
    }
    #endif
    // delay(500);
    // digitalWrite(PC13, HIGH);
    // DEBUG.print(".");
    // delay(500);
    // digitalWrite(PC13, LOW);

}