#ifndef VescData_h
#define VescData_h

#pragma once
#include <elapsedMillis.h>

#include "Arduino.h"

enum ReasonType
{
  BOARD_STOPPED,
  BOARD_MOVING,
  FIRST_PACKET,
  LAST_WILL,
  REQUESTED,
  VESC_OFFLINE,
  RESPONSE,
};

//--------------------------------------------

class VescData
{
public:
  unsigned long id;
  float batteryVoltage;
  bool moving;
  float ampHours;
  float motorCurrent;
  float odometer; // in kilometers
  bool vescOnline;
  ReasonType reason;
};

//--------------------------------------------

class ControllerData
{
public:
  unsigned long id;
  uint8_t throttle;
  bool cruise_control;
  uint8_t command;
};

class ControllerConfig
{
public:
  unsigned long id;
  uint16_t send_interval;
};

class ControllerClass
{
public:
  ControllerData data;
  ControllerConfig config;
  elapsedMillis sinceLastPacket;
  bool connected = false;

  void save(ControllerData latest)
  {
    _prev = data;
    data = latest;
    sinceLastPacket = 0;
  }

  void save(ControllerConfig latestConfig)
  {
    config = latestConfig;
    sinceLastPacket = 0;
  }

  uint16_t missedPackets()
  {
    return data.id > 0
               ? (data.id - _prev.id) - 1
               : 0;
  }

  bool throttleChanged()
  {
    return data.throttle != _prev.throttle;
  }

  bool hasTimedout()
  {
    return sinceLastPacket > CONTROLLER_CONNECTED_INTERVAL;
  }

private:
  ControllerData _prev;
};

//--------------------------------------------

class ControllerCommand
{
public:
  uint32_t id;
  HUDCommand::Mode mode;
  HUDCommand::Colour colour;

  ControllerCommand() {}

  ControllerCommand(HUDCommand::Mode mode, HUDCommand::Colour colour)
  {
    mode = mode;
    colour = colour;
  }
};

#endif