#ifndef ARDUINO_H
#include <Arduino.h>
#endif

//------------------------------------------------------------------
void packetAvailable_cb(uint16_t from_id, uint8_t type)
{
  controller.sinceLastPacket = 0;

  if (type == (int)PacketType::HUD)
  {
    uint8_t buff[sizeof(HUDData)];
    nrf24.read_into(buff, sizeof(HUDData));
    memcpy(&hudData, &buff, sizeof(HUDData));

    Serial.printf("Received: %s id: %d\n", packetNames[(int)type], hudData.id);
    ledsQueueManager->send(hudData.event);
  }
  else
  {
    Serial.printf("Packet not supported: %s\n", packetNames[(int)type]);
  }
}
//------------------------------------------------------------------

bool sendPacket(uint8_t *d, uint8_t len, uint8_t packetType)
{
  bool sent = nrf24.send(COMMS_CONTROLLER, packetType, d, len);

  return sent;
}
//------------------------------------------------------------------

void sendPacketToController()
{
  // sendPacket(bs, sizeof(ControllerData), PacketType::CONTROL);
}
//------------------------------------------------------------------

bool controllerTimedOut()
{
  unsigned long timeout = CONTROLLER_HEARTBEAT_INTERVAL;
  return controller.sinceLastPacket > (timeout + 100);
}

//------------------------------------------------------------------
