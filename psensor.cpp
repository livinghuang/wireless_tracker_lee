#include "bsp.h"
#include "psensor.h"

Dps310 Dps310PressureSensor = Dps310();

void dsp310_init()
{
  pinMode(Vext, OUTPUT);
#ifdef WIFI_TRK_VER_11
  digitalWrite(Vext, HIGH);
#else
  digitalWrite(Vext, LOW);
#endif

  delay(10);

  Wire.setPins(PSENSOR_I2C_SDA, PSENSOR_I2C_SCL);
  // Call begin to initialize Dps310PressureSensor
  // The parameter 0x76 is the bus address. The default address is 0x77 and does not need to be given.
  // Dps310PressureSensor.begin(Wire, 0x76);
  // Use the commented line below instead of the one above to use the default I2C address.
  // if you are using the Pressure 3 click Board, you need 0x76
  Dps310PressureSensor.begin(Wire, 0x77);
  Serial.println("Init complete!");
}

float temperature;
float pressure;
void dsp310_fetch(void)
{

  uint8_t oversampling = 7;
  int16_t ret;
  Serial.println();

  // lets the Dps310 perform a Single temperature measurement with the last (or standard) configuration
  // The result will be written to the paramerter temperature
  // ret = Dps310PressureSensor.measureTempOnce(temperature);
  // the commented line below does exactly the same as the one above, but you can also config the precision
  // oversampling can be a value from 0 to 7
  // the Dps 310 will perform 2^oversampling internal temperature measurements and combine them to one result with higher precision
  // measurements with higher precision take more time, consult datasheet for more information
  ret = Dps310PressureSensor.measureTempOnce(temperature, oversampling);

  if (ret != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" degrees of Celsius");
  }

  // Pressure measurement behaves like temperature measurement
  // ret = Dps310PressureSensor.measurePressureOnce(pressure);
  ret = Dps310PressureSensor.measurePressureOnce(pressure, oversampling);
  if (ret != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" Pascal");
  }

  // Wait some time
  delay(500);
}
