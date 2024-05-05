#include "bsp.h"
#include "Arduino.h"
#include "WiFi.h"
#include "LoRaWan_APP.h"
#include <Wire.h>

#include "HT_TinyGPS++.h"
#include "sdd.h"
#include "led.h"
#include "button.h"
#include "buzzer.h"
#include "gsensor.h"
#include "psensor.h"
#include "battery.h"
#include "adc5v.h"
#include "ble.h"
#include "lorawan.h"

#ifdef WIFI_TRK_VER_11
#include "HT_st7735.h"
HT_st7735 st7735;
#else
#include "HT_st7736.h"
HT_st7736 st7735;
#endif
typedef enum
{
	WIFI_CONNECT_TEST_INIT,
	WIFI_CONNECT_TEST,
	WIFI_SCAN_TEST,
	LORA_TEST_INIT,
	LORA_COMMUNICATION_TEST,
	DEEPSLEEP_TEST,
	GPS_TEST,
	LED_TEST,
	BUZZER_TEST,
	SD_INIT,
	SD_TEST,
	LORAWAN_INIT,
	LORAWAN_TEST,
	MPU_TEST_INIT,
	MPU_TEST,
	DPS_TEST_INIT,
	DPS_TEST,
	BAT_INIT,
	BAT_TEST,
	BUTTON_TEST,
	BLE_TEST_INIT,
	BLE_TEST,
	DI_TEST,
	ADC_5V_TEST,
	TRACKER_V3_TEST
} test_status_t;

TinyGPSPlus gps;

test_status_t test_status;
uint16_t wifi_connect_try_num = 15;
bool resendflag = false;
bool deepsleepflag = false;
bool interrupt_flag = false;
/********************************* lora  *********************************************/
#define RF_FREQUENCY 868000000 // Hz

#define TX_OUTPUT_POWER 10 // dBm

#define LORA_BANDWIDTH 0				// [0: 125 kHz,
																//  1: 250 kHz,
																//  2: 500 kHz,
																//  3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1				// [1: 4/5,
																//  2: 4/6,
																//  3: 4/7,
																//  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8	// Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0		// Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

typedef enum
{
	LOWPOWER,
	STATE_RX,
	STATE_TX
} States_t;

int16_t txNumber = 0;
int16_t rxNumber = 0;
States_t state;
bool sleepMode = false;
int16_t Rssi, rxSize;

String rssi = "RSSI --";
String packet;
String send_num;

unsigned int counter = 0;
bool receiveflag = false; // software flag for LoRa receiver, received data makes it true.
long lastSendTime = 0;		// last send time
int interval = 1000;			// interval between sends
uint64_t chipid;
int16_t RssiDetection = 0;

void OnTxDone(void)
{
	Serial.print("TX done......");
	state = STATE_RX;
}

void OnTxTimeout(void)
{
	Radio.Sleep();
	Serial.print("TX Timeout......");
	state = STATE_TX;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	rxNumber++;
	Rssi = rssi;
	rxSize = size;
	memcpy(rxpacket, payload, size);
	rxpacket[size] = '\0';
	Radio.Sleep();
	Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n", rxpacket, Rssi, rxSize);
	Serial.println("wait to send next packet");
	receiveflag = true;
	state = STATE_TX;
}

void lora_init(void)
{
	txNumber = 0;
	Rssi = 0;
	rxNumber = 0;
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxDone = OnRxDone;

	Radio.Init(&RadioEvents);
	Radio.SetChannel(RF_FREQUENCY);
	Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
										LORA_SPREADING_FACTOR, LORA_CODINGRATE,
										LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
										true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

	Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
										LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
										LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
										0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
	state = STATE_TX;
	// st7735.st7735_fill_screen(ST7735_BLACK);
	packet = "waiting lora data!";
	// st7735.st7735_write_str(0, 10, packet);
}

/********************************* lora  *********************************************/

void custom_delay(uint32_t time_ms)
{
#if 1
	uint32_t conut = time_ms / 10;
	while (conut > 0)
	{
		conut--;
		delay(10);
		if (interrupt_flag == true)
		{
			delay(200);
			if (digitalRead(0) == 0)
			{
				break;
			}
		}
	}
#else
	delay(time_ms);
#endif
}

// void wifi_connect_init(void)
// {
// 	// Set WiFi to station mode and disconnect from an AP if it was previously connected
// 	WiFi.disconnect(true);
// 	custom_delay(100);
// 	WiFi.mode(WIFI_STA);
// 	WiFi.setAutoConnect(true);
// 	WiFi.begin("Your WiFi SSID", "Your Password"); // fill in "Your WiFi SSID","Your Password"
// 	st7735.st7735_write_str(0, 20, "WIFI Setup done");
// }

// bool wifi_connect_try(uint8_t try_num)
// {
// 	uint8_t count;
// 	while (WiFi.status() != WL_CONNECTED && count < try_num)
// 	{
// 		count++;
// 		st7735.st7735_fill_screen(ST7735_BLACK);
// 		st7735.st7735_write_str(0, 0, "wifi connecting...");
// 		custom_delay(500);
// 	}
// 	if (WiFi.status() == WL_CONNECTED)
// 	{
// 		st7735.st7735_fill_screen(ST7735_BLACK);
// 		st7735.st7735_write_str(0, 0, "wifi connect OK");
// 		custom_delay(2500);
// 		return true;
// 	}
// 	else
// 	{
// 		st7735.st7735_fill_screen(ST7735_BLACK);
// 		st7735.st7735_write_str(0, 0, "wifi connect failed");
// 		custom_delay(1000);
// 		return false;
// 	}
// }

// void wifi_scan(unsigned int value)
// {
// 	unsigned int i;
// 	WiFi.disconnect(); //
// 	WiFi.mode(WIFI_STA);
// 	st7735.st7735_fill_screen(ST7735_BLACK);
// 	for (i = 0; i < value; i++)
// 	{
// 		st7735.st7735_write_str(0, 0, "Scan start...");
// 		int n = WiFi.scanNetworks();
// 		st7735.st7735_write_str(0, 30, "Scan done");

// 		if (n == 0)
// 		{
// 			st7735.st7735_fill_screen(ST7735_BLACK);
// 			st7735.st7735_write_str(0, 0, "no network found");
// 			custom_delay(2000);
// 		}
// 		else
// 		{
// 			st7735.st7735_fill_screen(ST7735_BLACK);
// 			st7735.st7735_write_str(0, 0, (String)n);
// 			st7735.st7735_write_str(30, 0, "networks found:");

// 			custom_delay(2000);

// 			for (int i = 0; (i < n) && (i < 0); ++i)
// 			{
// 				String str = (String)(i + 1) + ":" + (String)(WiFi.SSID(i)) + " (" + (String)(WiFi.RSSI(i)) + ")";
// 				st7735.st7735_write_str(0, 0, str);
// 				custom_delay(100);
// 				st7735.st7735_fill_screen(ST7735_BLACK);
// 			}
// 		}
// 	}
// }
bool test_switch = false;
void interrupt_GPIO0(void)
{
	interrupt_flag = true;
	test_switch = true;
}
void interrupt_handle(void)
{
	if (interrupt_flag)
	{
		interrupt_flag = false;
		Serial.println("interrupt handle");
		Serial.flush();
		if (digitalRead(0) == 0)
		{
			if (rxNumber < 2)
			{
				delay(500);
				if (digitalRead(0) == 0)
				{
					test_status = GPS_TEST;
				}
				else
				{
					resendflag = true;
				}
			}
			else
			{
				test_status = DEEPSLEEP_TEST;
				// deepsleepflag=true;
			}
		}
	}
}

void VextON(void)
{
	pinMode(Vext, OUTPUT);
#ifdef WIFI_TRK_VER_11
	digitalWrite(Vext, HIGH);
#else
	digitalWrite(Vext, LOW);
#endif
}

void VextOFF(void) // Vext default OFF
{
	pinMode(Vext, OUTPUT);
#ifdef WIFI_TRK_VER_11
	digitalWrite(Vext, LOW);
#else
	digitalWrite(Vext, HIGH);
#endif
}

void enter_deepsleep(void)
{
	Radio.Sleep();
	SPI.end();
	pinMode(RADIO_DIO_1, ANALOG);
	pinMode(RADIO_NSS, ANALOG);
	pinMode(RADIO_RESET, ANALOG);
	pinMode(RADIO_BUSY, ANALOG);
	pinMode(LORA_CLK, ANALOG);
	pinMode(LORA_MISO, ANALOG);
	pinMode(LORA_MOSI, ANALOG);
	esp_sleep_enable_timer_wakeup(600 * 1000 * (uint64_t)1000);
	esp_deep_sleep_start();
}

void lora_status_handle(void)
{
	if (resendflag)
	{
		resendflag = false;
		state = STATE_TX;
	}

	if (receiveflag && (state == LOWPOWER))
	{
		receiveflag = false;
		packet = "Rdata:";
		int i = 0;
		while (i < rxSize)
		{
			packet += rxpacket[i];
			i++;
		}
		String packSize = "R_Size:";
		packSize += String(rxSize, DEC);
		packSize += "R_rssi:";
		packSize += String(Rssi, DEC);
		send_num = "send num:";
		send_num += String(txNumber, DEC);
		// st7735.st7735_fill_screen(ST7735_BLACK);
		// delay(100);
		// st7735.st7735_write_str(0, 24, packet);
		// st7735.st7735_write_str(0, 36, packSize);
		// st7735.st7735_write_str(0, 48, send_num);
		if ((rxNumber % 2) == 0)
		{
			digitalWrite(LED, HIGH);
		}
	}
	switch (state)
	{
	case STATE_TX:
		delay(1000);
		txNumber++;
		sprintf(txpacket, "hello %d,Rssi:%d", txNumber, Rssi);
		Serial.printf("\r\nsending packet \"%s\" , length %d\r\n", txpacket, strlen(txpacket));
		Radio.Send((uint8_t *)txpacket, strlen(txpacket));
		state = LOWPOWER;
		break;
	case STATE_RX:
		Serial.println("into RX mode");
		Radio.Rx(0);
		state = LOWPOWER;
		break;
	case LOWPOWER:
		Radio.IrqProcess();
		break;
	default:
		break;
	}
}

void gps_test(void)
{
	pinMode(VGNSS_CTRL, OUTPUT);
	digitalWrite(VGNSS_CTRL, HIGH);
	Serial1.begin(115200, SERIAL_8N1, 33, 34);
	// Serial.println("gps_test");
	// st7735.st7735_fill_screen(ST7735_BLACK);
	// delay(100);
	// st7735.st7735_write_str(0, 0, (String) "gps_test");

	while (1)
	{
		if (Serial1.available() > 0)
		{
			if (Serial1.peek() != '\n')
			{
				gps.encode(Serial1.read());
			}
			else
			{
				Serial1.read();
				if (gps.time.second() == 0)
				{
					continue;
				}

				String time_str = "TIME:" + (String)gps.time.hour() + ":" + (String)gps.time.minute() + ":" + (String)gps.time.second() + ":" + (String)gps.time.centisecond();
				String latitude = "LAT: " + (String)gps.location.lat();
				String longitude = "LON: " + (String)gps.location.lng();
				st7735.st7735_write_str(110, 0, "GPS", Font_7x10, ST7735_MAGENTA, ST7735_BLACK);
				st7735.st7735_write_str(0, 30, time_str, Font_7x10, ST7735_CYAN, ST7735_BLACK);
				st7735.st7735_write_str(0, 42, latitude, Font_7x10, ST7735_CYAN, ST7735_BLACK);
				st7735.st7735_write_str(0, 54, longitude, Font_7x10, ST7735_CYAN, ST7735_BLACK);

				Serial.printf(" %02d:%02d:%02d.%02d", gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond());
				Serial.println();
				Serial.print("LAT: ");
				Serial.print(gps.location.lat(), 6);
				Serial.print(", LON: ");
				Serial.print(gps.location.lng(), 6);
				Serial.println();
				delay(1000);
				while (Serial1.read() > 0)
					;
			}
			if (interrupt_flag)
			{
				interrupt_flag = false;
				break;
			}
		}
	}
}

// uint32_t license[4] = {0x1C05BCF3, 0x37BC9B25, 0x1D9EB3A8, 0x1A3FC335};
uint32_t license[4] = {0xC6E94E6B, 0xE1CFE85E, 0x6A321BC8, 0x942EACA9};

void tracker_v3_test(void);

void ble_test(void)
{
	extern float bodyTemperature;
	extern uint8_t heartRate, SpO2, watchBatteryLevel;
	extern char *ble_mac_address;
	extern char paired_watch_mac_address[];
	extern bool watch_done;
	st7735.st7735_fill_screen(ST7735_BLACK);
	delay(50);
	st7735.st7735_write_str(0, 30, (String) "BLE SCAN INIT", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	st7735.st7735_write_str(0, 0, (String) "Target Device:", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	st7735.st7735_write_str(0, 15, (String)paired_watch_mac_address, Font_7x10, ST7735_WHITE, ST7735_BLACK);

	delay(50);
	ble_process();
	st7735.st7735_fill_screen(ST7735_BLACK);
	delay(50);
	st7735.st7735_write_str(0, 0, (String) "Target Device:", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	st7735.st7735_write_str(0, 15, (String)paired_watch_mac_address, Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if (watch_done)
	{
		watch_done = false;
		String body_temp = "Body Temp: " + (String)bodyTemperature + "deg C";
		st7735.st7735_write_str(0, 30, body_temp, Font_7x10, ST7735_WHITE, ST7735_BLACK);
		String heart_rate = "HR: " + (String)heartRate + "bpm";
		st7735.st7735_write_str(0, 45, heart_rate, Font_7x10, ST7735_WHITE, ST7735_BLACK);

		String spo2 = "; SpO2: " + (String)SpO2 + "%";
		st7735.st7735_write_str(70, 45, spo2, Font_7x10, ST7735_WHITE, ST7735_BLACK);
		String battery_level = "Watch Bat: " + (String)watchBatteryLevel + "%";
		st7735.st7735_write_str(0, 60, battery_level, Font_7x10, ST7735_WHITE, ST7735_BLACK);
	}
	else
	{
		st7735.st7735_write_str(0, 30, (String) "NOT FOUND", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	}
	delay(500);
	st7735.st7735_fill_screen(ST7735_BLACK);
}

void setup()
{
	Serial.begin(115200);
	Mcu.begin();
	st7735.st7735_init();
	st7735.st7735_fill_screen(ST7735_BLACK);

	attachInterrupt(0, interrupt_GPIO0, FALLING);
	resendflag = false;
	deepsleepflag = false;
	interrupt_flag = false;

	chipid = ESP.getEfuseMac();																	 // The chip ID is essentially its MAC address(length: 6 bytes).
	Serial.printf("ESP32ChipID=%04X", (uint16_t)(chipid >> 32)); // print High 2 bytes
	Serial.printf("%08X\n", (uint32_t)chipid);									 // print Low 4bytes.

	test_status = LED_TEST;
}

void loop()
{
	interrupt_handle();
	switch (test_status)
	{
	// case WIFI_CONNECT_TEST_INIT:
	// {
	// 	wifi_connect_init();
	// 	test_status = WIFI_CONNECT_TEST;
	// }
	// case WIFI_CONNECT_TEST:
	// {
	// 	if (wifi_connect_try(1) == true)
	// 	{
	// 		test_status = WIFI_SCAN_TEST;
	// 	}
	// 	wifi_connect_try_num--;
	// 	break;
	// }
	// case WIFI_SCAN_TEST:
	// {
	// 	wifi_scan(1);
	// 	test_status = LORA_TEST_INIT;
	// 	break;
	// }
	case TRACKER_V3_TEST:
	{
		tracker_v3_test();
		break;
	}
	case LORAWAN_INIT:
	{
		lorawan_init();
		test_status = LORAWAN_TEST;
		break;
	}
	case LORAWAN_TEST:
	{
		lorawan_loop();
		test_status = LORAWAN_TEST;
		break;
	}
	case LORA_TEST_INIT:
	{
		lora_init();
		test_status = LORA_COMMUNICATION_TEST;
		break;
	}
	case LORA_COMMUNICATION_TEST:
	{
		lora_status_handle();
		break;
	}
	case DEEPSLEEP_TEST:
	{
		enter_deepsleep();
		break;
	}
	case GPS_TEST:
	{
		gps_test();
		break;
	}
	case LED_TEST:
	{
		// only support for tracker module
		led_test();
		break;
	}
	case BUZZER_TEST:
	{
		buzzer_test();
		break;
	}
	case SD_INIT:
	{
		sd_init();
		extern bool sdInitialized;
		if (sdInitialized)
		{
			test_status = SD_TEST;
		}
		break;
	}
	case SD_TEST:
	{
		sd_test();
		break;
	}
	case MPU_TEST_INIT:
	{
		gsensor_init();
		test_status = MPU_TEST;
		break;
	}
	case MPU_TEST:
	{
		gsensor_test();
		break;
	}
	case DPS_TEST_INIT:
	{
		dsp310_init();
		Serial.flush();

		test_status = DPS_TEST;
		break;
	}
	case DPS_TEST:
	{
		dsp310_fetch();
		break;
	}
	case BAT_INIT:
	{
		battery_init();
		st7735.st7735_fill_screen(ST7735_BLACK);
		delay(100);
		st7735.st7735_write_str(0, 0, (String) "BATTERY INIT");
		test_status = BAT_TEST;
		break;
	}
	case BAT_TEST:
	{
		st7735.st7735_fill_screen(ST7735_BLACK);
		battery_test();
		extern uint32_t batteryVoltage;
		String battery_power = "BAT: " + (String)batteryVoltage;
		st7735.st7735_write_str(0, 40, battery_power);
		delay(1000);
		break;
	}
	case ADC_5V_TEST:
	{
		st7735.st7735_fill_screen(ST7735_BLACK);
		delay(100);
		adc5v_test();
		st7735.st7735_write_str(0, 0, (String) "ADC 5V INIT");
		extern uint32_t adc5Voltage;
		String adc5_power = "V5: " + (String)adc5Voltage;
		st7735.st7735_write_str(0, 40, adc5_power);
		break;
	}
	case BUTTON_TEST:
	{
		button_setting();
		button_test();
		break;
	}
	case BLE_TEST_INIT:
	{
		ble_init();
		Serial.println("BLE Init Done!");
		test_status = BLE_TEST;
		break;
	}
	case BLE_TEST:
	{
		ble_test();
		break;
	}

	case DI_TEST:
	{
		pinMode(DIO1, INPUT);
		pinMode(DIO2, INPUT);
		// pinMode(DIO3, INPUT);

		st7735.st7735_fill_screen(ST7735_BLACK);
		delay(50);
		if (digitalRead(DIO1))
		{
			st7735.st7735_write_str(0, 0, (String) "DIO1 = 1");
		}
		else
		{
			st7735.st7735_write_str(0, 0, (String) "DIO1 = 0");
		}
		if (digitalRead(DIO2))
		{
			st7735.st7735_write_str(0, 25, (String) "DIO2 = 1");
		}
		else
		{
			st7735.st7735_write_str(0, 25, (String) "DIO2 = 0");
		}
		delay(50);
		break;
	}
	default:
		break;
	}
	delay(50);
}

void tracker_v3_test_init(void)
{
	st7735.st7735_fill_screen(ST7735_BLACK);
	pinMode(BUTTON, INPUT_PULLUP);
	pinMode(LED_PIN, OUTPUT);
	pinMode(BUZZER, OUTPUT);
	pinMode(ADC_Ctrl, OUTPUT);
	digitalWrite(ADC_Ctrl, HIGH);
	pinMode(DIO1, INPUT_PULLUP);
	pinMode(PMC, INPUT_PULLUP);
}

void tracker_test_header(void)
{
	st7735.st7735_write_str(0, 0, (String) "Tracker Test", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	adc5v_test();
	extern uint32_t adc5Voltage;
	if (adc5Voltage > 2000)
	{
		st7735.st7735_write_str(0, 12, "USB", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	}
	else
	{
		st7735.st7735_write_str(0, 12, "BATTERY", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	}
	battery_test();
	extern uint32_t batteryVoltage;
	String battery_power = "BAT: " + (String)batteryVoltage;
	st7735.st7735_write_str(80, 12, battery_power, Font_7x10, ST7735_WHITE, ST7735_BLACK);
}

bool gsensor_initialed = false;
bool sd_initialed = false;
bool dsp_initialed = false;
bool lora_initialed = false;
bool ble_initialed = false;
bool emc_button_pressed = false;
bool dio1_button_pressed = false;
bool dio2_button_pressed = false;
void reset_test_status(void)
{
	gsensor_initialed = false;
	sd_initialed = false;
	dsp_initialed = false;
	lora_initialed = false;
	ble_initialed = false;
}

void tracker_v3_test(void)
{
	static int init = 0;
	if (!init)
	{
		tracker_v3_test_init();
		init++;
	}
	emc_button_pressed = !digitalRead(BUTTON);
	if (emc_button_pressed)
	{
		Serial.println("Button Pressed");
		digitalWrite(BUZZER, HIGH);
	}
	else
	{
		Serial.println("Button Released");
		digitalWrite(BUZZER, LOW);
	}

	while (!digitalRead(DIO1))
	{
		st7735.st7735_fill_screen(ST7735_BLACK);
		st7735.st7735_write_str(0, 20, "DIO1_PRESSED", Font_11x18, ST7735_MAGENTA, ST7735_BLACK);
	}
	while (!digitalRead(PMC))
	{
		st7735.st7735_fill_screen(ST7735_BLACK);
		st7735.st7735_write_str(0, 20, "PMC_PRESSED", Font_11x18, ST7735_MAGENTA, ST7735_BLACK);
	}

	static int count = 2;
	if (test_switch)
	{
		test_switch = false;
		count++;
	}
	String string_buffer1;
	String string_buffer2;

	switch (count % 7)
	{
	case 0:
		neopixelWrite(LED_PIN, 255, 0, 0);

		digitalWrite(ST7735_LED_K_Pin, HIGH);
		if (!gsensor_initialed)
		{
			reset_test_status();
			gsensor_initialed = true;
			st7735.st7735_fill_screen(ST7735_BLACK);
			gsensor_init();
		}
		extern int16_t ax, ay, az;
		extern int16_t gx, gy, gz;
		gsensor_test();
		string_buffer1 = "ax:" + (String)ax + " ay:" + (String)ay + " az:" + (String)az;
		string_buffer2 = "gx:" + (String)gx + " gy:" + (String)gy + " gz:" + (String)gz;
		tracker_test_header();
		st7735.st7735_write_str(110, 0, "Gsnr", Font_7x10, ST7735_MAGENTA, ST7735_BLACK);
		st7735.st7735_write_str(0, 30, string_buffer1, Font_7x10, ST7735_WHITE, ST7735_BLACK);
		st7735.st7735_write_str(0, 50, string_buffer2, Font_7x10, ST7735_WHITE, ST7735_BLACK);
		break;
	case 1:
		neopixelWrite(LED_PIN, 0, 255, 0);

		if (!dsp_initialed)
		{
			reset_test_status();
			gsensor_initialed = false;
			dsp_initialed = true;
			st7735.st7735_fill_screen(ST7735_BLACK);
			dsp310_init();
		}
		extern float temperature;
		extern float pressure;
		dsp310_fetch();
		string_buffer1 = "temperature:" + (String)temperature;
		string_buffer2 = "pressure:" + (String)pressure;
		tracker_test_header();
		st7735.st7735_write_str(110, 0, "DSP310", Font_7x10, ST7735_MAGENTA, ST7735_BLACK);
		st7735.st7735_write_str(0, 30, string_buffer1, Font_7x10, ST7735_WHITE, ST7735_BLACK);
		st7735.st7735_write_str(0, 50, string_buffer2, Font_7x10, ST7735_WHITE, ST7735_BLACK);
		break;
	case 2:
		neopixelWrite(LED_PIN, 0, 0, 255);

		if (!lora_initialed)
		{
			reset_test_status();
			lora_initialed = true;
			st7735.st7735_fill_screen(ST7735_BLACK);
			lora_init();
		}
		tracker_test_header();
		st7735.st7735_write_str(110, 0, "Lora", Font_7x10, ST7735_MAGENTA, ST7735_BLACK);
		st7735.st7735_write_str(0, 24, "TX:", Font_7x10, ST7735_GREEN, ST7735_BLACK);
		st7735.st7735_write_str(0, 36, txpacket, Font_7x10, ST7735_WHITE, ST7735_BLACK);
		st7735.st7735_write_str(0, 48, "RX:", Font_7x10, ST7735_GREEN, ST7735_BLACK);
		st7735.st7735_write_str(0, 60, rxpacket, Font_7x10, ST7735_WHITE, ST7735_BLACK);
		lora_status_handle();
		break;
	case 3:
		neopixelWrite(LED_PIN, 255, 255, 255);
		st7735.st7735_fill_screen(ST7735_BLACK);
		tracker_test_header();
		gps_test();
		break;
	case 4:
		neopixelWrite(LED_PIN, 255, 0, 255);
		if (!ble_initialed)
		{
			reset_test_status();
			ble_initialed = true;
			st7735.st7735_fill_screen(ST7735_BLACK);
			ble_init();
			st7735.st7735_write_str(50, 20, "BLE", Font_16x26, ST7735_MAGENTA, ST7735_BLACK);
			delay(1000);
		}
		ble_test();
		break;
	case 5:
		neopixelWrite(LED_PIN, 128, 128, 128);

		if (!sd_initialed)
		{
			reset_test_status();
			sd_initialed = true;
			st7735.st7735_fill_screen(ST7735_BLACK);
			tracker_test_header();
			sd_init();
			extern bool sdInitialized;
			if (sdInitialized)
			{
				st7735.st7735_write_str(0, 24, "SD Init Success", Font_7x10, ST7735_GREEN, ST7735_BLACK);
			}
			else
			{
				break;
			}
		}
		tracker_test_header();
		extern bool sd_write_success;
		st7735.st7735_write_str(110, 0, "SD", Font_7x10, ST7735_MAGENTA, ST7735_BLACK);
		if (sd_write_success)
		{
			st7735.st7735_write_str(0, 36, "Hello, SD card ^_^", Font_7x10, ST7735_GREEN, ST7735_BLACK);
			st7735.st7735_write_str(0, 48, "Successfully wrote to file", Font_7x10, ST7735_GREEN, ST7735_BLACK);
		}
		sd_test();
		break;
	case 6:
		neopixelWrite(LED_PIN, 0, 0, 0);

		st7735.st7735_fill_screen(ST7735_BLACK);
		tracker_test_header();
		digitalWrite(ST7735_LED_K_Pin, HIGH);
		st7735.st7735_write_str(0, 30, "Screen backlight on", Font_7x10, ST7735_MAGENTA, ST7735_BLACK);
		delay(500);
		digitalWrite(ST7735_LED_K_Pin, LOW);
		st7735.st7735_write_str(0, 30, "Screen backlight off - you never see me", Font_7x10, ST7735_MAGENTA, ST7735_BLACK);
		delay(500);
		break;
	}
	delay(10);
}
