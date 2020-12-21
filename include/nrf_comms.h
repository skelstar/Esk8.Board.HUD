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
    Instruction instruction = controllerClient.read();

    if (instruction.is<HUD::CYCLE_BRIGHTNESS>())
    {
      ledDisplay->cycleBrightness();
      hudQueue->send(HUD::FLASH | HUD::BLUE | HUD::SLOW);
    }
    else
    {
      hudQueue->send(instruction.get());
    }
  }
  else
  {
    Serial.printf("WARNING: Packet not supported: %s\n", Packet::getType(type));
  }
}
//------------------------------------------------------------------
