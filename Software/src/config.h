#ifdef __AVR__
    #define NRF_CS_PIN  8
    #define NRF_CE_PIN  7
#else
    #define NRF_CS_PIN  PA4
    #define NRF_CE_PIN  PA1
#endif

// #define NRF_GATEWAY_ADDRESS "DGTW"
// #define NRF_REMOTE_ADDRESS  "DRMT0"

#define NRF_REMOTE_ADDRESS "DGTW"
#define NRF_GATEWAY_ADDRESS  "DRMT0"

#define MAX_PACKET_LEN  256