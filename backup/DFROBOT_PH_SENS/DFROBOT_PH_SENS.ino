/*!
 * @file DFRobot_PH_Test.h
 * @brief This is the sample code for Gravity: Analog pH Sensor / Meter Kit V2, SKU:SEN0161-V2.
 * @n In order to guarantee precision, a temperature sensor such as DS18B20 is needed, to execute automatic temperature compensation.
 * @n You can send commands in the serial monitor to execute the calibration.
 * @n Serial Commands:
 * @n    enterph -> enter the calibration mode
 * @n    calph   -> calibrate with the standard buffer solution, two buffer solutions(4.0 and 7.0) will be automaticlly recognized
 * @n    exitph  -> save the calibrated parameters and exit from calibration mode
 *
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Jiawei Zhang](jiawei.zhang@dfrobot.com)
 * @version  V1.0
 * @date  2018-11-06
 * @url https://github.com/DFRobot/DFRobot_PH
 */

#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
DFRobot_PH ph;
#define PH_PIN 33
#define ESPADC 4095.0   //the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300 //the esp voltage supply value

LiquidCrystal_I2C lcd(0x27, 20, 4);
float voltage, phValue, temperature=25, acidVoltage = 2023, neutralVoltage = 1500;


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

void setup()
{
    lcd.init();
    lcd.backlight();
    Serial.begin(115200);  
    EEPROM.begin(32);//needed to permit storage of calibration value in eeprom
    ph.begin();
}

void loop()
{
    static unsigned long timepoint = millis();
    if(millis()-timepoint>1000U){                  //time interval: 1s
        timepoint = millis();
        //temperature = readTemperature();         // read your temperature sensor to execute temperature compensation


        voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage

        //float slope = (7.0 - 4.0) / ((neutralVoltage - 1500) / 3.0 - (acidVoltage - 1500) / 3.0);
        //float intercept = 7.0 - slope * (neutralVoltage - 1500) / 3.0;
        //float phValueCurrent = slope * (voltage - 1500) / 3.0 + intercept;

        phValue = ph.readPH(voltage, temperature);  // convert voltage to pH with temperature compensation

        Serial.print("temperature:");
        Serial.print(temperature,1);
        Serial.print("^C  pH:");
        Serial.println(phValue,4);
        Serial.print("^C  pH Current:");

        String phValueCurrentStr = String(phValue, 4);
        printToLCD(phValue, 4);
        //Serial.println(phValue,2);
    }

    ph.calibration(voltage,temperature);           // calibration process by Serail CMD
}

float readTemperature()
{
  //add your code here to get the temperature from your temperature sensor
}