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
#include <FsmManager.h>
#include <constants.h>
#include <VescData.h>
#include <elapsedMillis.h>

#include <NRF24L01Lib.h>
#include <GenericClient.h>

//------------------------------------------------------------------

NRF24L01Lib nrf24;

RF24 radio(NRF_CE, NRF_CS);
RF24Network network(radio);
GenericClient<HUDAction::Event, uint16_t> controllerClient(COMMS_CONTROLLER);

ControllerClass controller;

//------------------------------------------------------------------
bool hudFsmReady = false;

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
  controllerClient.sendTo(Packet::HUD, HUDAction::ONE_CLICK);
}

void buttonDoubleClickHandler(Button2 &btn)
{
  controllerClient.sendTo(Packet::HUD, HUDAction::TWO_CLICK);
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
    controllerClient.sendTo(Packet::HUD, HUDAction::THREE_CLICK);
  }
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

  HUD::createTask(CORE_1, TASK_PRIORITY_1);

  while (!hudFsmReady)
    vTaskDelay(100);

  nrf24.begin(&radio, &network, COMMS_HUD, packetAvailable_cb);

  controllerClient.begin(&network, packetAvailable_cb);
  controllerClient.setConnectedStateChangeCallback(controllerConnectedChange);
  controllerClient.setReadPacketCallback([](uint16_t command) {
    Serial.printf(IN_EVENT_FORMAT_STRING, "RX", HUDCommand1::getMode(command), "CtrlrClient");
  });
  controllerClient.setSentPacketCallback([](HUDAction::Event action) {
    Serial.printf(OUT_EVENT_FORMAT_STRING, "TX", HUDAction::getName(action), "CtrlrClient");
  });

  DEBUG("-----------------------------------------\n\n");

  Serial.printf("---------start of debug---------");
  // using namespace HUDCommand1;
  // uint16_t command = 0;
  // command |= 1 << TWO_FLASHES;
  // command |= 1 << BLUE;
  // command |= 1 << CommandBit::FAST;
  // Serial.printf("command is<TWO_FLASHES>(command): %s %s %s %s\n",
  //               is<TWO_FLASHES>(command) ? "TRUE" : "FALSE",
  //               is<BLUE>(command) ? "TRUE" : "FALSE",
  //               is<CommandBit::FAST>(command) ? "TRUE" : "FALSE",
  //               getMode(command));

  // hudQueue->send(command);

  // uint16_t rx = hudQueue->read<uint16_t>();

  using namespace HUDCommand1;

  Command command(0x00);
  command.set<Command::TWO_FLASHES>();
  command.set<Command::BLUE>();
  command.set<Command::FAST>();

  Serial.printf("command is<TWO_FLASHES>(command): %s %s %s %s\n",
                command.is<Command::Enum::TWO_FLASHES>() ? "TRUE" : "FALSE",
                command.is<Command::Enum::BLUE>() ? "TRUE" : "FALSE",
                command.is<Command::Enum::FAST>() ? "TRUE" : "FALSE",
                command.getmode1());
  Serial.printf("----------end of debug----------");

  controllerClient.sendTo(Packet::HUD, HUDAction::HEARTBEAT);
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
  //   using namespace HUDCommand1;
  //   sinceState1 = 0;
  //   hudQueue->send(1 << THREE_FLASHES | 1 << HUDCommand1::BLUE | 1 << HUDCommand1::FAST);
  // }
  vTaskDelay(10);
}