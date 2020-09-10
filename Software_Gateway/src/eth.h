#pragma once

#include <stddef.h>
#include <stdint.h>

void ethInit();
void ethForwardIpPacket(uint8_t *rawIpBytes, size_t size);