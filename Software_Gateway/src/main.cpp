#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Ethernet.h>
#include <RF24Mesh.h>
#include <RF24Network.h>

#undef Ethernet
#undef EthernetClient
#undef EthernetServer
#undef EthernetUDP

#include <Ethernet.h>
#include "eth.h"

#define SPI2_MOSI   PB15
#define SPI2_MISO   PB14
#define SPI2_SCK    PB13
#define SPI2_CS     PB12

#define SPI1_MOSI   PA7
#define SPI1_MISO   PA6
#define SPI1_SCK    PA5
#define SPI1_CS     PA4

#define NRF_CS_PIN  PA4
#define NRF_CE_PIN  PB0

SPIClass nrfSPI(SPI1_MOSI, SPI1_MISO, SPI1_SCK); // set this in RF24_config.h
RF24 radio(NRF_CE_PIN, NRF_CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
RF24EthernetClass RF24Ethernet(radio, network, mesh);

EthernetUDP udpClient;

#define DEBUG Serial

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

bool connectEthernet() {
    DEBUG.println("Initialize Ethernet");

    bool success= true;

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

void setup() {
    Serial.begin(115200);
    Serial.println("Gateway starting...");
    Serial.println(udpHdrLen);

    SPI.setMOSI(SPI2_MOSI);
    SPI.setMISO(SPI2_MISO);
    SPI.setSCLK(SPI2_SCK);

    generateMAC(macAddr);
    Ethernet.init(SPI2_CS);
    connectEthernet();
    udpClient.begin(1337);
    udpClient.beginPacket(IPAddress(192,168,137,1), 1234);
    udpClient.write("init", 5);
    udpClient.endPacket();

    nrfSPI.begin();
    mesh.setNodeID(0);
    mesh.begin();
    radio.printDetails();
}

#define IPV4_IDX_PROTOCOL 0x09
#define IPV4_PROTOCOL_UDP 0x11

void rf24ToEthernet() {
    RF24NetworkFrame *frame = network.frag_ptr;
    size_t size = frame->message_size;
    uint8_t *payload = frame->message_buffer;
    for(int i = 0; i < size; i++) {
        char buf[5];
        snprintf(buf, 10, "%02X ", payload[i]);
        Serial.print(buf);
    }
    Serial.println();

    delay(10);

    ipHdr_t *ipPacket = (ipHdr_t *)payload;
    // Serial.println(ipPacket->version);
    // Serial.println(ipPacket->ihl);
    // Serial.println(ipPacket->length);
    // Serial.println(ipPacket->protocol);
    // Serial.println(ipPacket->dstIp, 16);

    if(ipPacket->ihl != 5) {
        Serial.print("ERROR: IPv4 Option fields have not been tested yet.");
        return;
    }

    if(ipPacket->protocol == IPV4_PROTOCOL_UDP) {
        udpHdr_t* udpPacket = (udpHdr_t *)(payload + ipPacket->ihl * 4);
        delay(1);
        uint8_t *ip = (uint8_t*)&ipPacket->dstIp;
        delay(1);
        char buf[100];
        sprintf(buf, "-> %d.%d.%d.%d:%d (%d bytes) - ", ip[0], ip[1], ip[2], ip[3], (uint16_t)udpPacket->dstPort, (uint16_t)udpPacket->length - udpHdrLen);
        Serial.print(buf);
        Serial.write((uint8_t *)&udpPacket->payload, udpPacket->length - udpHdrLen);
        // delay(10);
        // Serial.print(*(uint32_t*)&udpPacket->payload);
        Serial.println();


        udpClient.beginPacket(IPAddress(ip), udpPacket->dstPort);
        udpClient.write((uint8_t *)&udpPacket->payload, udpPacket->length - udpHdrLen);
        udpClient.endPacket();
    }
    // uint8_t protocol = payload[IPV4_IDX_PROTOCOL];
    // if(protocol == IPV4_PROTOCOL_UDP) {

    // }
}

void loop() {
    // Provide RF24Network addresses to connecting & reconnecting nodes
    if(millis() > 2000){
        mesh.DHCP();
    }

    //Ensure any incoming user payloads are read from the buffer
    while(network.available()){
        RF24NetworkHeader header;
        network.read(header,0,0);    
    }

    // Handle external (TCP) data
    // Note: If not utilizing RF24Network payloads directly, users can edit the RF24Network_config.h file
    // and uncomment #define DISABLE_USER_PAYLOADS. This can save a few hundred bytes of RAM.

    if(mesh.update() == EXTERNAL_DATA_TYPE) {
        rf24ToEthernet();
    }

    // TODO: ethernet to rf24
}