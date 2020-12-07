#ifndef ESK8_SHARED_UTILS_H
#include <shared-utils.h>
#endif

#define BUTTON_PIN 16

namespace Button
{
  enum Event
  {
    EV_BTN_NONE = 0,
    EV_BTN_DOUBLE_CLICK,
    Length // has to be last, do not remove
  };

  std::string names[] = {
      "EV_BTN_NONE",
      "EV_BTN_DOUBLE_CLICK",
  };

  EnumManager<Button::Event> event(names);
} // namespace Button

#define LED_PULSE_SPEED_MS 600

#define LED_FLASH_LENGTH_MS 300
#define LED_SPIN_SPEED_MED_MS 200
#define LED_SPIN_SPEED_FAST_MS 60

#ifndef USE_DEEPSLEEP
#define USE_DEEPSLEEP 0
#endif

#define COMMS_CONTROLLER 00 // so controller can send to HUD (02)
#define COMMS_BOARD 01
#define COMMS_HUD 02

#define NRF_CE 33
#define NRF_CS 26

#define CONTROLLER_HEARTBEAT_INTERVAL 1000

//------------------------------------------------

#define CORE_0 0
#define CORE_1 1

#define TASK_PRIORITY_1 1
#define TASK_PRIORITY_2 2
#define TASK_PRIORITY_3 3
#define TASK_PRIORITY_4 4
#define TASK_PRIORITY_5 5

#define COLOUR_RED 0
#define COLOUR_BLUE 1
#define COLOUR_YELLOW 2
#define COLOUR_GREEN 3
#define COLOUR_WHITE 4
#define MODE_FLASH 5
#define MODE_PULSE 6
#define MODE_SPIN 7
// #define COLOUR_BLACK 8

//----------------------------
#ifndef ESK8_ENUM_MANAGER
#include <EnumManager.h>
#endif

// EnumManager<HUDCommand::Mode> hudCommandModes(HUDCommand::modeNames);

//----------------------------

enum HUDSpecialEvents
{
  DISCONNECTED = HUDCommand::ModeLength, // makes sure it doesn't conflict with HUDCommand
  CYCLE_BRIGHTNESS,
  SpecialEventLength
};

const char *specialEvent[] = {
    "DISCONNECTED",
    "CYCLE_BRIGHTNESS",
};

void assertHUDSpecialEvents()
{
  assertEnum("HUDSpecialEvents", HUDSpecialEvents::SpecialEventLength, ARRAY_SIZE(specialEvent));
}
//----------------------------

#ifndef PRINT_QUEUE_SEND
#define PRINT_QUEUE_SEND 0
#endif
#ifndef PRINT_QUEUE_READ
#define PRINT_QUEUE_READ 0
#endif
#ifndef PRINT_PACKET_RX
#define PRINT_PACKET_RX 0
#endif
#ifndef PRINT_SEND_ACTION
#define PRINT_SEND_ACTION 0
#endif