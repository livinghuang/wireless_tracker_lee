#ifndef _LED_H
#define _LED_H
#include "bsp.h"
#include "esp32-hal-rgb-led.h"
#include "esp32-hal.h"
enum COLOR
{
  OFF = 0,
  RED = 2,
  GREEN = 1,
  BLUE = 3,
  WHITE = 4
};

void led_test(void);
void COLOR_LED(COLOR color);
#endif