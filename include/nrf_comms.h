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
    hudQueue->send(1 << HUDCommand1::CommandBit::HEARTBEAT);
  }

  if (type == (int)Packet::HUD)
  {
    using namespace HUDCommand1;
    uint16_t command = controllerClient.read();

    if (is<CYCLE_BRIGHTNESS>(command))
    {
      ledDisplay->cycleBrightness();
      hudQueue->send(1 << TWO_FLASHES | 1 << CommandBit::SLOW | 1 << CommandBit::BLUE);
    }
    else
    {
      hudQueue->send(command);
    }
  }
  else
  {
    Serial.printf("WARNING: Packet not supported: %s\n", Packet::getType(type));
  }
}
//------------------------------------------------------------------
void controllerConnectedChange()
{
  Serial.printf(CLIENT_CONNECT_CHANGE_FORMAT_STRING,
                controllerClient.connected()
                    ? "<----->"
                    : "-- | --");
  // if (controllerClient.connected())
  //   hudQueue->send(1 << HUDCommand1::HEARTBEAT);
}

void printRxPacket(uint16_t command)
{
  if (PRINT_PACKET_RX)
    Serial.printf(RX_PACKET_FORMAT_STRING,
                  HUDCommand1::getMode(command, "printRxPacket"),
                  HUDCommand1::getColour(command),
                  HUDCommand1::getSpeed(command));
}