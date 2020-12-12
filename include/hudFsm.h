#ifndef ARDUINO_H
#include <Arduino.h>
#include <elapsedMillis.h>
#endif

#ifndef Fsm
#include <Fsm.h>
#endif

//---------------------------------------------

void printState(char *text);
void sneakyTrigger(uint8_t event);

elapsedMillis sinceStartedCycle,
    sinceStartedPulse,
    sinceUpdatedWipe,
    sinceStartedFlash,
    sinceReadQueue;
bool pulseOn;
CRGB origColour = CRGB::Black;
uint8_t flashPhase = 0;
ulong interval = 0;

//---------------------------------------------
namespace HUD
{
  enum StateID
  {
    IDLE = 0,
    FLASH,
    PULSE,
    SPIN,
    DISCONNECTED,
    Length,
  };

  std::string stateNames[] =
      {
          "IDLE",
          "FLASH",
          "PULSE",
          "SPIN",
          "DISCONNECTED",
  };

  std::string getStateName(uint8_t id)
  {
    return id < Length && ARRAY_SIZE(stateNames) == Length
               ? stateNames[id]
               : "OUT OF RANGE";
  }

  FsmManager<HUDCommand::Mode> stateFsm;

  State stateIdle([] {
        stateFsm.printState(IDLE);
        ledDisplay->setLeds(CRGB::Black); },
                  [] {},
                  [] {});

  bool flashLeds()
  {
    flashPhase++;
    if (flashPhase == ledDisplay->numFlashes * 2)
    {
      return true; // finished
    }
    else
    {
      CRGB colour = (flashPhase % 2 == 0) ? origColour : CRGB::Black;
      ledDisplay->setLeds(colour);
    }
    return false; // not finished
  }

  State stateFlash(
      [] {
        stateFsm.printState(FLASH);
        ledDisplay->setLeds();
        sinceStartedFlash = 0;
        flashPhase = 0;
        origColour = ledDisplay->getColour();
        interval = ledDisplay->getSpeedInterval();
      },
      [] {
        if (sinceStartedFlash > interval)
        {
          sinceStartedFlash = 0;

          bool finished = flashLeds();
          if (finished)
            stateFsm.trigger(HUDCommand::MODE_NONE);
        }
      },
      NULL);

  State statePulse(
      [] {
        stateFsm.printState(PULSE);
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
        stateFsm.printState(SPIN);
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
        stateFsm.printState(DISCONNECTED);
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

  Fsm fsm(&stateDisconnected);

  void addTransitions()
  {
    fsm.add_transition(&stateDisconnected, &stateIdle, HUDCommand::MODE_NONE, sendHeartbeatToController);
    fsm.add_transition(&stateIdle, &stateDisconnected, HUDSpecialEvents::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &stateFlash, HUDCommand::FLASH, NULL);
    fsm.add_transition(&stateFlash, &stateIdle, HUDCommand::MODE_NONE, NULL);
    fsm.add_transition(&stateFlash, &stateDisconnected, HUDSpecialEvents::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &statePulse, HUDCommand::PULSE, NULL);
    fsm.add_transition(&statePulse, &stateIdle, HUDCommand::MODE_NONE, NULL);
    fsm.add_transition(&statePulse, &stateDisconnected, HUDSpecialEvents::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &stateSpin, HUDCommand::SPIN, NULL);
    fsm.add_transition(&stateSpin, &stateIdle, HUDCommand::MODE_NONE, NULL);
  }
} // namespace HUD
