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
    hudQueue->send(HUD::HEARTBEAT);
  }

  if (type == (int)Packet::HUD)
  {
    using namespace HUD;
    Command command = controllerClient.read();

    if (command.is<HUD::CYCLE_BRIGHTNESS>())
    {
      ledDisplay->cycleBrightness();
      hudQueue->send(HUD::TWO_FLASHES | HUD::SLOW | HUD::BLUE);
    }
    else
    {
      hudQueue->send(command.get());
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
  //   hudQueue->send(1 << HUD::HEARTBEAT);
}

void printRxPacket(HUD::Command command)
{
  if (PRINT_PACKET_RX)
    Serial.printf(RX_PACKET_FORMAT_STRING,
                  command.getMode("printRxPacket"),
                  command.getColour(),
                  command.getSpeed());
}