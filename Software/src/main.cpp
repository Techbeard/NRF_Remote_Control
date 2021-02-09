// Set UIP_CONF_UDP to 1 in RF24Ethernet/uip-conf.h

#include <Arduino.h>
#include "nrf.h"
#include "config.h"
#include "globals.h"
#include "display.h"
#include "buttonMatrix.h"

#ifdef DEBUG_SERIAL
DebugPrints DEBUG;
#endif

// enable printf on DEBUG
extern "C" {
    int _write(int file, char *ptr, int len) {
        DEBUG.write(ptr, len);
        return len;
    }
}

void setup() {
    DEBUG.begin(115200);
    btnInit();
    displayInit();
    DEBUG.println("NRF24 Remote");
    nrfInit();
    DEBUG.println("Remote ready for use!");
}

uint32_t lastSend = 0;

void loop() {
    nrfLoop();
    btnLoop();
}