#ifndef ARDUINO_H
#include <Arduino.h>
#endif

//-----------------------------------------------------------------------

xQueueHandle xHudEventQueue;

Queue::Manager hudQueue(xHudEventQueue, /*ticks*/ 5);

//-----------------------------------------------------------------------

void hudStateTask_1(void *pvParameters)
{
  Serial.printf("hudStateTask_1 running on core %d\n", xPortGetCoreID());

  FastLED.setBrightness(brightnesses[brightnessIndex]);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  // Queue
  xHudEventQueue = xQueueCreate(/*len*/ 5, sizeof(uint8_t));
  hudQueue.setSentEventCallback([](uint8_t ev) {
    if (PRINT_QUEUE_SEND)
      Serial.printf("-->hudStateQueue->send: (%s)\n", HUDCommand::getMode(ev));
  });
  hudQueue.setReadEventCallback([](uint8_t ev) {
    if (PRINT_QUEUE_READ)
      Serial.printf("-->hudStateQueue->read: (%s)\n", HUDCommand::getMode(ev));
  });

  ledDisplay = new StripLedClass();
  ledDisplay->setLeds(CRGB::Blue);

  // fsm
  HUD::fsm.setEventTriggeredCb([](int ev) {
    Serial.printf("-->fsm.event: %s\n", HUDCommand::getMode(ev));
  });
  HUD::addTransitions();

  while (true)
  {
    vTaskDelay(10);

    if (sinceReadQueue > 100)
    {
      sinceReadQueue = 0;
      if (hudQueue.messageAvailable())
      {
        HUDCommand::Mode ev = hudQueue.read<HUDCommand::Mode>();
        HUD::fsm.trigger(ev);
      }
    }

    HUD::fsm.run_machine();
  }
  vTaskDelete(NULL);
}

//-----------------------------------------------------------------------

void createLedsTask(uint8_t core, uint8_t priority)
{
  xTaskCreatePinnedToCore(
      hudStateTask_1,
      "hudStateTask_1",
      10000,
      NULL,
      priority, NULL, core);
}

//-----------------------------------------------------------------------