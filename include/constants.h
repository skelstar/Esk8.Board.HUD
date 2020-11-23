

enum LedsStateEvent
{
  EV_LED_CONNECTED = 0,
  EV_LED_PULSE_RED,
  EV_LED_FLASH_GREEN,
  EV_LED_SPIN_GREEN_FAST,
  EV_LED_DISCONNECTED,
  EV_LED_IDLE,
};

const char *ledStateEventNames[] = {
    "EV_LED_CONNECTED",
    "EV_LED_PULSE_RED",
    "EV_LED_FLASH_GREEN",
    "EV_LED_SPIN_GREEN_FAST",
    "EV_LED_DISCONNECTED",
    "EV_LED_IDLE",
};

enum ButtonEvent
{
  EV_BTN_NONE = 0,
  EV_BTN_DOUBLE_CLICK,
};

const char *ButtonEventNames[] = {
    "EV_BTN_NONE",
    "EV_BTN_DOUBLE_CLICK",
};

#define LED_SPIN_SPEED_MED_MS 200
#define LED_SPIN_SPEED_FAST_MS 60

#ifndef USE_DEEPSLEEP
#define USE_DEEPSLEEP 0
#endif

elapsedMillis sinceUpdatedPerimeter;
