#include <SPI.h>
#include <RF24.h>
#include <IPAddress.h>

typedef enum : uint8_t {
    CMD_INIT = 0,
    CMD_DATA = 1
} pkgType_t;

typedef union {
    struct {
        uint8_t type : 7;
        uint8_t lastPacket : 1;
    } ;
    uint8_t raw;
} cmd_t;

#define INIT_HEADER_LEN     7
#define INIT_PAYLOAD_LEN    (30 - INIT_HEADER_LEN)
typedef struct {
    uint32_t ip;
    uint16_t port;
    uint8_t dataLen;
    uint8_t* data;
} initPayload_t;

#define DATA_HEADER_LEN     2
#define DATA_PAYLOAD_LEN    (30 - DATA_HEADER_LEN)
typedef struct {
    uint8_t packetNum;
    uint8_t dataLen;
    uint8_t* data;
} dataPayload_t;

typedef struct {
    cmd_t cmd;
    uint8_t checksum;
    union {
        initPayload_t initPayload;
        dataPayload_t dataPayload;
        uint8_t* rawPayload;
    };
} packet_t;

void initNRF();
void loopNRF();
void sendUDP(IPAddress ip, uint16_t port, String payload);
void sendUDP(IPAddress ip, uint16_t port, uint8_t* payload, uint16_t size);
void setNRFCallback(void (*cb)(uint8_t*, uint16_t));