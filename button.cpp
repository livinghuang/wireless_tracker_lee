#include "Arduino.h"
#include "button.h"

void button_setting(void)
{
  pinMode(BUTTON, INPUT);
}

void button_test(void)
{
  int pin = digitalRead(BUTTON);
  if (pin)
  {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
  }
  else
  {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
  }
}