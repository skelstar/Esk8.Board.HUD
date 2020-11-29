#ifndef ARDUINO_H
#include <Arduino.h>
#endif

template <typename T>
T readFromNrf();
void printRxPacket(ControllerCommand command);

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

    if (command.mode == HUDCommand::CYCLE_BRIGHTNESS)
    {
      ledDisplay->cycleBrightness();
      ledDisplay->setColour(HUDCommand::BLUE);
      ledDisplay->setSpeed(HUDCommand::SLOW);
      hudStateQueue->send(HUDCommand::FLASH);
    }
    else
    {
      ledDisplay->setColour(command.colour);
      ledDisplay->setSpeed(command.speed);
      ledDisplay->numFlashes = command.number;

      hudStateQueue->send(command.mode);
    }
    if (PRINT_PACKET_RX)
      printRxPacket(command);
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
  if (PRINT_SEND_ACTION)
    Serial.printf("Sending action: %s\n", HUDAction::names[(int)d]);
  return nrf24.send(COMMS_CONTROLLER, Packet::HUD, &action, sizeof(uint8_t));
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
void printRxPacket(ControllerCommand command)
{
  Serial.printf("-->rx: %s|%s|%s|%d\n",
                HUDCommand::modeNames[(int)command.mode],
                HUDCommand::colourName[(int)command.colour],
                HUDCommand::speed[(int)command.speed],
                command.number);
}