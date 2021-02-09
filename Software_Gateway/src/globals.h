#pragma once

#define DEBUG Serial

#define ETH_MOSI    PB15 // SPI2_MOSI 
#define ETH_MISO    PB14 // SPI2_MISO 
#define ETH_SCK     PB13 // SPI2_SCK  
#define ETH_CS      PB12 // SPI2_CS   

#define NRF_MOSI    PA7 // SPI1_MOSI
#define NRF_MISO    PA6 // SPI1_MISO
#define NRF_SCK     PA5 // SPI1_SCK 
#define NRF_CS      PA4 // SPI1_CS  
#define NRF_CE_PIN  PB1