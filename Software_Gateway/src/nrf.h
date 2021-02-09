#pragma once
#include <IPAddress.h>

void nrfInit();
void nrfLoop();
void nrfSendIpPacketToRemote(IPAddress fromIp, uint16_t fromPort, char *payload, int payloadSize);

typedef struct {
    uint32_t fromIp;
    uint16_t fromPort;
    uint16_t payloadLen;
    uint8_t payload[];
} dataToRemote_t;