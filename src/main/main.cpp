#ifdef DEBUG_SERIAL
#define DEBUG_OUT Serial
#endif
#define PRINTSTREAM_FALLBACK
#include <Arduino_Helpers.h>
#include "Debug.hpp"

#include <Arduino.h>
#include <VescData.h>
#include <elapsedMillis.h>

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
    Serial.printf("nrf24 update()\n");
  }

  delay(50);
}