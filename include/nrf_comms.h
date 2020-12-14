#ifndef ARDUINO_H
#include <Arduino.h>
#endif

template <typename T>
T readFromNrf();

//------------------------------------------------------------------
void packetAvailable_cb(uint16_t from_id, uint8_t type)
{
  controller.sinceLastPacket = 0;

  // if controller was offline, send HEARTBEAT
  if (false == controller.connected)
  {
    controller.connected = true;
    hudQueue->send(HUDCommand1::CommandBit::HEARTBEAT);
  }

  if (type == (int)Packet::HUD)
  {
    using namespace HUDCommand1;
    uint16_t command = controllerClient.read();

    if (is<CYCLE_BRIGHTNESS>(command))
    {
      ledDisplay->cycleBrightness();
      hudQueue->send(1 << TWO_FLASHES | 1 << CommandBit::SLOW | 1 << BLUE);
    }
    else
    {
      ledDisplay->setColour(command);
      ledDisplay->setSpeed(command);
      int numFlashes = 1;
      if (is<TWO_FLASHES>(command))
        numFlashes = 2;
      else if (is<THREE_FLASHES>(command))
        numFlashes = 3;
      ledDisplay->numFlashes = numFlashes;

      Serial.printf("packetAvailable_cb: %s, mapped: %d\n", HUDCommand1::getMode(command), HUD::Triggers::mapToTriggers(command));

      hudQueue->send(command);
    }
  }
  else
  {
    Serial.printf("Packet not supported: %s\n", Packet::getType(type));
  }
}
//------------------------------------------------------------------
void controllerConnectedChange()
{
  Serial.printf(CLIENT_CONNECT_CHANGE_FORMAT_STRING,
                controllerClient.connected()
                    ? "<----->"
                    : "-- | --");
}

void printRxPacket(uint16_t command)
{
  if (PRINT_PACKET_RX)
    Serial.printf(RX_PACKET_FORMAT_STRING,
                  HUDCommand1::getMode(command),
                  HUDCommand1::getColour(command),
                  HUDCommand1::getSpeed(command));
}