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

    // char b[200];
    // IPAddress ip = fromIp;
    // snprintf(b, 200, "Received packet - length: %d, from: %d.%d.%d.%d:%d, payload: %-128s", payloadSize, ip[0], ip[1], ip[2], ip[3], fromPort, payload);
    // DEBUG.println(b);

    uint8_t buf[MAX_PAYLOAD_SIZE];
    dataToRemote_t *data = (dataToRemote_t *)buf;
    payloadSize = min((unsigned int)payloadSize, sizeof(buf) - sizeof(dataToRemote_t)); // cap payloadSize

    data->fromIp = fromIp;
    data->fromPort = fromPort;
    data->payloadLen = payloadSize;
    memcpy(data->payload, payload, payloadSize);

    // broadcast IP packet to all nodes (for now)
    for(int i = 0; i < mesh.addrListTop; i++) {
        RF24Mesh::addrListStruct node = mesh.addrList[i];
        if(node.nodeID != 0) {
            DEBUG.println("Forwarding to node " + String(node.nodeID) + "/" + String(node.address) + " (ID/Address)");

            RF24NetworkHeader header(node.address);
            network.write(header, data, payloadSize + sizeof(dataToRemote_t));
            
        }
    }
}