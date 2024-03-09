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

#define DIO1 5
#define DIO2 6
#define PMC 6

#define LED_PIN 17 // Replace with your GPIO pin number

#define PSENSOR_I2C_SCL 15
#define PSENSOR_I2C_SDA 16

#define GSENSOR_I2C_SCL 15
#define GSENSOR_I2C_SDA 16
#define GSENSOR_INT 18

#define SD_CD 21
#define SD_MOSI 11
#define SD_MISO 10

#define BUZZER 46

#define SD_SCK 9

#define BUTTON 4

#define ADC_5V 7

#define ADC_Ctrl 2
#define ADC_IN 1

#define CS_PIN 21
#define SCK_PIN 9
#define MOSI_PIN 11
#define MISO_PIN 10
#define LORA_RST_PIN 12
#define SCREEN_BL 45
// #define DIO3 7 // no use in version 3.0, it connect to ADC_5V
