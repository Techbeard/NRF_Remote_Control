#pragma once 

#include <IPAddress.h>
#include "stdint.h"

void nrfInit();
void nrfLoop();
void nrfSendUDP(IPAddress ip, uint16_t port, String payload);
void nrfSendUDP(IPAddress ip, uint16_t port, uint8_t* payload, uint16_t size);
void nrfSendButtonAction(uint8_t btnId);

typedef struct {
    IPAddress ip;
    uint16_t port;
    const char* payload;
} buttonAction_t;