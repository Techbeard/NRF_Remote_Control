#include <Arduino.h>
#include "nrf.h"
#include "config.h"

void handleData(uint8_t* buf, uint16_t len) {
    Serial.println((char*)buf);
}

void setup() {
    Serial.begin(115200);
    initNRF();
    setNRFCallback(handleData);
}

uint32_t lastSend = 0;

void loop() {
    loopNRF();
    #ifdef SENDER
    if(millis() - lastSend > 500) {
        lastSend = millis();
        sendUDP(IPAddress((uint32_t)0x01020304), 12345, "Hello World " + String(millis()));
    }
    #endif
}