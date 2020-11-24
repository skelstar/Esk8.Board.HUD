#ifndef ARDUINO_H
#include <Arduino.h>
#include <elapsedMillis.h>
#endif

#ifndef Fsm
#include <Fsm.h>
#endif

//---------------------------------------------

void addLedsStateTransitions();
void printState(char *text);

const unsigned long FLASH_LENGTH_MS = 1000;
elapsedMillis sinceStartedCycle,
    sinceStartedPulseRed,
    sinceUpdatedWipe,
    sinceReadQueue;
bool pulseRedOn;

Fsm *ledsState;

//---------------------------------------------

State stateIdle(
    [] {
      printState("stateIdle");
      ledDisplay->setLeds(CRGB::Black);
    },
    [] {},
    [] {});

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

State stateSpinGreenFast(
    [] {
      printState("stateSpinGreenFast");
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

State stateFlashGreen(
    [] {
      printState("stateFlashGreen");
      ledDisplay->setLeds(CRGB::DarkGreen);
    },
    NULL,
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

//---------------------------------------------

void printEventCb(int ev)
{
  Serial.printf("--> event: %s\n", ledStateEventNames[ev]);
}

void addLedsStateTransitions()
{
  // ledsState->add_transition(&stateIdle, &stateFlashGreen, EV_LED_FLASH_GREEN, NULL);
  // ledsState->add_timed_transition(&stateFlashGreen, &stateIdle, FLASH_LENGTH_MS, NULL);
  // ledsState->add_transition(&stateFlashGreen, &stateDisconnected, EV_LED_DISCONNECTED, NULL);

  // ledsState->add_transition(&stateIdle, &statePulseRed, EV_LED_PULSE_RED, NULL);
  // ledsState->add_transition(&statePulseRed, &stateIdle, EV_LED_CONNECTED, NULL);
  // ledsState->add_transition(&statePulseRed, &stateDisconnected, EV_LED_DISCONNECTED, NULL);

  ledsState->add_transition(&stateDisconnected, &stateIdle, EV_LED_CONNECTED, NULL);
  // ledsState->add_transition(&stateIdle, &stateDisconnected, EV_LED_DISCONNECTED, NULL);

  // ledsState->add_transition(&stateIdle, &stateSpinGreenFast, EV_LED_SPIN_GREEN_FAST, NULL);
  // ledsState->add_transition(&stateSpinGreenFast, &stateIdle, EV_LED_CONNECTED, NULL);
}

void triggerLedsStateEvent(LedsStateEvent ev)
{
  ledsState->trigger(ev);
#ifdef PRINT_LED_READ_TRIGGER
  Serial.printf("ledState rx trigger: %s\n", ledStateEventNames[(int)ev]);
#endif
}

void printState(char *text)
{
#ifdef PRINT_LED_STATE
  Serial.printf("ledState: %s\n", text);
#endif
}