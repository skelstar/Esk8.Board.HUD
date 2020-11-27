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

const unsigned long FLASH_LENGTH_MS = 1000;
elapsedMillis sinceStartedCycle,
    sinceStartedPulseRed,
    sinceUpdatedWipe,
    sinceStartedFlash,
    sinceReadQueue;
bool pulseRedOn;

//---------------------------------------------

State stateIdle(
    [] {
      printState("stateIdle");
      ledDisplay->setLeds(CRGB::Black);
    },
    [] {},
    [] {});

State stateFlashGreen(
    [] {
      printState("stateFlashGreen");
      ledDisplay->setLeds(CRGB::DarkGreen);
    },
    NULL, NULL);

State statePulseRed(
    [] {
      printState("statePulseRed");
      ledDisplay->setLeds(CRGB::DarkRed);
      sinceStartedPulseRed = 0;
    },
    [] {
      if (sinceStartedPulseRed > 1000)
      {
        sinceStartedPulseRed = 0;
        pulseRedOn = !pulseRedOn;
        ledDisplay->setLeds(pulseRedOn ? CRGB::DarkRed : CRGB::Black);
      }
    },
    [] {
      ledDisplay->setLeds(CRGB::Black);
    });

State stateSpinGreen(
    [] {
      printState("stateSpinGreen");
      sinceUpdatedWipe = 0;
    },
    [] {
      if (sinceUpdatedWipe > LED_SPIN_SPEED_FAST_MS)
      {
        sinceUpdatedWipe = 0;
        ledDisplay->animation(CRGB::DarkGreen);
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
        ledDisplay->animation(CRGB::DarkBlue);
      }
    },
    [] {});

void sendConnected()
{
  uint8_t d = (HudActionEvent::HUD_ACTION_HEARTBEAT);
  nrf24.send(COMMS_CONTROLLER, HUD, &d, sizeof(uint8_t));
}

Fsm hudFsm(&stateDisconnected);

void addHudStateTransitions()
{
  hudFsm.add_transition(&stateDisconnected, &stateIdle, HUD_CMD_HEARTBEAT, sendConnected);
  hudFsm.add_transition(&stateIdle, &stateDisconnected, HUD_CMD_DISCONNECTED, NULL);

  hudFsm.add_transition(&stateIdle, &stateFlashGreen, HUD_CMD_FLASH_GREEN, NULL);
  hudFsm.add_timed_transition(&stateFlashGreen, &stateIdle, LED_FLASH_LENGTH_MS, NULL);
  hudFsm.add_transition(&stateFlashGreen, &stateDisconnected, HUD_CMD_DISCONNECTED, NULL);

  hudFsm.add_transition(&stateIdle, &statePulseRed, HUD_CMD_PULSE_RED, NULL);
  hudFsm.add_transition(&statePulseRed, &stateIdle, HUD_CMD_IDLE, NULL);
  hudFsm.add_transition(&statePulseRed, &stateDisconnected, HUD_CMD_DISCONNECTED, NULL);

  hudFsm.add_transition(&stateIdle, &stateSpinGreen, HUD_CMD_SPIN_GREEN, NULL);
  hudFsm.add_transition(&stateSpinGreen, &stateIdle, HUD_CMD_IDLE, NULL);
}

void printState(char *text)
{
#ifdef PRINT_LED_STATE
  Serial.printf("ledState: %s\n", text);
#endif
}