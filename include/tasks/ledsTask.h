#ifndef ARDUINO_H
#include <Arduino.h>
#endif

EventQueueManager *ledsQueueManager;

xQueueHandle xLedsEventQueue;

void ledTask_1(void *pvParameters)
{
  Serial.printf("ledTask_1 running on core %d\n", xPortGetCoreID());

  FastLED.setBrightness(brightnesses[brightnessIndex]);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  xLedsEventQueue = xQueueCreate(/*len*/ 5, sizeof(uint8_t));
  ledsQueueManager = new EventQueueManager(xLedsEventQueue, /*ticks*/ 5);

  ledDisplay = new StripLedClass();

  ledDisplay->setLeds(CRGB::Blue);

  ledsState = new Fsm(&stateDisconnected);
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
        switch (ev)
        {
        case EV_LED_CYCLE_BRIGHTNESS:
          ledDisplay->cycleBrightness();
          break;
        default:
          ledsState->trigger(ev);
        };
      }
    }

    ledsState->run_machine();
  }
  vTaskDelete(NULL);
}

void createLedsTask(uint8_t core, uint8_t priority)
{
  xTaskCreatePinnedToCore(
      ledTask_1,
      "ledTask_1",
      10000,
      NULL,
      priority, NULL, core);
}