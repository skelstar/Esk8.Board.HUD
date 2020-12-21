#ifdef DEBUG_SERIAL
#define DEBUG_OUT Serial
#endif
#define PRINTSTREAM_FALLBACK
// #include <Arduino_Helpers.h>
#include "Debug.hpp"

#include <Arduino.h>
#include <shared-utils.h>
#include <types.h>
#include <printFormatStrings.h>
#include <QueueManager.h>
#include <FsmManager.h>
#include <constants.h>
#include <VescData.h>
#include <elapsedMillis.h>

#include <NRF24L01Lib.h>
#include <GenericClient.h>

//------------------------------------------------------------------
bool hudFsmReady = false;

NRF24L01Lib nrf24;

RF24 radio(NRF_CE, NRF_CS);
RF24Network network(radio);

ControllerClass controller;

//------------------------------------------------------------------

#include <leds.h>
#include <Button2.h>
#include <hudFsm.h>
#include <tasks/hudStateTask.h>

//------------------------------------------------------------------

GenericClient<HUDAction::Event, HUD::Instruction> controllerClient(COMMS_CONTROLLER);

#include <nrf_comms.h>

void controllerConnectedChange()
{
  Serial.printf(CLIENT_CONNECT_CHANGE_FORMAT_STRING,
                controllerClient.connected()
                    ? "<----->"
                    : "-- | --");
}

void printTxPacket(HUDAction::Event action)
{
  if (PRINT_PACKET_TX)
    Serial.printf(PRINT_TX_PACKET_TO_FORMAT, "CTRLR", HUDAction::getName(action));
}
void printRxPacket(HUD::Instruction instruction)
{
  if (PRINT_PACKET_RX)
    Serial.printf(PRINT_RX_PACKET_FROM_FORMAT, "CTRLR", instruction.getMode());
}

void controllerClientInit()
{
  controllerClient.begin(&network, packetAvailable_cb);
  controllerClient.setConnectedStateChangeCallback(controllerConnectedChange);
  controllerClient.setSentPacketCallback(printTxPacket);
  controllerClient.setReadPacketCallback(printRxPacket);
}

//------------------------------------------------------------------

Button2 button(BUTTON_PIN);

void buttonPressedHandler(Button2 &btn)
{
  using HUDAction::Event;
  controllerClient.sendTo(Packet::HUD, HUDAction::Event::ONE_CLICK);
}

void buttonDoubleClickHandler(Button2 &btn)
{
  controllerClient.sendTo(Packet::HUD, HUDAction::Event::TWO_CLICK);
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
    controllerClient.sendTo(Packet::HUD, HUDAction::Event::THREE_CLICK);
  }
}
//-----------------------------------------------

void setup()
{
  Serial.begin(115200);
  DEBUG("-----------------------------------------\n");
  DEBUG("          Esk8.Board.HUD ready!\n");
  DEBUG("-----------------------------------------\n");

  if (USE_DEEPSLEEP)
    esp_sleep_enable_ext1_wakeup(GPIO_NUM_39, ESP_EXT1_WAKEUP_ALL_LOW);

  button.setClickHandler(buttonPressedHandler);
  button.setDoubleClickHandler(buttonDoubleClickHandler);
  button.setTripleClickHandler(buttonTripleClickHandler);

  HUD::createTask(CORE_1, TASK_PRIORITY_1);

  while (!hudFsmReady)
    vTaskDelay(100);

  nrf24.begin(&radio, &network, COMMS_HUD, packetAvailable_cb);

  controllerClientInit();

  controllerClient.sendTo(Packet::HUD, HUDAction::Event::HEARTBEAT);
}
//-----------------------------------------------

elapsedMillis sincePulse, sinceSentToClient, sinceCheckedNRF, sinceState1;

void loop()
{
  button.loop();

  if (sinceCheckedNRF > 100 && hudFsmReady)
  {
    sinceCheckedNRF = 0;
    controllerClient.update();
  }

  // if (sinceState1 > 10000)
  // {
  //   using namespace HUD;
  //   sinceState1 = 0;
  //   hudQueue->send(1 << THREE_FLASHES | 1 << HUD::BLUE | 1 << HUD::FAST);
  // }
  vTaskDelay(10);
}