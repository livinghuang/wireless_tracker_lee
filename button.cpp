#include "Arduino.h"
#include "button.h"
#include "bsp.h"
void button_setting(void)
{
  pinMode(BUTTON, INPUT);
}

void button_test(void)
{
  int pin = digitalRead(BUTTON);
  String button_string;
  if (pin)
  {
    button_string = "released";
  }
  else
  {
    button_string = "pressed";
  }
  Serial.println(button_string);
  st7735.st7735_fill_screen(ST7735_BLACK);
  st7735.st7735_write_str(0, 40, button_string);
  delay(1000);
}