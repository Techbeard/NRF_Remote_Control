#pragma once
#define SENDER


#define TARGET_IP   IPAddress(192,168,137,1)
#define TARGET_PORT 1234

#define BUTTON_MAPPING \
    {TARGET_IP, TARGET_PORT, "Button 0"}, \
    {TARGET_IP, TARGET_PORT, "Button 1"}, \
    {TARGET_IP, TARGET_PORT, "Button 2"}, \
    {TARGET_IP, TARGET_PORT, "Button 3"}, \
    {TARGET_IP, TARGET_PORT, "Button 4"}, \
    {TARGET_IP, TARGET_PORT, "Button 5"}, \
    {TARGET_IP, TARGET_PORT, "Button 6"}, \
    {TARGET_IP, TARGET_PORT, "Button 7"}, \
    {TARGET_IP, TARGET_PORT, "Button 8"}, \

#ifdef __AVR__
    #define NRF_CS_PIN  8
    #define NRF_CE_PIN  7
#else
    #define NRF_CS_PIN  PA4
    #define NRF_CE_PIN  PB13
#endif

// OLED pins are PB6 for SCL and PB7 for SDA

#ifdef SENDER
    #define NRF_TX_ADDRESS  0x47545700
    #define NRF_RX_ADDRESS  0x47545701
#else
    #define NRF_RX_ADDRESS  0x47545700
    #define NRF_TX_ADDRESS  0x47545701
#endif

#define MAX_PACKET_LEN  256

// #define DEBUG SerialUSB // use for serial output only
#define DEBUG_SERIAL SerialUSB // output to serial and OLED

// #define BTN_COLS        {PB15,  PA8,    PA9} //test
// #define BTN_ROWS        {PA10, PA11,  PA12} // test
#define BTN_COLS        {PA10,  PA2,    PA1}
#define BTN_ROWS        {PB3,   PB5,    PB9}
#define BTN_LED_ROWS    {PA15,  PB4,    PB8}

#define BTN_NUM 9