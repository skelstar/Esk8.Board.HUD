#ifndef ARDUINO_H
#include <Arduino.h>
#endif

template <typename T>
T readFromNrf();
void printRxPacket(ControllerCommand command);

//------------------------------------------------------------------

void controllerConnectedChange()
{
  Serial.printf(CLIENT_CONNECT_CHANGE_FORMAT_STRING,
                controllerClient.connected()
                    ? "CONNECTED"
                    : "DISCONNECTED");
}

void packetAvailable_cb(uint16_t from_id, uint8_t type)
{
  controller.sinceLastPacket = 0;

  // if controller was offline, send HEARTBEAT
  if (false == controller.connected)
  {
    controller.connected = true;
    hudQueue->send(HUDCommand::MODE_NONE);
  }

  if (type == (int)Packet::HUD)
  {
    ControllerCommand command = controllerClient.read();

    printRxPacket(command);

    if (command.mode == HUDCommand::CYCLE_BRIGHTNESS)
    {
      ledDisplay->cycleBrightness();
      ledDisplay->setColour(HUDCommand::BLUE);
      ledDisplay->setSpeed(HUDCommand::SLOW);
      hudQueue->send(HUDCommand::FLASH);
    }
    else
    {
      ledDisplay->setColour(command.colour);
      ledDisplay->setSpeed(command.speed);
      ledDisplay->numFlashes = command.number;

      hudQueue->send(command.mode);
    }
  }
  else
  {
    Serial.printf("Packet not supported: %s\n", Packet::getType(type));
  }
}
//------------------------------------------------------------------
void printRxPacket(ControllerCommand command)
{
  if (PRINT_PACKET_RX)
    Serial.printf(RX_PACKET_FORMAT_STRING,
                  HUDCommand::getMode(command.mode),
                  HUDCommand::getColour(command.colour),
                  HUDCommand::getSpeed(command.speed),
                  command.number);
}