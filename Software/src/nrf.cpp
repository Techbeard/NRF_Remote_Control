#include "nrf.h"
#include "config.h"

RF24 rf(NRF_CE_PIN, NRF_CS_PIN);

uint8_t txBuf[MAX_PACKET_LEN];
uint16_t txLen = 0, txOffset = 0;

uint8_t calcChecksum(initPacket_t pkg) {
    uint8_t chk = 0;
    chk ^= pkg.cmd;
    chk ^= (pkg.ip >> 24) & 0xFF;
    chk ^= (pkg.ip >> 16) & 0xFF;
    chk ^= (pkg.ip >>  8) & 0xFF;
    chk ^= (pkg.ip >>  0) & 0xFF;
    chk ^= (pkg.len >> 8) & 0xFF;
    chk ^= (pkg.len >> 0) & 0xFF;
    for(uint8_t i = 0; i < pkg.len; i++) {
        chk ^= pkg.payload[i];
    }
    return chk;
}

uint8_t calcChecksum(dataPacket_t pkg) {
    uint8_t chk = 0;
    chk ^= pkg.cmd;
    chk ^= pkg.packetNum;
    for(uint8_t i = 0; i < DATA_PAYLOAD_LEN; i++) {
        chk ^= pkg.payload[i];
    }
    return chk;
}

void sendUDPChunk() {
    dataPacket_t pkg;
    pkg.cmd = CMD_DATA;
    pkg.packetNum = txOffset / DATA_PAYLOAD_LEN;
    uint8_t len = DATA_PAYLOAD_LEN;
    if(txOffset + DATA_PAYLOAD_LEN > txLen) {
        len = txLen - txOffset;
        txLen = 0; // signal last packet send
    }
    uint8_t buf[len];
    memcpy(buf, txBuf, len);
    pkg.payload = buf;

    pkg.checksum = calcChecksum(pkg);
    bool success = rf.write(&pkg, len + DATA_HEADER_LEN); // blocking for now, TODO see if it's okay

    // if first transfer failed, don't bother with the rest of it
    if(!success) {
        txLen = 0;
    }
}

void sendUDP(IPAddress ip, uint16_t port, uint8_t* payload, uint16_t size) {
    initPacket_t pkg;
    pkg.cmd = CMD_INIT;
    pkg.ip = ip;
    pkg.port = port;

    if(size <= INIT_PAYLOAD_LEN) {
        pkg.len = size;
        pkg.payload = payload;
    }
    else {
        // copy start of payload into init packet
        uint8_t buf[INIT_PAYLOAD_LEN];
        memcpy(buf, payload, sizeof(buf));
        pkg.payload = buf;
        pkg.len = INIT_PAYLOAD_LEN;
        // copy remaining payload into buffer to be sent later
        txOffset = 0;
        txLen = size - INIT_PAYLOAD_LEN;
        memcpy(txBuf, payload + INIT_PAYLOAD_LEN, txLen);
    }

    pkg.checksum = calcChecksum(pkg);
    bool success = rf.write(&pkg, pkg.len + INIT_HEADER_LEN); // blocking for now, TODO see if it's okay
    
    // if first transfer failed, don't bother with the rest of it
    if(!success) {
        txLen = 0;
    }
}

void sendUDP(IPAddress ip, uint16_t port, String payload) {
    sendUDP(ip, port, (uint8_t*)payload.c_str(), payload.length() + 1);
}


void initNRF() {
    rf.begin();
    rf.setPALevel(RF24_PA_LOW);
    rf.openWritingPipe((uint8_t *) NRF_ADDRESS);
}

void loopNRF() {
    if(txLen > 0) {
        sendUDPChunk();
    }
}