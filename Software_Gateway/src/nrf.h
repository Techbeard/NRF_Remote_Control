#pragma once
#include <IPAddress.h>

void nrfInit();
void nrfLoop();
void nrfSendIpPacketToRemote(IPAddress fromIp, uint16_t fromPort, char *payload, int payloadSize);