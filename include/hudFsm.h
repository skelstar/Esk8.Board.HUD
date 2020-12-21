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
ulong flashInterval = 0;

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

    Item mapToTriggers(HUD::Command command)
    {
      using namespace HUD;
      Item item = Item::IDLE;
      if (command.is<HUD::HEARTBEAT>())
        item = Triggers::IDLE;
      else if (command.is<HUD::CYCLE_BRIGHTNESS>())
        item = Triggers::CYCLE_BRIGHTNESS;
      else if (command.is<HUD::DISCONNECTED>())
        item = Triggers::DISCONNECTED;
      else if (command.is<HUD::FLASH>())
        item = Triggers::FLASH;
      else if (command.is<HUD::PULSE>())
        item = Triggers::PULSE;
      else if (command.is<HUD::SPIN>())
        item = Triggers::SPIN;
      else if (command.is<HUD::TWO_FLASHES>())
        item = Triggers::FLASH;
      else if (command.is<HUD::THREE_FLASHES>())
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

  State stateFlash(
      StateID::FLASH,
      [] {
        stateFsm.printState(StateID::FLASH);
        sinceStartedFlash = 0;
        flashInterval = ledDisplay->getSpeedInterval();
        ledDisplay->flashLeds(/*start*/ true);
      },
      [] {
        if (sinceStartedFlash > flashInterval)
        {
          sinceStartedFlash = 0;
          if (ledDisplay->flashLeds() == FINISHED)
            stateFsm.trigger(Triggers::IDLE);
        }
      },
      [] {
        // clean up
        ledDisplay->setColour(LedColour::BLACK);
      });
  //----------------------------------------
  State statePulse(
      StateID::PULSE,
      [] {
        stateFsm.printState(StateID::PULSE);
        ledDisplay->setLeds();
        pulseOn = true;
        sinceStartedPulse = 0;
        // origColour = ledDisplay->getColour();
      },
      [] {
        ulong interval = ledDisplay->getSpeedInterval();
        if (sinceStartedPulse > interval)
        {
          sinceStartedPulse = 0;
          pulseOn = !pulseOn;
          // ledDisplay->setLeds(pulseOn ? origColour : LedColour::BLACK);
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
    uint8_t d = (HUDAction::Event::HEARTBEAT);
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
