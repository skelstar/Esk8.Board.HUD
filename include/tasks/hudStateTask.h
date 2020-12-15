#ifndef ARDUINO_H
#include <Arduino.h>
#endif

//-----------------------------------------------------------------------

Queue::Manager *hudQueue;

void hudQueueInit()
{
  hudQueue = new Queue::Manager(/*len*/ 5, sizeof(uint16_t), /*ticks*/ 5, 0);
  hudQueue->setSentEventCallback([](uint16_t ev) {
    if (PRINT_QUEUE_SEND)
      Serial.printf(OUT_EVENT_FORMAT_STRING, "[Q:Send]", HUDCommand1::getCommand(ev), "hudQueue");
  });
  hudQueue->setReadEventCallback([](uint16_t ev) {
    if (PRINT_QUEUE_READ)
      Serial.printf(IN_EVENT_FORMAT_STRING, "[Q:Read]", HUDCommand1::getCommand(ev), "hudQueue");
  });
}

//-----------------------------------------------------------------------

namespace HUD
{

  const char *taskName = "HUD::task";
  void setupLeds();
  void setupFsm();

  elapsedMillis sinceRunMachine;

  void task(void *pvParameters)
  {
    Serial.printf("%s running on core %d\n", taskName, xPortGetCoreID());

    hudQueueInit();
    setupLeds();
    setupFsm();

    Serial.printf("HUD fsm ready\n");
    hudFsmReady = true;

    // fsm
    while (true)
    {
      vTaskDelay(10);

      if (sinceReadQueue > 10)
      {
        sinceReadQueue = 0;
        using namespace HUDCommand1;
        uint16_t command = hudQueue->read<uint16_t>();

        if (command != 0)
        {
          ledDisplay->setColour(mapToLedColour(command));
          ledDisplay->setSpeed(mapToLedSpeed(command));
          ledDisplay->numFlashes = mapToNumFlashes(command);

          stateFsm.trigger(HUD::Triggers::mapToTriggers(command));
        }
      }
      sinceRunMachine = 0;
      fsm.run_machine();
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
    ledDisplay->setLeds(LedColour::BLUE);
  }

  void setupFsm()
  {
    addTransitions();

    stateFsm.begin(&fsm, STATE_STRING_FORMAT_WITH_EVENT, STATE_STRING_FORMAT_WITHOUT_EVENT);
    stateFsm.setGetStateNameCallback([](uint16_t id) {
      return HUD::StateID::getStateName(id);
    });
    stateFsm.setGetEventNameCallback([](uint16_t ev) {
      return Triggers::getName(ev);
    });
    stateFsm.setTriggeredCallback([](uint16_t tr) {
      Serial.printf(TRIGGER_PRINT_FORMAT, Triggers::getName(tr));
    });
  }

} // namespace HUD
  //-----------------------------------------------------------------------