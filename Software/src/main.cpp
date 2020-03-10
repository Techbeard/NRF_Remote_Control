#include <Arduino.h>
#include "nrf.h"

void handleData(uint8_t* buf, uint16_t len) {
    Serial.println((char*)buf);
}

void setup() {
    initNRF();
    setNRFCallback(handleData);
}

void loop() {
    loopNRF();
}