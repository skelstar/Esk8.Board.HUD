#ifdef DEBUG_SERIAL
#define DEBUG_OUT Serial
#endif
#define PRINTSTREAM_FALLBACK
// #include <Arduino_Helpers.h>
#include "Debug.hpp"

#include <Arduino.h>
#include <shared-utils.h>
#include <types.h>
#include <QueueManager.h>
#include <constants.h>
#include <VescData.h>
#include <elapsedMillis.h>
#include <NRF24L01Lib.h>

//------------------------------------------------------------------

NRF24L01Lib nrf24;

RF24 radio(NRF_CE, NRF_CS);
RF24Network network(radio);

ControllerClass controller;

//------------------------------------------------------------------

#include <leds.h>
#include <hudFsm.h>
#include <tasks/hudStateTask.h>
#include <Button2.h>
#include <nrf_comms.h>

Button2 button(BUTTON_PIN);

void buttonPressedHandler(Button2 &btn)
{
  using HUDAction::Event;
  sendActionToController(HUDAction::ONE_CLICK);
}

void buttonDoubleClickHandler(Button2 &btn)
{
  using HUDAction::Event;
  sendActionToController(HUDAction::TWO_CLICK);
}

void buttonTripleClickHandler(Button2 &btn)
{
  using HUDAction::Event;
  if (USE_DEEPSLEEP)
  {
    Serial.printf("Going to sleep!...");
    delay(100);
    esp_deep_sleep_start();
  }
  else
  {
    sendActionToController(HUDAction::THREE_CLICK);
  }
}

//-----------------------------------------------

void asserts()
{
  HUDAction::assertThis();
  HUDCommand::assertThis();
  Packet::assertThis();
  Button::assertThis();
  assertHUDSpecialEvents();
}

//-----------------------------------------------

void setup()
{
  Serial.begin(115200);
  DEBUG("-----------------------------------------\n");
  Serial.printf("Esk8.Board.HUD ready!\n");
  DEBUG("-----------------------------------------\n");

  if (USE_DEEPSLEEP)
    esp_sleep_enable_ext1_wakeup(GPIO_NUM_39, ESP_EXT1_WAKEUP_ALL_LOW);

  button.setClickHandler(buttonPressedHandler);
  button.setDoubleClickHandler(buttonDoubleClickHandler);
  button.setTripleClickHandler(buttonTripleClickHandler);

  createLedsTask(CORE_1, TASK_PRIORITY_1);

  nrf24.begin(&radio, &network, COMMS_HUD, packetAvailable_cb);
  DEBUG("-----------------------------------------\n\n");

  sendActionToController(HUDAction::HEARTBEAT);
}
//-----------------------------------------------

elapsedMillis sincePulse, sinceSentToClient, sinceCheckedNRF, sinceState1;

void loop()
{
  button.loop();

  if (sinceCheckedNRF > 100)
  {
    sinceCheckedNRF = 0;
    nrf24.update();
  }

  delay(10);
}