#include <SoftwareSerial.h>
#include <WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include "DFRobot_ESP_PH.h"
#include "EEPROM.h"

DFRobot_ESP_PH ph;
#define ESPADC 4095.0   //the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300 //the esp voltage supply value
#define EC_rx 16
#define EC_tx 17
#define PH_PIN 33
#define TEMP_PIN 4

const char* ssid = "SOLWER_MONITOR_DATA";
const char* password = "smartagri_iot";
const char* host = "192.168.1.100"; //101

WiFiServer server(80);
SoftwareSerial myserial(EC_rx, EC_tx);
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

String inputstring = "";
String sensorstring = "";
boolean input_string_complete = false;
boolean sensor_string_complete = false;

LiquidCrystal_I2C lcd(0x27, 20, 4);

float voltage, phValue, amb_temperature=25, temperature;
float FLOAT_EC; // Declare FLOAT_EC globally

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

//setup
void setup() {
  lcd.init();
  lcd.backlight();
  
  // Second row
  printToLCD("Initializing...", 1);

  Serial.begin(9600);
  myserial.begin(9600);
  sensors.begin();
  EEPROM.begin(32);//needed to permit storage of calibration value in eeprom
	ph.begin();

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



void serialEvent() {
  inputstring = Serial.readStringUntil(13);
  input_string_complete = true;
}

void loop() {
  printToLCD("Fetching Data...", 0);

  if (input_string_complete == true) {
    myserial.print(inputstring);
    myserial.print('\r');
    inputstring = "";
    input_string_complete = false;
  }

  if (myserial.available() > 0) {
    char inchar = (char)myserial.read();
    sensorstring += inchar;
    if (inchar == '\r') {
      sensor_string_complete = true;
    }
  }

  if (sensor_string_complete == true) {
    if (isdigit(sensorstring[0]) == false) {
      Serial.println(sensorstring);
    } else {

      // Print sensor data every second
      if (millis() - lastPrintTime >= printInterval) {
        lastPrintTime = millis();
        printData();
      }

      // Send data every 2 minutes
      if (millis() - lastSendTime >= sendInterval) {
        lastSendTime = millis();
        sendData(temperature, phValue, FLOAT_EC);
      }
    }
    sensorstring = "";
    sensor_string_complete = false;
  }
  ph.calibration(voltage, amb_temperature); // calibration process by Serail CMD
}

void sendData(float temp, float pH, float EC) {
  //lcd.clear();
  Serial.print("Sending...");
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    printToLCD("Err! HTTP.503", 0);
    return;
  }

  client.print(String("GET http://192.168.1.101/solwer/scripts/datalog.php?") +
               ("&temp=") + String(temp) +
               ("&pH=") + String(pH) +
               ("&EC=") + String(EC) +
               ("&GHName=GreenHouse2") +
               " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  client.println();
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      Serial.println(">>> The sensor data has been sent to the db <<<");
      printToLCD("Data Sent!", 0);
      client.stop();
      return;
    }
  }
}


void printData(void) {
  char sensorstring_array[30];
  

  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);

  sensorstring.toCharArray(sensorstring_array, 30);
  char *EC_str = strtok(sensorstring_array, ",");
  FLOAT_EC = atof(EC_str);


  voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
  phValue = ph.readPH(voltage, amb_temperature); // convert voltage to pH with temperature compensation

  //lcd.clear();
  printToLCD("Current:", 0);
  printToLCD("Temp: " + String(temperature) + " C", 1);
  printToLCD("pH: " + String(phValue), 2);
  printToLCD("EC: " + String(FLOAT_EC/1000) + " mS/cm", 3);

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" Celc ");
  Serial.print(" pH: ");
  Serial.print(phValue);
  Serial.print(" ");
  Serial.print(" EC:");
  Serial.print(FLOAT_EC/1000);
  Serial.print(" mS/cm ");
  Serial.print(" Timestamp: ");
  Serial.print(millis() / 1000);
  Serial.print("s");
  Serial.println();
}


