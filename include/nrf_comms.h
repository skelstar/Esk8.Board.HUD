#ifndef ARDUINO_H
#include <Arduino.h>
#endif
#ifndef ESK8_ENUM_MANAGER
#include <EnumManager.h>
#endif

template <typename T>
T readFromNrf();
void printRxPacket(ControllerCommand command);

//------------------------------------------------------------------

void controllerConnectedChange()
{
  Serial.printf("Controller: %s\n", controllerClient.connected() ? "CONNECTED" : "DISCONNECTED");
}

void packetAvailable_cb(uint16_t from_id, uint8_t type)
{
  controller.sinceLastPacket = 0;

  // if controller was offline, send HEARTBEAT
  if (false == controller.connected)
  {
    controller.connected = true;
    hudQueue.send(HUDCommand::MODE_NONE);
  }

  if (type == (int)Packet::HUD)
  {
    ControllerCommand command = controllerClient.read<ControllerCommand>();

    if (command.mode == HUDCommand::CYCLE_BRIGHTNESS)
    {
      ledDisplay->cycleBrightness();
      ledDisplay->setColour(HUDCommand::BLUE);
      ledDisplay->setSpeed(HUDCommand::SLOW);
      hudQueue.send(HUDCommand::FLASH);
    }
    else
    {
      ledDisplay->setColour(command.colour);
      ledDisplay->setSpeed(command.speed);
      ledDisplay->numFlashes = command.number;

      hudQueue.send(command.mode);
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
void printRxPacket(ControllerCommand command)
{
  Serial.printf("-->rx: %s|%s|%s|%d\n",
                HUDCommand::modes.getNameSafe(command.mode),
                HUDCommand::colours.getNameSafe(command.colour),
                HUDCommand::speed.getNameSafe(command.speed),
                command.number);
}