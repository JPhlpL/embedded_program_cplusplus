#include <Wire.h>
#define PH_PIN 32
// #define offSetVal 
float voltage,phValue,temperature = 25;

float acidVoltage = 2023;
float neutralVoltage = 1500;

void setup(){
  Serial.begin(115200);
  delay(1000);
}

void loop(){
  static unsigned long timepoint = millis();
  if(millis()-timepoint>1000U){
    timepoint = millis();

    voltage = analogRead(PH_PIN)/4095.0*3300;

    float slope = (7.0-4.0)/((neutralVoltage-1500)/3.0 -(acidVoltage-1500)/3.0);
    float intercept = 7.0 - slope * (neutralVoltage - 1500) / 3.0;

    phValue = slope * (voltage - 1500)/3.0+intercept;

    Serial.print("Voltage: ");
    Serial.print(voltage, 1);
    Serial.print(" pH:");
    Serial.println(phValue,2);

    
  }
}