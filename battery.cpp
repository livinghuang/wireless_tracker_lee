#include "Arduino.h"
#include "battery.h"

#define ADC_Ctrl 2
#define ADC_IN 1

/*!
 * ADC maximum value
 */
#define BAT_MAX_VALUE 793 // 4.2V
#define BAT_MIN_VALUE 588 // 3.3V

uint32_t batteryVoltage = 0;
uint16_t BatteryMeasureVolage(void)
{
  uint32_t vDiv = analogReadMilliVolts(ADC_IN);

  // Divider bridge VBAT<->390k - <-- | --> - 100k <->GND => vBat = 4.9 * vDiv
  batteryVoltage = 49 * vDiv;
  batteryVoltage = batteryVoltage / 10 + 320;
  delay(2);
  Serial.println(batteryVoltage);
  delay(50);
  return batteryVoltage;
}

void battery_init(void)
{
  pinMode(ADC_Ctrl, OUTPUT);
  digitalWrite(ADC_Ctrl, HIGH);
}
void battery_test(void)
{
  BatteryMeasureVolage();
}