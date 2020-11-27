#ifndef ARDUINO_H
#include <Arduino.h>
#endif

//------------------------------------------------------------------
void packetAvailable_cb(uint16_t from_id, uint8_t type)
{
  controller.sinceLastPacket = 0;

  DEBUG("packetAvailable_cb");

  if (false == controller.connected)
  {
    controller.connected = true;
    // hudStateQueue->send(HUD_CMD_HEARTBEAT);
  }

  if (type == (int)PacketType::HUD)
  {
    HUDData data;
    uint8_t buff[sizeof(HUDData)];
    nrf24.read_into(buff, sizeof(HUDData));
    memcpy(&data, &buff, sizeof(HUDData));

    // if (!data.event == HUD_CMD_HEARTBEAT)
    Serial.printf("packetAvailable_cb: %s\n", hudCommandNames[(int)data.event]);

    hudStateQueue->send(data.event);
  }
  else
  {
    Serial.printf("Packet not supported: %s\n", packetTypeNames[(int)type]);
  }
}
//------------------------------------------------------------------

bool sendActionToController(HudActionEvent d)
{
  uint8_t action = (int)d;
  return nrf24.send(COMMS_CONTROLLER, HUD, &action, sizeof(uint8_t));
}

bool sendPacket(uint8_t *d, uint8_t len, uint8_t packetType)
{
  return nrf24.send(COMMS_CONTROLLER, packetType, d, len);
}
//------------------------------------------------------------------