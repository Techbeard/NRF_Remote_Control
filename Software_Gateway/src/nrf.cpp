#include "nrf.h"

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Ethernet.h>
#include <RF24Mesh.h>
#include <RF24Network.h>
#include "globals.h"
#include "eth.h"

// undefine RF24 aliases for default ethernet classes, we still need them
#undef Ethernet
#undef EthernetClient
#undef EthernetServer
#undef EthernetUDP

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
    for(int i = 0; i < size; i++) {
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
    mesh.begin();
    // radio.printDetails();
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

    // TODO: ethernet to rf24
}