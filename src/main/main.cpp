#ifdef DEBUG_SERIAL
#define DEBUG_OUT Serial
#endif
#define PRINTSTREAM_FALLBACK
#include <Arduino_Helpers.h>
#include "Debug.hpp"

#include <Arduino.h>
#include <VescData.h>
#include <elapsedMillis.h>

// #define SOFTSPI
// ESPmini pins
// #define SOFT_SPI_MISO_PIN 22 // Orange
// #define SOFT_SPI_MOSI_PIN 21 // Blue
// #define SOFT_SPI_SCK_PIN 16  // Yellow
// #define SPI_CE 5  // WHITE
// #define SPI_CS 23 // GREEN

// controller pins
// #define SOFT_SPI_MISO_PIN 12 // Orange
// #define SOFT_SPI_MOSI_PIN 13 // Blue
// #define SOFT_SPI_SCK_PIN 15  // Yellow
// #define SPI_CE 17
// #define SPI_CS 2

#include <RF24Network.h>
#include <NRF24L01Lib.h>

#define COMMS_BOARD 00
#define COMMS_CONTROLLER 01
#define COMMS_HUD 02

//------------------------------------------------------------------

VescData board_packet;

ControllerClass controller;

NRF24L01Lib nrf24;

RF24 radio(SPI_CE_PIN, SPI_CS_PIN);
RF24Network network(radio);

//------------------------------------------------------------------
void packet_available_cb(uint16_t from_id, uint8_t type)
{
  Serial.printf("rx from %d\n", from_id);
}
//------------------------------------------------------------------

void setup()
{
  delay(3000);
  Serial.begin(115200);
  Serial.printf("ready!\n");

  nrf24.begin(&radio, &network, COMMS_BOARD, packet_available_cb);

  // Serial.printf("MISO:%d MOSI:%d SCK:%d CE:%d CS:%d\n",
  //               SOFT_SPI_MISO_PIN,
  //               SOFT_SPI_MOSI_PIN,
  //               SOFT_SPI_SCK_PIN,
  //               SPI_CE_PIN,
  //               SPI_CS_PIN);

  // xTaskCreatePinnedToCore(
  //     footLightTask_0,
  //     "footLightTask_0",
  //     /*stack size*/ 10000,
  //     /*params*/ NULL,
  //     /*priority*/ 3,
  //     /*handle*/ NULL,
  //     /*core*/ 0);
  // xFootLightEventQueue = xQueueCreate(1, sizeof(FootLightEvent));
}

elapsedMillis sincePulse, sinceCheckRF24;

void loop()
{

  if (sincePulse > 1000)
  {
    sincePulse = 0;
    Serial.printf("pulse\n");
  }

  if (sinceCheckRF24 > 500)
  {
    sinceCheckRF24 = 0;
    nrf24.update();
  }

  delay(50);
}