#pragma once
// #define SENDER


#ifdef __AVR__
    #define NRF_CS_PIN  8
    #define NRF_CE_PIN  7
#else
    #define NRF_CS_PIN  PA4
    #define NRF_CE_PIN  PB13
#endif

// OLED pins are PB6 for SCL and PB7 for SDA

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

// #define BTN_COLS        {PB15,  PA8,    PA9} //test
// #define BTN_ROWS        {PA10, PA11,  PA12} // test
#define BTN_COLS        {PA10,  PA2,    PA1}
#define BTN_ROWS        {PB3,   PB5,    PB9}
#define BTN_LED_ROWS    {PA15,  PB4,    PB8}