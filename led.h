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
void led(enum LedColor color);
void led_setting(void);
void led_test(void);
#endif