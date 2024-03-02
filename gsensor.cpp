#include "bsp.h"
#include "Arduino.h"
#include "LoRaWan_APP.h"
#include "gsensor.h"
#include "MPU6050.h"

MPU6050_Base MPU;

void gsensor_test(void)
{
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  MPU.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  Serial.print("A: ");
  Serial.print(ax);
  Serial.print(" ");
  Serial.print(ay);
  Serial.print(" ");
  Serial.print(az);
  Serial.print(" ");

  Serial.print("G: ");
  Serial.print(gx);
  Serial.print(" ");
  Serial.print(gy);
  Serial.print(" ");
  Serial.println(gz);

  delay(500);
}

void gsensor_init()
{
  pinMode(Vext, OUTPUT);
#ifdef WIFI_TRK_VER_11
  digitalWrite(Vext, HIGH);
#else
  digitalWrite(Vext, LOW);
#endif

  delay(10);

  Wire.begin(GSENSOR_I2C_SDA, GSENSOR_I2C_SCL, 100000);
  Wire.setClock(100000);

  // start test
  // Initialize communication with the MPU6050
  // Wire.begin(); // For boards with dedicated SDA, SCL pins
  // Wire.begin(SDA_PIN, SCL_PIN); // Use this for boards without dedicated I2C pins
  // Wire.setClock(400000); // Set I2C clock speed to 400kHz

  MPU.initialize(); // Initialize the MPU6050
  if (!MPU.testConnection())
  {
    Serial.println("MPU6050 connection failed");
    delay(1000);
    // while (1)
    //   ;
  }
  Serial.println("MPU6050 connection successful");
}