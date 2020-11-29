#ifndef ARDUINO_H
#include <Arduino.h>
#include <elapsedMillis.h>
#endif

#ifndef Fsm
#include <Fsm.h>
#endif

//---------------------------------------------

void addHudStateTransitions();
void printState(char *text);
void sneakyTrigger(uint8_t event);

elapsedMillis sinceStartedCycle,
    sinceStartedPulse,
    sinceUpdatedWipe,
    sinceStartedFlash,
    sinceReadQueue;
bool pulseOn;
CRGB origColour = CRGB::Black;

//---------------------------------------------

State stateIdle(
    [] {
      printState("stateIdle");
      ledDisplay->setLeds(CRGB::Black);
    },
    [] {},
    [] {});

State stateFlash(
    [] {
      printState("stateFlash");
      ledDisplay->setLeds();
      sinceStartedFlash = 0;
    },
    [] {
      ulong interval = ledDisplay->getSpeedInterval();
      if (sinceStartedFlash > interval)
        sneakyTrigger((int)HUDCommand::MODE_NONE);
    },
    NULL);

State statePulse(
    [] {
      printState("statePulse");
      ledDisplay->setLeds();
      pulseOn = true;
      sinceStartedPulse = 0;
      origColour = ledDisplay->getColour();
    },
    [] {
      ulong interval = ledDisplay->getSpeedInterval();
      if (sinceStartedPulse > interval)
      {
        sinceStartedPulse = 0;
        pulseOn = !pulseOn;
        ledDisplay->setLeds(pulseOn ? origColour : CRGB::Black);
      }
    },
    [] {
      ledDisplay->setLeds(CRGB::Black);
    });

State stateSpin(
    [] {
      printState("stateSpin");
      sinceUpdatedWipe = 0;
    },
    [] {
      if (sinceUpdatedWipe > LED_SPIN_SPEED_FAST_MS)
      {
        sinceUpdatedWipe = 0;
        ledDisplay->animation();
      }
    },
    [] {
      ledDisplay->setLeds(CRGB::Black);
    });

State stateDisconnected(
    [] {
      printState("stateDisconnected");
      sinceUpdatedWipe = 0;
    },
    [] {
      if (sinceUpdatedWipe > LED_SPIN_SPEED_MED_MS)
      {
        sinceUpdatedWipe = 0;
        ledDisplay->setColour(CRGB::DarkBlue);
        ledDisplay->animation();
      }
    },
    [] {});

void sendHeartbeatToController()
{
  uint8_t d = (HUDAction::HEARTBEAT);
  nrf24.send(COMMS_CONTROLLER, Packet::HUD, &d, sizeof(uint8_t));
}

Fsm hudFsm(&stateDisconnected);

void addHudStateTransitions()
{
  hudFsm.add_transition(&stateDisconnected, &stateIdle, HUDCommand::MODE_NONE, sendHeartbeatToController);
  hudFsm.add_transition(&stateIdle, &stateDisconnected, HUDSpecialEvents::DISCONNECTED, NULL);

  hudFsm.add_transition(&stateIdle, &stateFlash, HUDCommand::FLASH, NULL);
  hudFsm.add_transition(&stateFlash, &stateIdle, HUDCommand::MODE_NONE, NULL);
  // hudFsm.add_timed_transition(&stateFlash, &stateIdle, ledDisplay->getSpeedInterval(), NULL);
  hudFsm.add_transition(&stateFlash, &stateDisconnected, HUDSpecialEvents::DISCONNECTED, NULL);

  hudFsm.add_transition(&stateIdle, &statePulse, HUDCommand::PULSE, NULL);
  hudFsm.add_transition(&statePulse, &stateIdle, HUDCommand::MODE_NONE, NULL);
  hudFsm.add_transition(&statePulse, &stateDisconnected, HUDSpecialEvents::DISCONNECTED, NULL);

  hudFsm.add_transition(&stateIdle, &stateSpin, HUDCommand::SPIN, NULL);
  hudFsm.add_transition(&stateSpin, &stateIdle, HUDCommand::MODE_NONE, NULL);
}

void printState(char *text)
{
#ifdef PRINT_LED_STATE
  Serial.printf("-----------------------ledState: %s\n", text);
#endif
}

void sneakyTrigger(uint8_t event)
{
  hudFsm.trigger(event);
}