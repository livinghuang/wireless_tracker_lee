#include "led.h"

void led(enum LedColor color)
{
  pinMode(LED_PIN, OUTPUT);
  delay(100);
  switch (color)
  {
  case RED:
    neopixelWrite(LED_PIN, 255, 0, 0);
    break;
  case GREEN:
    neopixelWrite(LED_PIN, 0, 255, 0);
    break;
  case BLUE:
    neopixelWrite(LED_PIN, 0, 0, 255);
    break;
  case WHITE:
    neopixelWrite(LED_PIN, 255, 255, 255);
    break;
  case OFF:
    neopixelWrite(LED_PIN, 0, 0, 0);
    break;
  }
  delay(1000);
}
void led_setting(void)
{
  pinMode(LED_PIN, OUTPUT);
}

void led_test(void)
{
  // Example: Set WS2812 LED to Red
  neopixelWrite(LED_PIN, 255, 0, 0);
  delay(1000);

  // Example: Set WS2812 LED to Green
  neopixelWrite(LED_PIN, 0, 255, 0);
  delay(1000);

  // Example: Set WS2812 LED to Blue
  neopixelWrite(LED_PIN, 0, 0, 255);
  delay(1000);

  // Example: Set WS2812 LED to White
  neopixelWrite(LED_PIN, 255, 255, 255);
  delay(1000);
}