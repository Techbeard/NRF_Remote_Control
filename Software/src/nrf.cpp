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

RF24UDP udpClient; // Set UIP_CONF_UDP to 1 in uip-conf.h


void nrfInit() {
    DEBUG.println("Radio Init...");
    Ethernet.begin(IPAddress(10,10,2,4));
    bool success = mesh.begin(97, RF24_250KBPS);
    if(!success) {
        DEBUG.println("*ERR Mesh connection failed!");
    }
    Ethernet.set_gateway(IPAddress(10,10,2,2));
    // radio.printDetails();
    DEBUG.print("NRF Power Level: ");
    DEBUG.println(String(radio.getPALevel())); // 0-3 = MIN, LOW, HIGH, MAX
    DEBUG.println("Radio Init done!");
}

uint32_t meshTimer;

void nrfLoop() {
    // Every 30 seconds, test mesh connectivity
    if(millis() - meshTimer > 30000) { 
        meshTimer = millis();
        if (!mesh.checkConnection() ) {
            mesh.renewAddress();
        }
    }

}
void nrfSendUDP(IPAddress ip, uint16_t port, uint8_t* payload, uint16_t size) {
    udpClient.beginPacket(ip, port);
    udpClient.write(payload, size);
    udpClient.endPacket();
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