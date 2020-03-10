#include "nrf.h"
#include "config.h"

RF24 rf(NRF_CE_PIN, NRF_CS_PIN);

uint8_t txBuf[MAX_PACKET_LEN];
uint16_t txLen = 0, txOffset = 0;
uint8_t rxBuf[MAX_PACKET_LEN];
uint16_t rxLen = 0, rxOffset = 0, rxCounter = 0;

void (*_callback)(uint8_t*, uint16_t) = NULL;

void setNRFCallback(void (*cb)(uint8_t*, uint16_t)) { 
    _callback = cb; 
}

uint8_t calcChecksum(packet_t pkg) {
    uint8_t chk = 0;
    chk ^= pkg.cmd.raw;

    switch(pkg.cmd.type) {
        case CMD_INIT:
            chk ^= (pkg.initPayload.ip >> 24) & 0xFF;
            chk ^= (pkg.initPayload.ip >> 16) & 0xFF;
            chk ^= (pkg.initPayload.ip >>  8) & 0xFF;
            chk ^= (pkg.initPayload.ip >>  0) & 0xFF;
            chk ^= pkg.initPayload.dataLen;
            for(uint8_t i = 0; i < pkg.initPayload.dataLen; i++) {
                chk ^= pkg.initPayload.data[i];
            }
            break;

        case CMD_DATA:
            chk ^= pkg.dataPayload.packetNum;
            chk ^= pkg.dataPayload.dataLen;
            for(uint8_t i = 0; i < pkg.dataPayload.dataLen; i++) {
                chk ^= pkg.dataPayload.data[i];
            }
            break;
    }

    return chk;
}

void sendUDPChunk() {
    packet_t pkg;
    pkg.cmd.type = CMD_DATA;
    pkg.dataPayload.packetNum = txOffset / DATA_PAYLOAD_LEN;
    
    pkg.dataPayload.dataLen = DATA_PAYLOAD_LEN;
    if(txOffset + DATA_PAYLOAD_LEN > txLen) {
        pkg.dataPayload.dataLen = txLen - txOffset;
        pkg.cmd.lastPacket = true;
        txLen = 0; // signal last packet send
    }
    uint8_t buf[pkg.dataPayload.dataLen];
    memcpy(buf, txBuf, pkg.dataPayload.dataLen);
    pkg.dataPayload.data = buf;
    txOffset += pkg.dataPayload.dataLen;

    pkg.checksum = calcChecksum(pkg);
    bool success = rf.write(&pkg, pkg.dataPayload.dataLen + DATA_HEADER_LEN); // blocking for now, TODO see if it's okay

    // if first transfer failed, don't bother with the rest of it
    if(!success) {
        txLen = 0;
    }
}

void sendUDP(IPAddress ip, uint16_t port, uint8_t* payload, uint16_t size) {
    packet_t pkg;
    pkg.cmd.type = CMD_INIT;
    pkg.initPayload.ip = ip;
    pkg.initPayload.port = port;

    if(size <= INIT_PAYLOAD_LEN) { 
        pkg.initPayload.dataLen = size;
        pkg.initPayload.data = payload;
        pkg.cmd.lastPacket = true;
    }
    else {
        // copy start of payload into init packet
        uint8_t buf[INIT_PAYLOAD_LEN];
        memcpy(buf, payload, sizeof(buf));
        pkg.initPayload.data = buf;
        pkg.initPayload.dataLen = INIT_PAYLOAD_LEN;
        // copy remaining payload into buffer to be sent later
        txOffset = 0;
        txLen = size - INIT_PAYLOAD_LEN;
        memcpy(txBuf, payload + INIT_PAYLOAD_LEN, txLen);
    }

    pkg.checksum = calcChecksum(pkg);
    bool success = rf.write(&pkg, pkg.initPayload.dataLen + INIT_HEADER_LEN); // blocking for now, TODO see if it's okay
    
    // if first transfer failed, don't bother with the rest of it
    if(!success) {
        txLen = 0;
    }
}

void sendUDP(IPAddress ip, uint16_t port, String payload) {
    sendUDP(ip, port, (uint8_t*)payload.c_str(), payload.length() + 1);
}

void handlePacket(uint8_t *buf, uint16_t size) {
    packet_t pkg = *(packet_t*)buf;

    if(pkg.checksum != calcChecksum(pkg)) {
        return;
    }
    
    switch(pkg.cmd.type) {
        case CMD_INIT: 
            if(rxLen == 0) { // check that buffer is empty TODO: maybe multibuffer
                rxLen = pkg.initPayload.dataLen;
                if(rxLen <= MAX_PACKET_LEN) {
                    memcpy(rxBuf, pkg.initPayload.data, rxLen);
                }
                if(pkg.cmd.lastPacket) {
                    // handle data and set rxLen back to 0 when finished
                    if(_callback) {
                        _callback(rxBuf, rxLen);
                    }
                    rxLen = 0;
                }
            }
            break;
        case CMD_DATA: {
            uint8_t len = pkg.dataPayload.dataLen;
            // simply write data into buffer at offset calculated by packetNumber
            // don't do any integrity checks as of right now, TODO
            uint8_t offset = INIT_PAYLOAD_LEN + pkg.dataPayload.packetNum * DATA_PAYLOAD_LEN;
            if(offset + len <= MAX_PACKET_LEN) {
                memcpy(rxBuf + offset, pkg.initPayload.data, len);
                rxLen += len;
            }
            if(pkg.cmd.lastPacket) {
                if(_callback) {
                    _callback(rxBuf, rxLen);
                }
                rxLen = 0;
            }
            break;
        }
    }
}

void initNRF() {
    rf.begin();
    rf.setPALevel(RF24_PA_LOW);
    rf.openWritingPipe((uint8_t *) NRF_GATEWAY_ADDRESS);
    rf.openReadingPipe(1, (uint8_t *) NRF_REMOTE_ADDRESS);
    rf.startListening();
}

void loopNRF() {
    if(txLen > 0) {
        sendUDPChunk();
    }

    if(rf.available()) {
        uint8_t buf[rf.getPayloadSize()];
        rf.read(buf, rf.getPayloadSize());
        handlePacket(buf, rf.getPayloadSize());
    }
}