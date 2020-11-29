
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

#define NUM_BRIGHTNESS_LEVELS 4
uint8_t brightnesses[NUM_BRIGHTNESS_LEVELS] = {10, 30, 100, 255};
uint8_t brightnessIndex = 1;

CRGB leds[NUM_LEDS];

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

const uint8_t clockwisePerimeter[] = {0, 1, 2, 3, 4, 9, 14, 19, 24, 23, 22, 21, 20, 15, 10, 5};
#define NUM_PERIMETER_LEDS 15
const uint8_t clockwiseInnerPerimeter[] = {6, 7, 8, 13, 18, 17, 16, 11};
#define NUM_INNER_PERIMETER_LEDS 7

//--------------------------------------------
class LedBaseClass
{
public:
  virtual void animation() = 0;

  void cycleBrightness()
  {
    brightnessIndex++;
    if (brightnessIndex == NUM_BRIGHTNESS_LEVELS)
      brightnessIndex = 0;
    FastLED.setBrightness(brightnesses[brightnessIndex]);
    FastLED.show();
    Serial.printf("brightness now %d\n", brightnesses[brightnessIndex]);
  }

  void setLeds()
  {
    for (int num = 0; num < NUM_LEDS; num++)
    {
      leds[num] = _colour;
    }
    leds[0] = _colour;
    FastLED.show();
  }

  void setLeds(CRGB colour)
  {
    _colour = colour;
    for (int num = 0; num < NUM_LEDS; num++)
    {
      leds[num] = _colour;
    }
    leds[0] = _colour;
    FastLED.show();
  }

  HUDCommand::Speed getSpeed()
  {
    return _speed;
  }

  void setSpeed(HUDCommand::Speed speed)
  {
    _speed = speed;
  }

  unsigned long getSpeedInterval()
  {
    switch (_speed)
    {
    case HUDCommand::SLOW:
      return 1000;
    case HUDCommand::MED:
      return 600;
    case HUDCommand::FAST:
      return 150;
    default:
      Serial.printf("WARNING: NO_SPEED was selected\n");
      return 0;
    }
  }

  CRGB getColour()
  {
    return _colour;
  }

  void setColour(CRGB colour)
  {
    _colour = colour;
  }

  void setColour(HUDCommand::Colour colour)
  {
    switch (colour)
    {
    case HUDCommand::RED:
      _colour = CRGB::Red;
      break;
    case HUDCommand::GREEN:
      _colour = CRGB::Green;
      break;
    case HUDCommand::YELLOW:
      _colour = CRGB::Yellow;
      break;
    case HUDCommand::BLUE:
      _colour = CRGB::Blue;
      break;
    case HUDCommand::WHITE:
      _colour = CRGB::White;
      break;
    default:
      _colour = CRGB::Black;
    }
  }

protected:
  uint8_t _walkIdx = 0, _walkIdx2 = -1, _walkIdx3 = -2;
  CRGB _colour = CRGB::Black;
  HUDCommand::Speed _speed;
};
//--------------------------------------------

class MatrixLedClass : public LedBaseClass
{
public:
  void animation()
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      if (i == clockwiseInnerPerimeter[_walkIdx])
      {
        leds[i] = _colour;
      }
      else if (i == clockwiseInnerPerimeter[_walkIdx2])
      {
        leds[i] = _colour;
        leds[i].fadeLightBy(80);
      }
      else if (i == clockwiseInnerPerimeter[_walkIdx3])
      {
        leds[i] = _colour;
        leds[i].fadeLightBy(150);
      }
      else
      {
        leds[i] = CRGB::Black;
      }
    }
    _walkIdx3 = _walkIdx2;
    _walkIdx2 = _walkIdx;
    _walkIdx = _walkIdx > NUM_INNER_PERIMETER_LEDS - 1 ? 0 : _walkIdx + 1;
    FastLED.show();
  }
};
//--------------------------------------------

class StripLedClass : public LedBaseClass
{
public:
  void animation()
  {
    for (int i = 0; i < NUM_PIXELS; i++)
    {
      if (i == _walkIdx)
      {
        leds[i] = _colour;
      }
      else if (i == _walkIdx2)
      {
        leds[i] = _colour;
        leds[i].fadeLightBy(80);
      }
      else if (i == _walkIdx3)
      {
        leds[i] = _colour;
        leds[i].fadeLightBy(150);
      }
      else
      {
        leds[i] = CRGB::Black;
      }
    }
    _walkIdx3 = _walkIdx2;
    _walkIdx2 = _walkIdx;
    _walkIdx = _walkIdx > NUM_PIXELS - 1 ? 0 : _walkIdx + 1;
    FastLED.show();
  }
};

StripLedClass *ledDisplay;
