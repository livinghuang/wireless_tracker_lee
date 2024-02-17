#include "bsp.h"
#include "Arduino.h"
#include "battery.h"

/*!
 * ADC maximum value
 */
#define BAT_MAX_VALUE 793 // 4.2V
#define BAT_MIN_VALUE 588 // 3.3V

uint32_t adc5Voltage = 0;
uint16_t ADC5VMeasureVolage(void)
{
  uint32_t vDiv = analogReadMilliVolts(ADC_5V);

  // Divider bridge V5<->510k - <-- | --> - 510k <->GND => adc5Voltage = 2 * vDiv
  adc5Voltage = 2 * vDiv;
  // adc5Voltage = adc5Voltage;
  delay(2);
  Serial.println(adc5Voltage);
  delay(50);
  return adc5Voltage;
}

void adc5v_test(void)
{
  ADC5VMeasureVolage();
}