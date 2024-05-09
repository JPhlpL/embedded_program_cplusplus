#define ESPADC 4096.0//the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300//the esp voltage supply value
#define PH_PIN 15//the esp gpio data pin number
#define EC_rx 16
#define EC_tx 17
#define TEMP_PIN 33

#include "DFRobot_ESP_PH.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>


DFRobot_ESP_PH ph;


const char* ssid = "SOLWER_MONITOR";
const char* password = "smartagri_iot";
const char* host = "192.168.1.25";

WiFiServer server(80);
SoftwareSerial myserial(EC_rx, EC_tx);
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

String inputstring = "";
String sensorstring = "";
boolean input_string_complete = false;
boolean sensor_string_complete = false;

LiquidCrystal_I2C lcd(0x27, 20, 4);

float voltage, phValue, amb_temperature = 25, water_temp;
float FLOAT_EC = 0.00; // Declare FLOAT_EC globally
float acidVoltage = 2023;
float neutralVoltage = 1500;

unsigned long lastPrintTime = 0;
unsigned long lastSendTime = 0;
const unsigned long printInterval = 10000;  // Print data every 1 minute
const unsigned long sendInterval = 150000; // Send data every 2.5 minutes

//print to lcd
void printToLCD(String message, int row) {
  lcd.setCursor(0, row);
  int len = message.length();
  if (len <= 20) {
    lcd.print(message);
  } else {
    for (int i = 0; i < len; i += 20) {
      lcd.print(message.substring(i, i + 20));
      if (i + 20 < len) {
        lcd.setCursor(0, row + 1);
      }
    }
  }
}

//for loading dots transition lcd
void printLoadingDots(int numDots, int delayTime) {
  int dotCount = 0;
  unsigned long startTime = millis();
  
  while (millis() - startTime < delayTime) {
    lcd.print(".");
    delay(250); // Adjust the delay time between dots
    lcd.clear();
    delay(250); // Adjust the delay time between dots
  }
}


void setup()
{
   lcd.init();
  lcd.backlight();
  
  // Second row
  printToLCD("Initializing...", 1);

  Serial.begin(9600);
  myserial.begin(9600);
  sensors.begin();
  EEPROM.begin(32);//needed to permit storage of calibration value in eeprom


  // Second row
  lcd.clear();

  WiFi.begin(ssid, password);
  int loadingProgress = 0; // Variable to track loading progress

  while (WiFi.status() != WL_CONNECTED) {
    printToLCD("Connecting to WiFi", 1);

    // Print loading dots
    for (int i = 0; i < loadingProgress; ++i) {
      lcd.print(".");
    }

    // Update loading progress
    loadingProgress = (loadingProgress + 1) % 4;

    delay(500); // Adjust the delay time as needed
    lcd.clear();
  }

  lcd.clear();
  printToLCD("Server started", 1);
  printToLCD(WiFi.localIP().toString(), 2);

  server.begin();
  Serial.println("Server started");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("\n");

  delay(2000); // Wait for 2 seconds

  lcd.clear();
  printToLCD("Welcome to Monitoring", 1);
  printToLCD("Module!", 2);
  lcd.clear();

  Serial.println("Welcome to Monitoring Module!\n");
  inputstring.reserve(10);
  sensorstring.reserve(30);

  unsigned long loadingStartTime = millis();
  while (millis() - loadingStartTime < 60000) { // 1 minute
    // Print loading dots
    printToLCD("Fetching Data", 0);
    for (int i = 0; i < loadingProgress; ++i) {
      lcd.print(".");
    }

    // Update loading progress
    loadingProgress = (loadingProgress + 1) % 4;
    
    delay(500); // Adjust the delay time as needed
    lcd.clear();
  }
  //lcd.clear();
}




void loop()
{
	static unsigned long timepoint = millis();
	if (millis() - timepoint > 1000U) //time interval: 1s
	{
		timepoint = millis();
		//voltage = rawPinValue / esp32ADC * esp32Vin
		voltage = analogRead(PH_PIN)/4095.0*3300;
    float slope = (7.0-4.0)/((neutralVoltage-1500)/3.0 -(acidVoltage-1500)/3.0);
    float intercept = 7.0 - slope * (neutralVoltage - 1500) / 3.0;

    phValue = slope * (voltage - 1500)/3.0+intercept;

    Serial.print("Voltage: ");
    Serial.print(voltage, 1);
    Serial.print(" pH:");
    Serial.println(phValue,2);

	}
	// ph.calibration(voltage, amb_temperature); // calibration process by Serail CMD
}

float readTemperature()
{
	//add your code here to get the temperature from your temperature sensor
}
