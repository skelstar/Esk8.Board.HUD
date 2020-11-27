

enum LedsStateEvent
{
  EV_LED_CONNECTED = 0,
  EV_LED_PULSE_RED,
  EV_LED_FLASH_GREEN,
  EV_LED_SPIN_GREEN_FAST,
  EV_LED_DISCONNECTED,
  EV_LED_IDLE,
  EV_LED_CYCLE_BRIGHTNESS,
};

const char *ledStateEventNames[] = {
    "EV_LED_CONNECTED",
    "EV_LED_PULSE_RED",
    "EV_LED_FLASH_GREEN",
    "EV_LED_SPIN_GREEN_FAST",
    "EV_LED_DISCONNECTED",
    "EV_LED_IDLE",
    "EV_LED_CYCLE_BRIGHTNESS",
};

#define BUTTON_PIN 16

enum ButtonEvent
{
  EV_BTN_NONE = 0,
  EV_BTN_DOUBLE_CLICK,
};

#define DOUBLECLICK_MS 300
#define LONGCLICK_MS 1000

const char *ButtonEventNames[] = {
    "EV_BTN_NONE",
    "EV_BTN_DOUBLE_CLICK",
};

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
