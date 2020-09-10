#include <Arduino.h>
#include "globals.h"
#include "eth.h"
#include "nrf.h"

void setup() {
    DEBUG.begin(115200);
    nrfInit();
    ethInit();
}

void loop() {
    nrfLoop();
}