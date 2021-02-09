#include "nrf.h"

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Ethernet.h>
#include <RF24Mesh.h>
#include <RF24Network.h>
#include "globals.h"
#include "buttonMatrix.h"

// SPIClass nrfSPI(NRF_MOSI, NRF_MISO, NRF_SCK); // define _SPI to nrfSPI in RF24_config.h
RF24 radio(NRF_CE_PIN, NRF_CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
RF24EthernetClass RF24Ethernet(radio, network, mesh);

RF24UDP udpClient; // Set UIP_CONF_UDP to 1 in RF24Ethernet/uip-conf.h

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
}

void nrfParseIncomingPacket(dataToRemote_t *data) {
    char b[200];
    IPAddress ip = data->fromIp;
    snprintf(b, 200, "Received packet - length: %d, from: %d.%d.%d.%d:%d, payload: %-128s", data->payloadLen, ip[0], ip[1], ip[2], ip[3], data->fromPort, data->payload);
    // DEBUG.println(b);

    // make sure payload is a null terminated string
    data->payload[data->payloadLen] = 0;
    char *payload = (char *)data->payload;

    char *command;
    command = strtok(payload, ";"); // get first token
    char* param[4];
    uint8_t numParams = 0;
    for(int i = 0; i < sizeof(param); i++) {
        char* nextToken = strtok(NULL, ";"); // get next token
        if(nextToken != NULL) {
            param[i] = nextToken;
            numParams = i + 1;
        }
        else {
            break;
        }
    }

    String cmd = String(command);
    cmd.toLowerCase();

    if(cmd == "setled" && numParams >= 2) {
        uint8_t ledNum, state;
        sscanf(param[0], "%hhd", &ledNum);
        sscanf(param[1], "%hhd", &state);
        btnSetLed(ledNum, state);
    }
    else if(cmd == "clearleds") {
        for(int i = 0; i < BTN_NUM; i++) {
            btnSetLed(i, 0);
        }
    }
    else if(cmd == "print" && numParams >= 1) {
        dispPrint(param[0]);
    }

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

    // have to disable button multiplexing because for some reason network.update() leads to GPIO glitches??
    btnEnableMultiplexing(false);
    network.update();
    btnEnableMultiplexing(true);
    if(network.available()) {
        RF24NetworkHeader header;
        uint8_t buf[MAX_PAYLOAD_SIZE];
        network.read(header, buf, sizeof(buf));
        dataToRemote_t *data = (dataToRemote_t *)buf;

        nrfParseIncomingPacket(data);
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
        // DEBUG.println(action.payload);
        nrfSendUDP(action.ip, action.port, action.payload);
    }
}