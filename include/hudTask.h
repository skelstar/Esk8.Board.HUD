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
    sinceReadQueue;
bool pulseRedOn;

Fsm *ledsState;

//---------------------------------------------

State stateIdle(
    [] {
      printState("stateIdle");
    },
    [] {},
    [] {});

State statePulseRed(
    [] {
      printState("statePulseRed");
      setLeds(CRGB::DarkRed);
      sinceStartedPulseRed = 0;
    },
    [] {
      if (sinceStartedPulseRed > 1000)
      {
        sinceStartedPulseRed = 0;
        pulseRedOn = !pulseRedOn;
        setLeds(pulseRedOn ? CRGB::DarkRed : CRGB::Black);
      }
    },
    [] {
      setLeds(CRGB::Black);
    });

State stateFlashGreen(
    [] {
      printState("stateFlashGreen");
      setLeds(CRGB::DarkGreen);
    },
    [] {},
    [] {
      setLeds(CRGB::Black);
    });

State stateDisconnected(
    [] {
      printState("stateDisconnected");
    },
    [] {},
    [] {});

//---------------------------------------------

void printEventCb(int ev)
{
  Serial.printf("--> event: %s\n", ledStateEventNames[ev]);
}

void ledTask_1(void *pvParameters)
{
  Serial.printf("ledTask_1 running on core %d\n", xPortGetCoreID());

  ledsState = new Fsm(&stateIdle);
  ledsState->setEventTriggeredCb(printEventCb);
  addLedsStateTransitions();

  while (true)
  {
    vTaskDelay(10);

    if (sinceReadQueue > 500)
    {
      sinceReadQueue = 0;
      if (ledsQueueManager->messageAvailable())
      {
        LedsStateEvent ev = (LedsStateEvent)ledsQueueManager->read();
        ledsState->trigger(ev);
      }
    }

    ledsState->run_machine();
  }
  vTaskDelete(NULL);
}

void addLedsStateTransitions()
{
  ledsState->add_transition(&stateIdle, &stateFlashGreen, EV_LED_FLASH_GREEN, NULL);
  ledsState->add_timed_transition(&stateFlashGreen, &stateIdle, FLASH_LENGTH_MS, NULL);
  ledsState->add_transition(&stateFlashGreen, &stateDisconnected, EV_LED_DISCONNECTED, NULL);

  ledsState->add_transition(&stateIdle, &statePulseRed, EV_LED_PULSE_RED, NULL);
  ledsState->add_transition(&statePulseRed, &stateIdle, EV_LED_CONNECTED, NULL);
  ledsState->add_transition(&statePulseRed, &stateDisconnected, EV_LED_DISCONNECTED, NULL);

  ledsState->add_transition(&stateDisconnected, &stateIdle, EV_LED_CONNECTED, NULL);
  ledsState->add_transition(&stateIdle, &stateDisconnected, EV_LED_DISCONNECTED, NULL);
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