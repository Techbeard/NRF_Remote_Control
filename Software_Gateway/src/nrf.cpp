#include "nrf.h"

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Ethernet.h>
#include <RF24Mesh.h>
#include <RF24Network.h>
#include "globals.h"
#include "eth.h"
#include "ip.h"

// undefine RF24 aliases for default ethernet classes, we still need them
#undef Ethernet
#undef EthernetClient
#undef EthernetServer
#undef EthernetUDP
// #undef DNSClient

SPIClass nrfSPI(NRF_MOSI, NRF_MISO, NRF_SCK); // define _SPI to nrfSPI in RF24_config.h
RF24 radio(NRF_CE_PIN, NRF_CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
RF24EthernetClass RF24Ethernet(radio, network, mesh);

void nrfHandleData() {
    RF24NetworkFrame *frame = network.frag_ptr;
    size_t size = frame->message_size;
    uint8_t *payload = frame->message_buffer;

    // print raw received bytes
    for(size_t i = 0; i < size; i++) {
        char buf[5];
        snprintf(buf, 10, "%02X ", payload[i]);
        DEBUG.print(buf);
    }
    DEBUG.println();

    ethForwardIpPacket(payload, size);
}

void nrfInit() {
    nrfSPI.begin();
    mesh.setNodeID(0);
    mesh.begin(97, RF24_250KBPS);
    radio.printDetails();
}

void nrfLoop() {
    // Provide RF24Network addresses to connecting & reconnecting nodes
    if(millis() > 2000){
        mesh.DHCP();
    }

    // Ensure any incoming user payloads are read from the buffer
    while(network.available()){
        RF24NetworkHeader header;
        network.read(header,0,0);    
    }
        
    // Handle external (TCP/UDP) data
    if(mesh.update() == EXTERNAL_DATA_TYPE) {
        nrfHandleData();
    }
}

void nrfSendIpPacketToRemote(IPAddress fromIp, uint16_t fromPort, char *payload, int payloadSize) {

    char buf[200];
    IPAddress ip = fromIp;
    snprintf(buf, 200, "Received packet - length: %d, from: %d.%d.%d.%d:%d, payload: %-128s", payloadSize, ip[0], ip[1], ip[2], ip[3], fromPort, payload);
    DEBUG.println(buf);

    // broadcast IP packet to all nodes (for now)
    for(int i = 0; i < mesh.addrListTop; i++) {
        RF24Mesh::addrListStruct node = mesh.addrList[i];
        if(node.nodeID != 0) {
            DEBUG.println("Forwarding to node " + String(node.nodeID) + "/" + String(node.address) + " (ID/Address)");

            // cobble together basic UDP packet for the remote to parse
            uint8_t ipBuf[MAX_PAYLOAD_SIZE] = {0};
            ipPkt_t *ipPacket = (ipPkt_t *)ipBuf;
            udpPkt_t *udpPacket = (udpPkt_t *)ipPacket->payload;

            ipPacket->version = 4;
            ipPacket->ihl = 5;
            ipPacket->length = ipHeaderLen + udpHdrLen + payloadSize;
            ipPacket->identification = 1;
            ipPacket->ttl = 64;
            ipPacket->protocol = IPV4_PROTOCOL_UDP;
            ipPacket->headerChecksum = 0;
            ipPacket->srcIp = htonl(fromIp);
            ipPacket->dstIp = htonl(IPAddress(10,10,2,4));
            udpPacket->srcPort = fromPort;
            udpPacket->dstPort = UDP_LISTEN_PORT;
            udpPacket->length = udpHdrLen + payloadSize;
            udpPacket->checksum = 0;
            memcpy(udpPacket->payload, payload, min(payloadSize, MAX_PAYLOAD_SIZE - ipHeaderLen - udpHdrLen));
            
            /* checksum not actually needed
            uint32_t ipChecksum = 0;
            for(int i = 0; i < ipHeaderLen / 2; i++) {
                ipChecksum += ipBuf[i*2] << 8 | ipBuf[i*2+1];
            }
            ipPacket->headerChecksum = ~(((ipChecksum >> 16) & 0xFFFF) + (ipChecksum & 0xFFFF));

            ipChecksum = 0;
            for(int i = 0; i < ipHeaderLen / 2; i++) {
                ipChecksum += ipBuf[i*2] << 8 | ipBuf[i*2+1];
            }
            DEBUG.println(ipChecksum, 16);
            DEBUG.println((ipChecksum >> 16) + (ipChecksum & 0xFFFF), 16);

            // https://www.securitynik.com/2015/08/calculating-udp-checksum-with-taste-of.html
            uint32_t udpChecksum = ipPacket->srcIp & 0xFFFF;
            udpChecksum += ipPacket->srcIp >> 16;
            udpChecksum += ipPacket->dstIp & 0xFFFF;
            udpChecksum += ipPacket->dstIp >> 16;
            udpChecksum += ipPacket->protocol;
            udpChecksum += udpPacket->length * 2; // for some reason this gets added twice
            udpChecksum += udpPacket->srcPort;
            udpChecksum += udpPacket->dstPort;
            udpChecksum += udpPacket->length;
            udpPacket->checksum = ~((udpChecksum >> 16) + (udpChecksum & 0xFFFF));*/


            RF24NetworkHeader header(node.address, EXTERNAL_DATA_TYPE);
            // network.write(header, payload, payloadSize);
            network.write(header, ipBuf, ipPacket->length);
            
        }
    }
}