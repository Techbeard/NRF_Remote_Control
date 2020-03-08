#include <SPI.h>
#include <RF24.h>
#include <IPAddress.h>

#define MAX_PACKET_LEN  1500

typedef enum : uint8_t {
    CMD_INIT = 0,
    CMD_DATA = 1,
    CMD_DATA_END = 2
} cmd_t;

#define INIT_HEADER_LEN     9
#define INIT_PAYLOAD_LEN    (32 - INIT_HEADER_LEN)
typedef struct {
    cmd_t cmd;
    uint32_t ip;
    uint16_t port;
    uint8_t checksum;
    uint8_t len;
    uint8_t* payload;
} initPacket_t;

#define DATA_HEADER_LEN     4
#define DATA_PAYLOAD_LEN    (32 - DATA_HEADER_LEN)
typedef struct {
    cmd_t cmd;
    uint8_t packetNum;
    uint8_t checksum;
    uint8_t len;
    uint8_t* payload;
} dataPacket_t;

void initNRF();
void loopNRF();
void sendUDP(IPAddress ip, uint16_t port, String payload);
void sendUDP(IPAddress ip, uint16_t port, uint8_t* payload, uint16_t size);