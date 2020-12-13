#ifndef ARDUINO_H
#include <Arduino.h>
#endif

//-----------------------------------------------------------------------

xQueueHandle xHudEventQueue;

Queue::Manager *hudQueue;

//-----------------------------------------------------------------------

namespace HUD
{
  void task(void *pvParameters)
  {
    Serial.printf("hudStateTask_1 running on core %d\n", xPortGetCoreID());

    // Queue
    xHudEventQueue = xQueueCreate(/*len*/ 5, sizeof(uint8_t));
    hudQueue = new Queue::Manager(xHudEventQueue, /*ticks*/ 5, HUDCommand::MODE_NO_EVENT);
    hudQueue->setSentEventCallback([](uint8_t ev) {
      if (PRINT_QUEUE_SEND)
        Serial.printf(QUEUE_SEND_FORMAT_STRING, HUDCommand::getMode(ev));
    });
    hudQueue->setReadEventCallback([](uint8_t ev) {
      if (PRINT_QUEUE_READ)
        Serial.printf(QUEUE_READ_FORMAT_STRING, HUDCommand::getMode(ev));
    });

    // leds
    FastLED.setBrightness(brightnesses[brightnessIndex]);
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
    ledDisplay = new StripLedClass();
    ledDisplay->setLeds(CRGB::Blue);

    // fsm
    stateFsm.begin(&fsm, STATE_STRING_FORMAT_LONG, STATE_STRING_FORMAT_SHORT);
    stateFsm.setGetStateNameCallback([](uint8_t id) {
      return HUD::getStateName(id);
    });
    fsm.setTriggeredCb([](int ev) {
      Serial.printf(FSM_TRIGGER_FORMAT_STRING, HUDCommand::getMode(ev));
    });
    addTransitions();

    while (true)
    {
      vTaskDelay(10);

      if (sinceReadQueue > 100)
      {
        sinceReadQueue = 0;
        HUDCommand::Mode ev = hudQueue->read<HUDCommand::Mode>();
        if (ev != HUDCommand::MODE_NO_EVENT)
        {
          stateFsm.trigger(ev);
        }
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
        "HUD::task",
        10000,
        NULL,
        priority,
        NULL,
        core);
  }
} // namespace HUD
  //-----------------------------------------------------------------------