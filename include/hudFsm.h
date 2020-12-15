#ifndef ARDUINO_H
#include <Arduino.h>
#include <elapsedMillis.h>
#endif

#ifndef Fsm
#include <Fsm.h>
#endif

//---------------------------------------------

void printState(char *text);

elapsedMillis sinceStartedCycle,
    sinceStartedPulse,
    sinceUpdatedWipe,
    sinceStartedFlash,
    sinceReadQueue;
bool pulseOn;
LedColour origColour = LedColour::BLACK;
uint8_t flashPhase = 0;
ulong interval = 0;

//---------------------------------------------
namespace HUD
{
  namespace Triggers
  {
    enum Item
    {
      IDLE = 0,
      CYCLE_BRIGHTNESS,
      DISCONNECTED,
      PULSE,
      FLASH,
      SPIN,
    };

    const char *getName(uint16_t item)
    {
      switch (item)
      {
      case IDLE:
        return "IDLE";
      case CYCLE_BRIGHTNESS:
        return "CYCLE_BRIGHTNESS";
      case DISCONNECTED:
        return "DISCONNECTED";
      case PULSE:
        return "PULSE";
      case FLASH:
        return "FLASH";
      case SPIN:
        return "SPIN";
      }
      return "WARNING: Triggers.getName() OUT_OF_RANGE";
    }

    Item mapToTriggers(uint16_t command)
    {
      using namespace HUDCommand1;
      Item item = Item::IDLE;
      if (is<HEARTBEAT>(command))
        item = Triggers::IDLE;
      else if (is<HUDCommand1::CYCLE_BRIGHTNESS>(command))
        item = Triggers::CYCLE_BRIGHTNESS;
      else if (is<HUDCommand1::DISCONNECTED>(command))
        item = Triggers::DISCONNECTED;
      else if (is<HUDCommand1::FLASH>(command))
        item = Triggers::FLASH;
      else if (is<HUDCommand1::PULSE>(command))
        item = Triggers::PULSE;
      else if (is<HUDCommand1::SPIN>(command))
        item = Triggers::SPIN;
      else if (is<TWO_FLASHES>(command))
        item = Triggers::FLASH;
      else if (is<THREE_FLASHES>(command))
        item = Triggers::FLASH;
      else
        Serial.printf("WARNING: command could not be mapped: %d\n", command);
      return item;
    }
  } // namespace Triggers

  namespace StateID
  {
    enum ID
    {
      IDLE = 0,
      FLASH,
      PULSE,
      SPIN,
      DISCONNECTED,
      Length,
    };

    // TODO uint8_t?
    const char *getStateName(uint16_t id)
    {
      switch (id)
      {
      case IDLE:
        return "IDLE";
      case FLASH:
        return "FLASH";
      case PULSE:
        return "PULSE";
      case SPIN:
        return "SPIN";
      case DISCONNECTED:
        return "DISCONNECTED";
      }
      return "OUT OF RANGE (getStateName)";
    }
  } // namespace StateID

  FsmManager<uint16_t> stateFsm;

  //===============================================================================
  State stateDisconnected(
      StateID::DISCONNECTED,
      [] {
        Serial.printf("Disconnected\n");
        stateFsm.printState(StateID::DISCONNECTED);
        sinceUpdatedWipe = 0;
      },
      [] {
        if (sinceUpdatedWipe > LED_SPIN_SPEED_MED_MS)
        {
          sinceUpdatedWipe = 0;
        }
      },
      [] {});
  //----------------------------------------
  State stateIdle(
      StateID::IDLE,
      [] {
        stateFsm.printState(StateID::IDLE);
        ledDisplay->setLeds(LedColour::BLACK); },
      [] {},
      [] {});
  //----------------------------------------

#define FINISHED 1
#define NOT_FINISHED 0

  bool flashLeds()
  {
    flashPhase++;
    if (flashPhase == ledDisplay->numFlashes * 2)
      return FINISHED;
    else
    {
      LedColour colour = (flashPhase % 2 == 0) ? origColour : LedColour::BLACK;
      ledDisplay->setLeds(colour);
    }
    return NOT_FINISHED;
  }

  State stateFlash(
      StateID::FLASH,
      [] {
        stateFsm.printState(StateID::FLASH);
        sinceStartedFlash = 0;
        flashPhase = 0;
        origColour = ledDisplay->getColour();
        ledDisplay->setLeds(origColour);
        interval = ledDisplay->getSpeedInterval();
      },
      [] {
        if (sinceStartedFlash > interval)
        {
          sinceStartedFlash = 0;
          bool finished = flashLeds();
          if (finished)
            stateFsm.trigger(Triggers::IDLE);
        }
      },
      NULL);
  //----------------------------------------
  State statePulse(
      StateID::PULSE,
      [] {
        stateFsm.printState(StateID::PULSE);
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
          ledDisplay->setLeds(pulseOn ? origColour : LedColour::BLACK);
        }
      },
      [] {
        ledDisplay->setLeds(LedColour::BLACK);
      });
  //----------------------------------------
  State stateSpin(
      StateID::SPIN,
      [] {
        stateFsm.printState(StateID::SPIN);
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
        ledDisplay->setLeds(LedColour::BLACK);
      });
  //----------------------------------------

  void sendHeartbeatToController()
  {
    uint8_t d = (HUDAction::HEARTBEAT);
    nrf24.send(COMMS_CONTROLLER, Packet::HUD, &d, sizeof(uint8_t));
  }

  Fsm fsm(&stateDisconnected);

  void addTransitions()
  {
    fsm.add_transition(&stateDisconnected, &stateIdle, Triggers::IDLE, sendHeartbeatToController);
    fsm.add_transition(&stateIdle, &stateDisconnected, Triggers::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &stateFlash, Triggers::FLASH, NULL);
    fsm.add_transition(&stateFlash, &stateIdle, Triggers::IDLE, NULL);
    fsm.add_transition(&stateFlash, &stateDisconnected, Triggers::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &statePulse, Triggers::PULSE, NULL);
    fsm.add_transition(&statePulse, &stateIdle, Triggers::IDLE, NULL);
    fsm.add_transition(&statePulse, &stateDisconnected, Triggers::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &stateSpin, Triggers::SPIN, NULL);
    fsm.add_transition(&stateSpin, &stateIdle, Triggers::IDLE, NULL);
  }
} // namespace HUD
