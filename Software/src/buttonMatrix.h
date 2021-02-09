#pragma once
#include <stdint.h>

void btnInit();
void btnLoop();
void btnSetLed(uint8_t index, uint8_t state);
void btnEnableMultiplexing(bool status);