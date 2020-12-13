#ifndef ARDUINO_H
#include <Arduino.h>
#endif

//-----------------------------------------------------------------------

xQueueHandle xHudEventQueue;

Queue::Manager *hudQueue;

//-----------------------------------------------------------------------

namespace HUD
{
  const char *taskName = "HUD::task";
  void setupQueue();
  void setupLeds();
  void setupFsm();

  void task(void *pvParameters)
  {
    Serial.printf("%s running on core %d\n", taskName, xPortGetCoreID());

    setupQueue();

    setupLeds();

    setupFsm();

    // fsm
    while (true)
    {
      vTaskDelay(10);

      if (sinceReadQueue > 100)
      {
        using namespace HUD::Triggers;
        sinceReadQueue = 0;
        uint16_t ev = hudQueue->read<uint16_t>();
        if (ev != 0)
          stateFsm.trigger(mapToTriggers(ev));
      }
      stateFsm.runMachine();
    }
    vTaskDelete(NULL);
  }

  //-----------------------------------------------------------------------

  void createTask(uint8_t core, uint8_t priority)
  {
    xTaskCreatePinnedToCore(
        task,
        taskName,
        10000,
        NULL,
        priority,
        NULL,
        core);
  }

  void setupLeds()
  {
    FastLED.setBrightness(brightnesses[brightnessIndex]);
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
    ledDisplay = new StripLedClass();
    ledDisplay->setLeds(CRGB::Blue);
  }

  void setupQueue()
  {
    xHudEventQueue = xQueueCreate(/*len*/ 5, sizeof(uint16_t));
    hudQueue = new Queue::Manager(xHudEventQueue, /*ticks*/ 5, 0);
    hudQueue->setSentEventCallback([](uint8_t ev) {
      // if (PRINT_QUEUE_SEND)
      //   Serial.printf(QUEUE_SEND_FORMAT_STRING, HUDCommand1::getName(ev));
    });
    hudQueue->setReadEventCallback([](uint8_t ev) {
      // if (PRINT_QUEUE_READ)
      //   Serial.printf(QUEUE_READ_FORMAT_STRING, HUDCommand1::getName(ev));
    });
  }

  void setupFsm()
  {
    stateFsm.begin(&fsm, STATE_STRING_FORMAT_WITH_EVENT, STATE_STRING_FORMAT_WITHOUT_EVENT);
    stateFsm.setGetStateNameCallback([](uint8_t id) {
      return HUD::StateID::getStateName(id);
    });
    stateFsm.setGetEventNameCallback([](uint8_t ev) {
      return Triggers::getName(ev);
    });
    fsm.setTriggeredCb([](int ev) {
      // Serial.printf(FSM_TRIGGER_FORMAT_STRING, HUDCommand1::getName(ev));
    });
    addTransitions();
  }

} // namespace HUD
  //-----------------------------------------------------------------------