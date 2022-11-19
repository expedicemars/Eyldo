#include <Arduino.h>

// DHT
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 7
#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

// OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// CO2
#include "MHZ19.h"
#include <SoftwareSerial.h>
#define RX_PIN 5
#define TX_PIN 6

SoftwareSerial Serial1(TX_PIN, RX_PIN);
MHZ19 MHZ(&Serial1);

void setup() {
	Serial.begin(9600);
	// Initialize device.
	dht.begin();
	sensor_t sensor;
	dht.temperature().getSensor(&sensor);
	dht.humidity().getSensor(&sensor);
	// Set delay between sensor readings based on sensor details.
	delayMS = sensor.min_delay / 1000;

	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
		Serial.println(F("SSD1306 allocation failed"));
	}

	// Clear the buffer
	display.clearDisplay();

	Serial1.begin(9600);
}

float humidity = 0.0;
float temperature = 0.0;
float CO2 = 0.0;

void loop() {
	// ------------ DHT ------------
	// Delay between measurements.
	delay(delayMS);
	// Get temperature event and print its value.
	sensors_event_t event;
	dht.temperature().getEvent(&event);

	if (isnan(event.temperature)) {
		Serial.println(F("Error reading temperature!"));
	}
	else {
		Serial.print(F("Temperature: "));
		Serial.print(event.temperature);
		Serial.println(F("°C"));
		temperature = event.temperature;
	}

	// Get humidity event and print its value.
	dht.humidity().getEvent(&event);
	if (isnan(event.relative_humidity)) {
		Serial.println(F("Error reading humidity!"));
	}
	else {
		Serial.print(F("Humidity: "));
		Serial.print(event.relative_humidity);
		Serial.println(F("%"));
		humidity = event.relative_humidity;
	}

	// ------------ OLED ------------
	display.clearDisplay();

	display.setTextSize(1);      // Normal 1:1 pixel scale
	display.setTextColor(SSD1306_WHITE); // Draw white text
	display.setCursor(0, 0);     // Start at top-left corner
	display.cp437(true);         // Use full 256 char 'Code Page 437' font

	String temp = "Teplota: ";
	String temp_num = String(temperature + 273.15);
	String temp_unit = " K";
	temp.concat(temp_num);
	temp.concat(temp_unit);
	display.write(temp.c_str(), temp.length());
	
	display.write('\n');

	String hum = "Vlhkost: ";
	String hum_num = String(humidity);
	String hum_unit = " %";
	hum.concat(hum_num);
	hum.concat(hum_unit);
	display.write(hum.c_str(), hum.length());

	display.write('\n');

	MHZ19_RESULT response = MHZ.retrieveData();
	if (response == MHZ19_RESULT_OK)
	{
		CO2 = MHZ.getCO2();
	}
	else
	{
		Serial.print(F("Error, code: "));
		Serial.println(response);
	}

	Serial.println(CO2);

	String CO2_text = "CO2: ";
	String CO2_num = String((int)CO2);
	String CO2_unit = " ppm";
	CO2_text.concat(CO2_num);
	CO2_text.concat(CO2_unit);
	display.write(CO2_text.c_str(), CO2_text.length());

	display.display();
}