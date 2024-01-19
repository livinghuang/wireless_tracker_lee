#define _BSP_H
#ifdef _BSP_H
//---------------------------------------------------------------------------------------//
#define WIFI_TRK_VER_11 // If your board is Version 1.1, please choice WIFI_TRK_VER_11

// #define WIFI_TRK_VER_10 // If your board is Version 1.0, please choice WIFI_TRK_VER_10
//---------------------------------------------------------------------------------------//

#ifdef WIFI_TRK_VER_11
#include "HT_st7735.h"
extern HT_st7735 st7735;
#define VGNSS_CTRL Vext
#else
#include "HT_st7736.h"
extern HT_st7736 st7735;
#define VGNSS_CTRL 37
#endif

#define ADC_Ctrl 2
#define ADC_IN 1
// Define the GPIO pin where the WS2812 LED data input is connected
#define LED_PIN 17 // Replace with your GPIO pin number
#define CS_PIN 21
#define SCK_PIN 9
#define MOSI_PIN 11
#define MISO_PIN 10
#define LORA_RST_PIN 12
#define GSENSOR_I2C_SDA 16
#define GSENSOR_I2C_SCK 15
#define BUZZER 46
#define BUTTON 4

#endif