#ifndef ARDUINO_H
#include <Arduino.h>
#endif

EventQueueManager *hudStateQueue;

xQueueHandle xHudEventQueue;

void hudStateTask_1(void *pvParameters)
{
  Serial.printf("hudStateTask_1 running on core %d\n", xPortGetCoreID());

  FastLED.setBrightness(brightnesses[brightnessIndex]);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  xHudEventQueue = xQueueCreate(/*len*/ 5, sizeof(uint8_t));
  hudStateQueue = new EventQueueManager(xHudEventQueue, /*ticks*/ 5);
  hudStateQueue->setSentEventCallback([](uint8_t ev) {
    Serial.printf("-->hudStateQueue->send: (%s)\n", hudCommandNames[ev]);
  });
  hudStateQueue->setReadEventCallback([](uint8_t ev) {
    Serial.printf("<--hudStateQueue->read: (%s)\n", hudCommandNames[ev]);
  });

  ledDisplay = new StripLedClass();

  ledDisplay->setLeds(CRGB::Blue);

  hudFsm.setEventTriggeredCb([](int ev) {
    Serial.printf("-->hudFsm.event: %s\n", hudCommandNames[ev]);
  });
  addHudStateTransitions();

  while (true)
  {
    vTaskDelay(10);

    if (sinceReadQueue > 100)
    {
      sinceReadQueue = 0;
      if (hudStateQueue->messageAvailable())
      {
        HUDCommand ev = (HUDCommand)hudStateQueue->read();
        switch (ev)
        {
        case HUD_CMD_CYCLE_BRIGHTNESS:
          ledDisplay->cycleBrightness();
          break;
        default:
          hudFsm.trigger(ev);
        };
      }
    }

    hudFsm.run_machine();
  }
  vTaskDelete(NULL);
}

void createLedsTask(uint8_t core, uint8_t priority)
{
  xTaskCreatePinnedToCore(
      hudStateTask_1,
      "hudStateTask_1",
      10000,
      NULL,
      priority, NULL, core);
}