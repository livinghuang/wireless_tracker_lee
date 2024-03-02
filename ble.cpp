#include "ble.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 1; // In seconds

#define TIMESTAMP_LENGTH 5
#define STEPS_LENGTH 2
#define CALORIES_LENGTH 2
#define HEART_RATE_LENGTH 1
#define SYSTOLIC_BLOOD_PRESSURE_LENGTH 1
#define DIASTOLIC_BLOOD_PRESSURE_LENGTH 1
#define BATTERY_LEVEL_LENGTH 1
#define SOS_LENGTH 1
#define KEY_LENGTH 1
#define BODY_TEMPERATURE_LENGTH 2
#define SLEEP_LENGTH 2
#define SPO2_LENGTH 1
#define LUNCH_LENGTH 1

char paired_watch_mac_address[] = "d5:12:d8:96:00:31";
float bodyTemperature;
uint8_t heartRate, SpO2, watchBatteryLevel;
char ble_mac_address[20];
bool watch_done = false;

struct ble_data
{
  uint8_t timestamp[TIMESTAMP_LENGTH];
  uint8_t steps[STEPS_LENGTH];
  uint8_t calories[CALORIES_LENGTH];
  uint8_t heart_rate[HEART_RATE_LENGTH];
  uint8_t diastolic_blood_pressure[DIASTOLIC_BLOOD_PRESSURE_LENGTH];
  uint8_t systolic_blood_pressure[SYSTOLIC_BLOOD_PRESSURE_LENGTH];
  uint8_t battery_level[BATTERY_LEVEL_LENGTH];
  uint8_t sos[SOS_LENGTH];
  uint8_t key[KEY_LENGTH];
  uint8_t body_temperature[BODY_TEMPERATURE_LENGTH];
  uint8_t sleep[SLEEP_LENGTH];
  uint8_t spo2[SPO2_LENGTH];
  uint8_t lunch[LUNCH_LENGTH];
};

void print_hex(uint8_t *raw, int len)
{
  for (size_t i = 0; i < len; i++)
  {
    Serial.printf("%02X", (unsigned char)raw[i]);
  }
  Serial.println();
}

void ble_parse(uint8_t *raw, int len)
{
  /*
  020105040959 543916FF 180506170D178B0114798958640000016C00005F0007FF D512D8960031 0409595439
  Device name (fixed) : YT9
  Project ID (fixed) : 543916FF
  Coming bytes is the payload example: 180506170D 178B 0114 79 89 58 640000016C00005F0007FFD512D89600310409595439

  TIMESTAMP_LENGTH 5
  STEPS_LENGTH 2
  CALORIES_LENGTH 2
  HEART_RATE_LENGTH 1
  SYSTOLIC_BLOOD_PRESSURE_LENGTH 1
  DIASTOLIC_BLOOD_PRESSURE_LENGTH 1
  BATTERY_LEVEL_LENGTH 1
  SOS_LENGTH 1
  KEY_LENGTH 1
  BODY_TEMPERATURE_LENGTH 2
  SLEEP_LENGTH 2
  SPO2_LENGTH 1
  LUNCH_LENGTH 1
  */

  Serial.print("Payload in Hex: ");
  print_hex(raw + 10, len);

  struct ble_data ble_payload;
  memcpy(&ble_payload, raw + 10, sizeof(ble_payload));

  Serial.print("Timestamp: ");
  print_hex(ble_payload.timestamp, TIMESTAMP_LENGTH);

  Serial.print("Steps: ");
  print_hex(ble_payload.steps, STEPS_LENGTH);
  Serial.println(ble_payload.steps[0] * 256 + ble_payload.steps[1]);

  Serial.print("Calories: ");
  print_hex(ble_payload.calories, CALORIES_LENGTH);
  Serial.println(ble_payload.calories[0] * 256 + ble_payload.calories[1]);

  Serial.print("Heart Rate: ");
  print_hex(ble_payload.heart_rate, HEART_RATE_LENGTH);
  heartRate = ble_payload.heart_rate[0];
  Serial.println(heartRate);

  Serial.print("Systolic Blood Pressure: ");
  print_hex(ble_payload.systolic_blood_pressure, SYSTOLIC_BLOOD_PRESSURE_LENGTH);
  Serial.println(ble_payload.systolic_blood_pressure[0]);

  Serial.print("Diastolic Blood Pressure: ");
  print_hex(ble_payload.diastolic_blood_pressure, DIASTOLIC_BLOOD_PRESSURE_LENGTH);
  Serial.println(ble_payload.diastolic_blood_pressure[0]);

  Serial.print("Battery Level: ");
  print_hex(ble_payload.battery_level, BATTERY_LEVEL_LENGTH);
  watchBatteryLevel = ble_payload.battery_level[0];
  Serial.println(watchBatteryLevel);

  Serial.print("body_temperature: ");
  print_hex(ble_payload.body_temperature, BODY_TEMPERATURE_LENGTH);
  bodyTemperature = float(ble_payload.body_temperature[0] * 256 + ble_payload.body_temperature[1]) / 10;
  Serial.println(bodyTemperature);

  Serial.print("SpO2: ");
  print_hex(ble_payload.spo2, SPO2_LENGTH);
  SpO2 = ble_payload.spo2[0];
  Serial.println(SpO2);
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    // Check if the advertised device name is "YT9"
    if (advertisedDevice.haveName() && advertisedDevice.getName() == "YT9")
    {
      Serial.print("Found Device: ");
      Serial.println(advertisedDevice.toString().c_str());
      Serial.println("Device Address: ");
      // Print the MAC address of the device

      strcpy(ble_mac_address, advertisedDevice.getAddress().toString().c_str());
      Serial.println(ble_mac_address);
      if (strcmp(ble_mac_address, paired_watch_mac_address) == 0)
      {
        watch_done = true;
        ble_parse(advertisedDevice.getPayload(), advertisedDevice.getPayloadLength());
      }
      else
      {
        Serial.println("Device not found!");
      }
    }
  }
};

void ble_init()
{
  Serial.println("Scanning...");
  // Initialize BLE for scanning
  BLEDevice::init("");

  // Create a BLE scanner
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // Active scan uses more power, but gets results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}

void ble_process()
{
  // Start scanning for devices for a specified time
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->start(scanTime, true);
  Serial.println("Scan done!");
  pBLEScan->clearResults(); // Clear results to free memory
}
