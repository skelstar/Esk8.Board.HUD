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
  namespace Triggers
  {
    enum Item
    {
      IDLE,
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
      return "Trigger OUT_OF_RANGE";
    }

    Item mapToTriggers(uint16_t command)
    {
      using namespace HUDCommand1;
      Serial.printf("mapping from %s\n", HUDCommand1::getMode(command));
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
      // Serial.printf("mapped to: %s\n", getName(item));
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

    std::string name[] =
        {
            "IDLE",
            "FLASH",
            "PULSE",
            "SPIN",
            "DISCONNECTED",
    };

    const char *getStateName(uint8_t id)
    {
      return id < Length && ARRAY_SIZE(name) == Length
                 ? name[id].c_str()
                 : "getStateName: OUT OF RANGE";
    }
  } // namespace StateID

  FsmManager<uint16_t> stateFsm;

  State stateIdle([] {
        stateFsm.printState(StateID::IDLE);
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
        stateFsm.printState(StateID::FLASH);
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
            stateFsm.trigger(1 << Triggers::IDLE);
        }
      },
      NULL);

  State statePulse(
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
          ledDisplay->setLeds(pulseOn ? origColour : CRGB::Black);
        }
      },
      [] {
        ledDisplay->setLeds(CRGB::Black);
      });

  State stateSpin(
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
        ledDisplay->setLeds(CRGB::Black);
      });

  State stateDisconnected(
      [] {
        stateFsm.printState(StateID::DISCONNECTED);
        sinceUpdatedWipe = 0;
      },
      [] {
        if (sinceUpdatedWipe > LED_SPIN_SPEED_MED_MS)
        {
          sinceUpdatedWipe = 0;
          // ledDisplay->setColour(CRGB::DarkBlue);
          // ledDisplay->animation();
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
    fsm.add_transition(&stateDisconnected, &stateIdle, Triggers::IDLE, sendHeartbeatToController);
    fsm.add_transition(&stateIdle, &stateDisconnected, Triggers::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &stateFlash, 1 << Triggers::FLASH, NULL);
    fsm.add_transition(&stateFlash, &stateIdle, 1 << Triggers::IDLE, NULL);
    fsm.add_transition(&stateFlash, &stateDisconnected, 1 << Triggers::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &statePulse, 1 << Triggers::PULSE, NULL);
    fsm.add_transition(&statePulse, &stateIdle, 1 << Triggers::IDLE, NULL);
    fsm.add_transition(&statePulse, &stateDisconnected, 1 << Triggers::DISCONNECTED, NULL);

    fsm.add_transition(&stateIdle, &stateSpin, 1 << Triggers::SPIN, NULL);
    fsm.add_transition(&stateSpin, &stateIdle, 1 << Triggers::IDLE, NULL);
  }
} // namespace HUD
