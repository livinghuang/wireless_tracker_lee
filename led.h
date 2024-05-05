#ifndef _LED_H
#define _LED_H
#include "bsp.h"
#include "esp32-hal-rgb-led.h"
enum LedColor
{
  RED,
  GREEN,
  BLUE,
  WHITE,
  OFF
};

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp32-hal.h"

#ifndef RGB_BRIGHTNESS
#define RGB_BRIGHTNESS 64
#endif

  void _neopixelWrite(uint8_t pin, uint8_t *red_vals, uint8_t *green_vals, uint8_t *blue_vals, int num_leds);

#ifdef __cplusplus
}

void led(enum LedColor color);
void led_setting(void);
void led_test(void);

#endif

#endif