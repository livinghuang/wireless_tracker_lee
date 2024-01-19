#include "Arduino.h"
#include "buzzer.h"
#include "bsp.h"
void buzzer_on(void)
{
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, HIGH);
}
void buzzer_off(void)
{
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}

void buzzer_setting(void)
{
  buzzer_off();
}
void buzzer_test(void)
{
  buzzer_on();
  delay(1000);
  buzzer_off();
  delay(1000);
}