#pragma once
#define SENDER

#ifdef __AVR__
    #define NRF_CS_PIN  8
    #define NRF_CE_PIN  7
#else
    #define NRF_CS_PIN  PA4
    #define NRF_CE_PIN  PA1
#endif

#ifdef SENDER
    #define NRF_TX_ADDRESS "DGTW"
    #define NRF_RX_ADDRESS  "DRMT0"
#else
    #define NRF_RX_ADDRESS "DGTW"
    #define NRF_TX_ADDRESS  "DRMT0"
#endif

#define MAX_PACKET_LEN  256

// #define DEBUG Serial // use for serial output only
#define DEBUG_SERIAL Serial // output to serial and OLED