#ifdef DEBUG_SERIAL
#define DEBUG_OUT Serial
#endif
#define PRINTSTREAM_FALLBACK
// #include <Arduino_Helpers.h>
#include "Debug.hpp"

#include <Arduino.h>
#include <types.h>
#include <shared-utils.h>
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

#include <EventQueueManager.h>

bool packetReady;

#include <leds.h>
#include <hudState.h>
#include <tasks/hudStateTask.h>
#include <Button2.h>
#include <nrf_comms.h>

Button2 button(BUTTON_PIN);

void buttonPressedHandler(Button2 &btn)
{
  hudStateQueue->send(HUD_CMD_CYCLE_BRIGHTNESS);
}

void buttonDoubleClickHandler(Button2 &btn)
{
  sendActionToController(HUD_ACTION_DOUBLE_CLICK);
}

void buttonTripleClickHandler(Button2 &btn)
{
  if (USE_DEEPSLEEP)
  {
    Serial.printf("Going to sleep!...");
    delay(100);
    esp_deep_sleep_start();
  }
  else
  {
    sendActionToController(HUD_ACTION_TRIPLE_CLICK);
  }
}

//-----------------------------------------------
void asserts()
{
  assertEnum("LedsStateEvent", LedsStateEvent::EV_LED_Length, ARRAY_SIZE(ledStateEventNames));
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

  button.setPressedHandler(buttonPressedHandler);
  button.setDoubleClickHandler(buttonDoubleClickHandler);
  button.setTripleClickHandler(buttonTripleClickHandler);

  createLedsTask(CORE_1, TASK_PRIORITY_1);

  nrf24.begin(&radio, &network, COMMS_HUD, packetAvailable_cb);
  DEBUG("-----------------------------------------\n\n");

  sendActionToController(HUD_ACTION_HEARTBEAT);
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