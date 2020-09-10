#include "eth.h"

#include <SPI.h>
#include <Ethernet.h>
#include "ip.h"
#include "globals.h"

EthernetUDP udpClient;

uint8_t macAddr[6];
char macStr[13];

static inline uint64_t mix(uint64_t h) {
    h ^= h >> 23;
    h *= 0x2127599bf4325c37ULL;
    h ^= h >> 47;
    //
    return h;
}

// hash function copied from https://stackoverflow.com/a/47895889
uint64_t fastHash64(const void * buf, size_t len, uint64_t seed) {
    const uint64_t m = 0x880355f21e6d1965ULL;
    const uint64_t * pos = (const uint64_t*)buf;
    const uint64_t * end = pos + (len / 8);
    const unsigned char * pos2;
    uint64_t h = seed ^ (len * m);
    uint64_t v;

    while(pos != end)
    {
        v  = *pos++;
        h ^= mix(v);
        h *= m;
    }

    pos2 = (const unsigned char*)pos;
    v = 0;

    switch(len & 7)
    {
        case 7: v ^= (uint64_t)pos2[6] << 48;
        case 6: v ^= (uint64_t)pos2[5] << 40;
        case 5: v ^= (uint64_t)pos2[4] << 32;
        case 4: v ^= (uint64_t)pos2[3] << 24;
        case 3: v ^= (uint64_t)pos2[2] << 16;
        case 2: v ^= (uint64_t)pos2[1] << 8;
        case 1: v ^= (uint64_t)pos2[0];
                h ^= mix(v);
                h *= m;
    }

    return mix(h);
}

void generateMAC(uint8_t *macArray) {

    // uint32_t uid[3] = {HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2()};
    uint64_t hash = fastHash64((uint8_t *)UID_BASE, 12, 0x421337f00beefULL);
    memcpy(macArray, &hash, 6);
    macArray[0] = 0x42; // set first byte to predefined value, where private bit is set

    snprintf(macStr, 13, "%02X%02X%02X%02X%02X%02X", macArray[0], macArray[1], macArray[2], macArray[3], macArray[4], macArray[5]);
}

bool ethConnect() {
    DEBUG.println("Initialize Ethernet");

    bool success = true;

    if (Ethernet.begin(macAddr, 10000) == 0) {
        DEBUG.println("Failed to configure Ethernet using DHCP");
        success = false;
    }
    // Ethernet.begin(macAddr, IPAddress(_config.ip));

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        DEBUG.println("Ethernet module was not found.  Sorry, can't run without hardware. :(");
        delay(5000);
        HAL_NVIC_SystemReset();
    } else if (Ethernet.linkStatus() == LinkOFF) {
        DEBUG.println("Ethernet cable is not connected.");
        success = false;
    }
    
    if(success) {
        // print your local IP address:
        DEBUG.print("My IP address: ");
        DEBUG.println(Ethernet.localIP());
    }
    return success;
}

void ethInit() {
    SPI.setMOSI(ETH_MOSI);
    SPI.setMISO(ETH_MISO);
    SPI.setSCLK(ETH_SCK);

    generateMAC(macAddr);
    Ethernet.init(ETH_CS);
    ethConnect();
    udpClient.begin(1337);
}

void ethForwardIpPacket(uint8_t *rawIpBytes, size_t size) {
    ipPkt_t *ipPacket = (ipPkt_t *)rawIpBytes;
    
    if(ipPacket->protocol == IPV4_PROTOCOL_UDP) {
        // udpPkt_t *udpPacket = (udpPkt_t *)&ipPacket->payload;
        udpPkt_t *udpPacket = (udpPkt_t *)(rawIpBytes + ipPacket->ihl * 4);
        uint8_t *ip = (uint8_t *)&ipPacket->dstIp;
        uint16_t udpPayloadLen = udpPacket->length - udpHdrLen;
        
        // print parsed packet
        char buf[100];
        sprintf(buf, "-> %d.%d.%d.%d:%d (%d bytes) - ", ip[0], ip[1], ip[2], ip[3], (uint16_t)udpPacket->dstPort, (uint16_t)udpPacket->length - udpHdrLen);
        DEBUG.print(buf);
        DEBUG.write(udpPacket->payload, udpPayloadLen);
        DEBUG.println();


        udpClient.beginPacket(IPAddress(ip), udpPacket->dstPort);
        udpClient.write(udpPacket->payload, udpPayloadLen);
        udpClient.endPacket();
    }
}