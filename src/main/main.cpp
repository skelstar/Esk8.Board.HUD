#ifdef DEBUG_SERIAL
#define DEBUG_OUT Serial
#endif
#define PRINTSTREAM_FALLBACK
#include <Arduino_Helpers.h>
#include "Debug.hpp"

#include <Arduino.h>
#include <VescData.h>
#include <elapsedMillis.h>

#define COMMS_BOARD 00
#define COMMS_CONTROLLER 01
#define COMMS_HUD 02

enum LedsStateEvent
{
  EV_LED_CONNECTED = 0,
  EV_LED_PULSE_RED,
  EV_LED_FLASH_GREEN,
  EV_LED_DISCONNECTED,
  EV_LED_IDLE,
};

const char *ledStateEventNames[] = {
    "EV_LED_CONNECTED",
    "EV_LED_PULSE_RED",
    "EV_LED_FLASH_GREEN",
    "EV_LED_DISCONNECTED",
    "EV_LED_IDLE",
};

class Hud
{
public:
  bool clientConnected;
} hud;

//------------------------------------------------------------------

#include <HUDData.h>
#include <EventQueueManager.h>

bool packetReady;
HUDData hudData;

xQueueHandle xLedsEventQueue;

EventQueueManager *ledsQueueManager;

#include <leds.h>
#include <hudTask.h>
#include <bleNotify.h>

#define DOUBLECLICK_MS 300
#define LONGCLICK_MS 1000

#include <Button2.h>

Button2 button(39);

#define BRIGHTNESS_LEVELS 4
uint8_t brightnesses[BRIGHTNESS_LEVELS] = {10, 30, 100, 255};
uint8_t brightnessIndex = 1;

ControllerClass controller;

void setup()
{
  Serial.begin(115200);
  Serial.printf("Esk8.Board.HUD ready!\n");

  FastLED.setBrightness(brightnesses[brightnessIndex]);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  setLeds(CRGB::Black);

  button.setTapHandler([](Button2 &btn) {
    brightnessIndex++;
    if (brightnessIndex == BRIGHTNESS_LEVELS)
      brightnessIndex = 0;
    FastLED.setBrightness(brightnesses[brightnessIndex]);
    FastLED.show();
  });

  // core 0
  xTaskCreatePinnedToCore(ledTask_1, "ledTask_1", 10000, NULL, /*priority*/ 3, NULL, /*core*/ 1);

  xLedsEventQueue = xQueueCreate(/*len*/ 5, sizeof(uint8_t));

  ledsQueueManager = new EventQueueManager(xLedsEventQueue, /*ticks*/ 5);

  setupBLE();
}

elapsedMillis sincePulse, sinceSentToClient;

void loop()
{
  button.loop();

  delay(10);
}