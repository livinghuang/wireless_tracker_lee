#include "esp32-hal-rgb-led.h"
#include "bsp.h"

#define LED_DATA_LENGTH 27
#define LED_NUMS 4
enum COLOR
{
  OFF = 0,
  RED = 2,
  GREEN = 1,
  BLUE = 3,
  WHITE = 4
};
#define RMT_BUFFER_SIZE LED_DATA_LENGTH *LED_NUMS + 1
rmt_data_t rmt_buffer[RMT_BUFFER_SIZE];
int rmt_buffer_index = 0;
void setResetBit()
{
  rmt_buffer[rmt_buffer_index].level0 = 1;      // T1H
  rmt_buffer[rmt_buffer_index].duration0 = 100; // 0.8us
  rmt_buffer[rmt_buffer_index].level1 = 0;      // T1L
  rmt_buffer[rmt_buffer_index].duration1 = 300; // 0.4us
  rmt_buffer_index++;
}

/*
T0H 0 码， 高电平时间 220ns~380ns
T1H 1 码， 高电平时间 580ns~1µs
T0L 0 码， 低电平时间 580ns~1µs
T1L 1 码， 低电平时间 580ns~1µs
RES 帧单位，低电平时间 280µs 以上
*/

void setBit(int bitVal)
{
  if (bitVal)
  {
    // HIGH bit
    rmt_buffer[rmt_buffer_index].level0 = 1;    // T1H
    rmt_buffer[rmt_buffer_index].duration0 = 8; // 0.8us
    rmt_buffer[rmt_buffer_index].level1 = 0;    // T1L
    rmt_buffer[rmt_buffer_index].duration1 = 4; // 0.4us
  }
  else
  {
    // LOW bit
    rmt_buffer[rmt_buffer_index].level0 = 1;    // T0H
    rmt_buffer[rmt_buffer_index].duration0 = 4; // 0.4us
    rmt_buffer[rmt_buffer_index].level1 = 0;    // T0L
    rmt_buffer[rmt_buffer_index].duration1 = 8; // 0.8us
  }
}
void setByte(uint8_t byte)
{
  for (int bit = 0; bit < 8; bit++)
  {
    setBit((byte & (1 << (7 - bit))));
    rmt_buffer_index++;
  }
  rmt_buffer[rmt_buffer_index].level0 = 0;    // T0H
  rmt_buffer[rmt_buffer_index].duration0 = 4; // 0.4us
  rmt_buffer[rmt_buffer_index].level1 = 0;    // T0L
  rmt_buffer[rmt_buffer_index].duration1 = 8; // 0.8us
  rmt_buffer_index++;
}
void setRGB(uint8_t red, uint8_t green, uint8_t blue, int led_num)
{
  rmt_buffer_index = led_num * LED_DATA_LENGTH;
  setByte(green);
  setByte(red);
  setByte(blue);
}

void setLedColor(enum COLOR color, int led_num)
{
  int strength = 255;
  switch (color)
  {
  case OFF:;
    setRGB(0, 0, 0, led_num);
    break;
  case RED:
    setRGB(strength, 0, 0, led_num);
    break;
  case GREEN:
    setRGB(0, strength, 0, led_num);
    break;
  case BLUE:
    setRGB(0, 0, strength, led_num);
    break;
  case WHITE:
    setRGB(strength, strength, strength, led_num);
    break;
  }
}

void printLED(COLOR color)
{
  static rmt_obj_t *rmt_send = NULL;
  float tick_rate_hz;
  memset(rmt_buffer, 0, sizeof(rmt_buffer));
  if (rmt_send == NULL)
  {
    if ((rmt_send = rmtInit(LED_PIN, RMT_TX_MODE, RMT_MEM_128)) == NULL)
    {
      Serial.println("RGB LED driver initialization failed!");
      rmt_send = NULL;
      return;
    }
  }
  tick_rate_hz = rmtSetTick(rmt_send, 100);
  Serial.printf("RGB LED tick rate: %f Hz\n", tick_rate_hz);

  setLedColor(color, 0);
  setLedColor(color, 1);
  // setLedColor(color, 2);
  setResetBit();
  for (int i = 0; i < RMT_BUFFER_SIZE; i++)
  {
    Serial.printf("rmt_buffer[%d]: ", i);
    Serial.printf("level0: %d, duration0: %d, level1: %d, duration1: %d \n", rmt_buffer[i].level0, rmt_buffer[i].duration0, rmt_buffer[i].level1, rmt_buffer[i].duration1);
  }
  Serial.printf("rmt_buffer_index: %d \n", rmt_buffer_index);
  rmtWrite(rmt_send, rmt_buffer, rmt_buffer_index);
  delay(30);
}

void led_test(void)
{
  static int i = 0;
  i++;
  printLED((COLOR(i % 5)));
  delay(1000);
}
