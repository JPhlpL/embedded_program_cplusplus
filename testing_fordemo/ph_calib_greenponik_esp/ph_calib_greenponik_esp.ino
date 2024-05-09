
#include "DFRobot_ESP_PH.h"
#include <EEPROM.h>

DFRobot_ESP_PH ph;
// #define ESPADC 4096.0   //the esp Analog Digital Convertion value
// #define ESPVOLTAGE 3300 //the esp voltage supply value
#define ESPADC 4096.0
#define ESPVOLTAGE 3300.0
#define PH_PIN 32		//the esp gpio data pin number
float voltage, phValue, amb_temperature = 25;

void setup()
{
	Serial.begin(115200);
	EEPROM.begin(32);//needed to permit storage of calibration value in eeprom
	ph.begin();
}
  
void loop()
{
	static unsigned long timepoint = millis();
	if (millis() - timepoint > 1000U) //time interval: 1s
	{
		timepoint = millis();
		//voltage = rawPinValue / esp32ADC * esp32Vin
		voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
		Serial.print("voltage:");
		Serial.println(voltage, 4);
		
		//temperature = readTemperature();  // read your temperature sensor to execute temperature compensation
		Serial.print("temperature:");
		Serial.print(amb_temperature, 1);
		Serial.println("^C");

		phValue = ph.readPH(voltage, amb_temperature); // convert voltage to pH with temperature compensation
		Serial.print("pH:");
		Serial.println(phValue, 4);
	}
	ph.calibration(voltage, amb_temperature); // calibration process by Serail CMD
}

float readTemperature()

{
	//add your code here to get the temperature from your temperature sensor
}
