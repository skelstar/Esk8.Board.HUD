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
    hudStateQueue->send(HUDCommand::MODE_NONE);
  }

  if (type == (int)Packet::HUD)
  {
    ControllerCommand command = readFromNrf<ControllerCommand>();

    ledDisplay->setColour(command.colour);
    hudStateQueue->send(command.mode);
    if (PRINT_PACKET_RX)
      Serial.printf("-->rx: %s:%s\n",
                    HUDCommand::modeNames[(int)command.mode],
                    HUDCommand::colourName[(int)command.colour]);
  }
  else
  {
    Serial.printf("Packet not supported: %s\n", Packet::names[(int)type]);
  }
}
//------------------------------------------------------------------

bool sendActionToController(HUDAction::Event d)
{
  uint8_t action = (int)d;
  return nrf24.send(COMMS_CONTROLLER, Packet::HUD, &action, sizeof(uint8_t));
}

bool sendPacket(uint8_t *d, uint8_t len, uint8_t packetType)
{
  return nrf24.send(COMMS_CONTROLLER, packetType, d, len);
}
//------------------------------------------------------------------

template <typename T>
T readFromNrf()
{
  T ev;
  uint8_t buff[sizeof(T)];
  nrf24.read_into(buff, sizeof(T));
  memcpy(&ev, &buff, sizeof(T));
  return ev;
}
//------------------------------------------------------------------
