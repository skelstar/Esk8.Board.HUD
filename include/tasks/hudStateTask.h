#ifndef ARDUINO_H
#include <Arduino.h>
#endif

QueueManager *hudStateQueue;

xQueueHandle xHudEventQueue;

void hudStateTask_1(void *pvParameters)
{
  Serial.printf("hudStateTask_1 running on core %d\n", xPortGetCoreID());

  FastLED.setBrightness(brightnesses[brightnessIndex]);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  xHudEventQueue = xQueueCreate(/*len*/ 5, sizeof(uint8_t));
  hudStateQueue = new QueueManager(xHudEventQueue, /*ticks*/ 5);
  hudStateQueue->setSentEventCallback([](uint8_t ev) {
    if (PRINT_QUEUE_SEND)
      Serial.printf("-->hudStateQueue->send: (%s)\n", HUDCommand::modeNames[ev]);
  });
  hudStateQueue->setReadEventCallback([](uint8_t ev) {
    if (PRINT_QUEUE_READ)
      Serial.printf("<--hudStateQueue->read: (%s)\n", HUDCommand::modeNames[ev]);
  });

  ledDisplay = new StripLedClass();

  ledDisplay->setLeds(CRGB::Blue);

  hudFsm.setEventTriggeredCb([](int ev) {
    Serial.printf("-->hudFsm.event: %s\n", HUDCommand::modeNames[ev]);
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
        HUDCommand::Mode ev = hudStateQueue->read<HUDCommand::Mode>();
        hudFsm.trigger(ev);
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