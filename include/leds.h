
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

enum LedColour
{
  BLUE = 0,
  RED,
  GREEN,
  WHITE,
  BLACK,
};

enum LedSpeed
{
  NONE = 0,
  SLOW,
  FAST,
};

const char *ledColourName(LedColour col)
{
  switch (col)
  {
  case BLUE:
    return "BLUE";
  case RED:
    return "RED";
  case GREEN:
    return "GREEN";
  case WHITE:
    return "WHITE";
  case BLACK:
    return "BLACK";
  }
  return "OUT OF RANGE (ledColourName)";
}

const char *ledSpeedName(LedSpeed speed)
{
  switch (speed)
  {
  case NONE:
    return "NONE";
  case SLOW:
    return "SLOW";
  case FAST:
    return "FAST";
  }
  return "OUT OF RANGE (ledspeedName)";
}

LedColour mapToLedColour(HUD::Command command)
{
  LedColour colour = LedColour::BLACK;
  using namespace HUD;
  if (command.is<HUD::GREEN>())
    colour = LedColour::GREEN;
  else if (command.is<HUD::RED>())
    colour = LedColour::RED;
  else if (command.is<HUD::BLUE>())
    colour = LedColour::BLUE;
  // Serial.printf("Mapped colour to %s\n", ledColourName(colour));
  return colour;
}

LedSpeed mapToLedSpeed(HUD::Command command)
{
  LedSpeed spd = LedSpeed::NONE;
  using namespace HUD;
  if (command.is<HUD::SLOW>())
    spd = LedSpeed::SLOW;
  else if (command.is<HUD::FAST>())
    spd = LedSpeed::FAST;
  return spd;
}

uint8_t mapToNumFlashes(HUD::Command command)
{
  using namespace HUD;
  if (command.is<HUD::TWO_FLASHES>())
    return 2;
  else if (command.is<HUD::THREE_FLASHES>())
    return 3;
  return 1;
}

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
    CRGB col = _getCRGB(_colour);
    for (int num = 0; num < NUM_LEDS; num++)
    {
      leds[num] = col;
    }
    leds[0] = col;
    FastLED.show();
  }

  void setLeds(LedColour colour)
  {
    _colour = colour;
    setLeds();
  }

  LedSpeed getSpeed()
  {
    return _speed;
  }

  void setSpeed(LedSpeed speed)
  {
    _speed = speed;
  }

  unsigned long getSpeedInterval()
  {
    switch (_speed)
    {
    case LedSpeed::SLOW:
      return 500;
    case LedSpeed::FAST:
      return 100;
    default:
      return 0;
    }
  }

  LedColour getColour()
  {
    return _colour;
  }

  void setColour(LedColour col)
  {
    _colour = col;
  }

  bool flashLeds(bool start = false)
  {
    if (start)
    {
      _flashPhase = 0;
      _originalColour = _colour;
    }
    if (_flashPhase % 2 == 0)
      setLeds(_originalColour);
    else
      setLeds(LedColour::BLACK);
    _flashPhase++;
    return _flashPhase == numFlashes * 2;
  }

  uint8_t numFlashes = 0;

protected:
  uint8_t _walkIdx = 0, _walkIdx2 = -1, _walkIdx3 = -2;
  uint8_t _flashPhase = 0;
  LedColour _colour = BLACK, _originalColour = BLACK;
  LedSpeed _speed;

  CRGB _getCRGB(LedColour col)
  {
    switch (col)
    {
    case BLUE:
      return CRGB::DarkBlue;
    case RED:
      return CRGB::DarkRed;
    case GREEN:
      return CRGB::DarkGreen;
    case WHITE:
      return CRGB::White;
    }
    return CRGB::Black;
  }
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
    CRGB col = _getCRGB(_colour);
    for (int i = 0; i < NUM_PIXELS; i++)
    {
      if (i == _walkIdx)
      {
        leds[i] = col;
      }
      else if (i == _walkIdx2)
      {
        leds[i] = col;
        leds[i].fadeLightBy(80);
      }
      else if (i == _walkIdx3)
      {
        leds[i] = col;
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
