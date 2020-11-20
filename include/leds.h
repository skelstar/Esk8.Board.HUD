
#include <FastLED.h>

#define NUM_LEDS 25
#define LED_PIN 27

enum LedSection
{
  BOARD,
  CONNECTION,
  BOTTOM_LEFT,
  BOTTOM_RIGHT
};

CRGB leds[NUM_LEDS];

void setLeds(CRGB colour)
{
  for (int num = 0; num < NUM_LEDS; num++)
  {
    leds[num] = colour;
  }
  FastLED.show();
}

// 00 01 02 03 04
// 05 06 07 08 09
// 10 11 12 13 14
// 15 16 17 18 19
// 20 21 22 23 24

const uint8_t outerRing[5][5] =
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}};

const uint8_t middleDots[5][5] =
    {
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 1, 1, 1, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0}};

void setLeds(LedSection section, CRGB colour)
{
  for (int num = 0; num < NUM_LEDS; num++)
  {
    switch (section)
    {
    case BOARD:
      if (num == 3 || num == 4 || num == 8 || num == 9)
        leds[num] = colour;
      break;
    case CONNECTION:
      if (num == 0 || num == 1 || num == 5 || num == 6)
        leds[num] = colour;
      break;
    case BOTTOM_LEFT:
      if (num == 15 || num == 16 || num == 20 || num == 21)
        leds[num] = colour;
      break;
    case BOTTOM_RIGHT:
      if (num == 18 || num == 19 || num == 23 || num == 24)
        leds[num] = colour;
      break;
    }
  }
  FastLED.show();
}

void setPatternLeds(const uint8_t pattern[5][5], CRGB colour)
{
  for (uint8_t x = 0; x < 5; x++)
  {
    for (int y = 0; y < 5; y++)
    {
      uint8_t num = x * 5 + y;
      leds[num] = pattern[x][y] == 1 ? colour : CRGB::Black;
    }
  }
  FastLED.show();
}

void setMiddleLeds(LedSection section, CRGB colour)
{
  for (int num = 0; num < NUM_LEDS; num++)
  {
    switch (section)
    {
    case BOARD:
      if (num == 3 || num == 4 || num == 8 || num == 9)
        leds[num] = colour;
      break;
    case CONNECTION:
      if (num == 0 || num == 1 || num == 5 || num == 6)
        leds[num] = colour;
      break;
    case BOTTOM_LEFT:
      if (num == 15 || num == 16 || num == 20 || num == 21)
        leds[num] = colour;
      break;
    case BOTTOM_RIGHT:
      if (num == 18 || num == 19 || num == 23 || num == 24)
        leds[num] = colour;
      break;
    }
  }
  FastLED.show();
}
