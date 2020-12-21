#ifndef ARDUINO_H
#include <Arduino.h>
#endif

//-----------------------------------------------------------------------

Queue::Manager *hudQueue;

void hudQueueInit()
{
  hudQueue = new Queue::Manager(/*len*/ 5, sizeof(HUD::Instruction), /*ticks*/ 5, 0);
  hudQueue->setSentEventCallback([](uint16_t ev) {
    HUD::Instruction c(ev);
    if (PRINT_QUEUE_SEND)
      Serial.printf(PRINT_QUEUE_SEND_FORMAT, c.getInstruction(), "HUD");
  });
  hudQueue->setReadEventCallback([](uint16_t ev) {
    HUD::Instruction c(ev);
    if (PRINT_QUEUE_READ)
      Serial.printf(PRINT_QUEUE_READ_FORMAT, "HUD", c.getInstruction());
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
        using namespace HUD;
        uint16_t instruction = hudQueue->read<uint16_t>();

        if (instruction != 0)
        {
          ledDisplay->setColour(mapToLedColour(instruction));
          ledDisplay->setSpeed(mapToLedSpeed(instruction));
          ledDisplay->numFlashes = mapToNumFlashes(instruction);

          stateFsm.trigger(HUD::Triggers::mapToTriggers(instruction));
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

  void printFsmState(uint16_t id)
  {
    if (PRINT_FSM_STATE)
      Serial.printf(PRINT_STATE_FORMAT, "STATE", HUD::StateID::getStateName(id));
  }
  void printFsmTrigger(uint16_t ev)
  {
    if (PRINT_FSM_TRIGGER)
      Serial.printf(PRINT_sFSM_sTRIGGER_FORMAT, "STATE", Triggers::getName(ev));
  }

  void setupFsm()
  {
    addTransitions();

    stateFsm.begin(&fsm);
    stateFsm.setPrintStateCallback(printFsmState);
    stateFsm.setTriggeredCallback(printFsmTrigger);
  } // namespace HUD

} // namespace HUD
  //-----------------------------------------------------------------------