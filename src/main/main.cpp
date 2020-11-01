#ifdef DEBUG_SERIAL
#define DEBUG_OUT Serial
#endif
#define PRINTSTREAM_FALLBACK
#include <Arduino_Helpers.h>
#include "Debug.hpp"

#include <Arduino.h>
#include <VescData.h>
#include <elapsedMillis.h>

#define COMMS_BOARD 00
#define COMMS_CONTROLLER 01
#define COMMS_HUD 02

//------------------------------------------------------------------

#include <HUDData.h>

bool packetReady;
HUDData hudData;

#include <bleNotify.h>

ControllerClass controller;

void setup()
{
  delay(3000);
  Serial.begin(115200);
  Serial.printf("Esk8.Board.HUD ready!\n");

  setupBLE();
}

elapsedMillis sincePulse, sinceSentToClient;

void loop()
{

  if (sincePulse > 1000)
  {
    sincePulse = 0;
    // Serial.printf("pulse\n");
  }

  if (sinceSentToClient > 3000 && clientConnected)
  {
    sinceSentToClient = 0;
    sendDataToClient(hudData);
    hudData.id++;
    Serial.printf("sending to client\n");
  }

  if (packetReady)
  {
    packetReady = false;
    Serial.printf("onWrite() got: %d, event: %s\n", hudData.id, eventToString(hudData.state));
    hudData.id++;
    sendDataToClient(hudData);
  }

  delay(50);
}