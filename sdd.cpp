#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define CS_PIN 21
#define SCK_PIN 9
#define MOSI_PIN 11
#define MISO_PIN 10
#define LORA_RST_PIN 12

bool sdInitialized = false;
bool sdd_setting();
void sd_setting()
{
  Serial.begin(115200);
  pinMode(46, OUTPUT);
  digitalWrite(46, LOW);
  pinMode(LORA_RST_PIN, OUTPUT);
  digitalWrite(LORA_RST_PIN, LOW);

  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);

  sdInitialized = sdd_setting();

  if (sdInitialized)
  {
    Serial.println("SD卡初始化成功");
  }
  else
  {
    Serial.println("SD卡初始化失敗");
  }
}

void sd_test()
{
  sd_setting();
  delay(100);
  // 主要迴圈程式碼放在這裡
  File file = SD.open("/test.txt");
  if (file)
  {
    Serial.println("Successfully opened file:");
    while (file.available())
    {
      Serial.write(file.read());
    }
    file.close();
  }
  else
  {
    Serial.println("Failed to open file");
  }

  File writeFile = SD.open("/test.txt", FILE_WRITE);
  if (writeFile)
  {
    String data = "Hello, SD card ^_^";
    writeFile.println(data);
    writeFile.close();
    Serial.println("Successfully wrote to file");
  }
  else
  {
    Serial.println("Failed to write to file");
  }

  delay(1000);
}

bool sdd_setting()
{
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  if (!SD.begin(CS_PIN, SPI))
  {
    return false;
  }

  return true;
}
