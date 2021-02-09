#include "nrf.h"

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Ethernet.h>
#include <RF24Mesh.h>
#include <RF24Network.h>
#include "globals.h"

// SPIClass nrfSPI(NRF_MOSI, NRF_MISO, NRF_SCK); // define _SPI to nrfSPI in RF24_config.h
RF24 radio(NRF_CE_PIN, NRF_CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
RF24EthernetClass RF24Ethernet(radio, network, mesh);

RF24UDP udpClient; // Set UIP_CONF_UDP to 1 in RF24Ethernet/uip-conf.h

RF24Server server(1337);

void nrfInit() {
    DEBUG.println("Radio Init...");
    Ethernet.begin(IPAddress(10,10,2,4));
    bool success = false;
    // while(!success) {
        success = mesh.begin(97, RF24_250KBPS);
        if(!success) {
            DEBUG.println("*ERR Mesh connection failed!");
        }
    // }
    Ethernet.set_gateway(IPAddress(10,10,2,2));
    // radio.printDetails();
    DEBUG.print("NRF Power Level: ");
    DEBUG.print(String(radio.getPALevel())); // 0-3 = MIN, LOW, HIGH, MAX
    DEBUG.print(" / CH: x");
    DEBUG.println(String(radio.getChannel(), 16));
    DEBUG.println("Radio Init done!");
    radio.printDetails();

    server.begin();
}

uint32_t meshTimer;

void nrfLoop() {
    // Every 30 seconds, test mesh connectivity
    if(millis() - meshTimer > 5000) { 
        meshTimer = millis();
        if (!mesh.checkConnection() ) {
            uint16_t addr = mesh.renewAddress(100); // 100ms timeout for mesh renewal (original: 7500)
            if(!addr) {
                DEBUG.println("Mesh connection attempt failed");
                meshTimer = 0; // immediately try again
            }
        }
    }

    if(EthernetClient client = server.available()) {
        DEBUG.println("Got something");
        while(client.available()) {
            DEBUG.print(String((char)client.read()));
        }
        DEBUG.println("");
    }

    if(mesh.update() == EXTERNAL_DATA_TYPE) {
        RF24NetworkFrame *frame = network.frag_ptr;
        size_t size = frame->message_size;
        uint8_t *payload = frame->message_buffer;

        // print raw received bytes
        for(size_t i = 0; i < size; i++) {
            char buf[5];
            snprintf(buf, 10, "%02X ", payload[i]);
            DEBUG.print(buf);
        }
        DEBUG.println("");
    }
}
void nrfSendUDP(IPAddress ip, uint16_t port, uint8_t* payload, uint16_t size) {
    // DEBUG.print("Snd... ");
    // note: not a real indicator of success when mesh connection not established
    int success = udpClient.beginPacket(ip, port); 
    if(success) {
        udpClient.write(payload, size);
        udpClient.endPacket();
        // DEBUG.println("Success");
    }
    else {
        // DEBUG.println("Error sending UDP packet");
    }
}

void nrfSendUDP(IPAddress ip, uint16_t port, String payload) {
    nrfSendUDP(ip, port, (uint8_t *)payload.c_str(), payload.length());
}

buttonAction_t btnAction[BTN_NUM] = {BUTTON_MAPPING};

void nrfSendButtonAction(uint8_t btnId) {
    if(btnId < BTN_NUM) {
        buttonAction_t action = btnAction[btnId];
        DEBUG.println(action.payload);
        nrfSendUDP(action.ip, action.port, action.payload);
    }
}