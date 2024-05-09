#include <SoftwareSerial.h>                           //we have to include the SoftwareSerial library, or else we can't use it
#include <WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define EC_rx 16                                          //define what pin rx is going to be
#define EC_tx 17                                          //define what pin tx is going to be
#define PH_PIN 33
#define TEMP_PIN 4

const char* ssid = "SOLWER_MONITOR";
const char* password = "smartagri_iot";
const char* host = "192.168.1.25";

WiFiServer server(80);
SoftwareSerial myserial(EC_rx, EC_tx);                      //define how the soft serial port is going to work
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

String inputstring = "";                              //a string to hold incoming data from the PC
String sensorstring = "";                             //a string to hold the data from the Atlas Scientific product
boolean input_string_complete = false;                //have we received all the data from the PC
boolean sensor_string_complete = false;               //have we received all the data from the Atlas Scientific product

float voltage, phValue, temperature, acidVoltage = 2023, neutralVoltage = 1500;

void setup() {             
  Serial.print("Initializing");                        //set up the hardware
  Serial.begin(9600);                                 //set baud rate for the hardware serial port_0 to 9600
  myserial.begin(9600);                               //set baud rate for the software serial port to 9600
  sensors.begin();
  //WiFi Initializing
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    server.begin();
    Serial.println("Server started");
    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("\n");
  //WiFi Initializing
  Serial.println("Welcome to Monitoring System!\n");
  inputstring.reserve(10);                            //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);                           //set aside some bytes for receiving data from Atlas Scientific product
}


void serialEvent() {                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
}

void loop() {                                         //here we go...z

  if (input_string_complete == true) {                //if a string from the PC has been received in its entirety
    myserial.print(inputstring);                      //send that string to the Atlas Scientific product
    myserial.print('\r');                             //add a <CR> to the end of the string
    inputstring = "";                                 //clear the string
    input_string_complete = false;                    //reset the flag used to tell if we have received a completed string from the PC
  }

  if (myserial.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character
    char inchar = (char)myserial.read();              //get the char we just received
    sensorstring += inchar;                           //add the char to the var called sensorstring
    if (inchar == '\r') {                             //if the incoming character is a <CR>
      sensor_string_complete = true;                  //set the flag
    }
  }

  if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
    if (isdigit(sensorstring[0]) == false) {          //if the first character in the string is a digit
      Serial.println(sensorstring);                   //send that string to the PC's serial monitor
    }
    else                                              //if the first character in the string is NOT a digit
    {
      printData();                                //then call this function 
    }
    sensorstring = "";                                //clear the string
    sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
  }
}

//Sending to DB
void sendData(float temp, float pH, float EC) {
  Serial.print("Connecting to: ");
  Serial.print(host);
  Serial.println();
  WiFiClient client;
  const int httpPort = 80;
  if(!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
   // This will send the request to the server
  client.print(String("GET http://192.168.1.25/_solwerSmartAgriIoT/scripts/datalog_monitordose.php?") + 
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
    if (millis() - timeout > 30000) //30 seconds
    {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
    }
  }
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
  }
  delay(10000); // 10 Seconds
  Serial.println();
  Serial.println("closing connection");
}
//Sending to DB


void printData(void) {                            //this function will pars the string  
  static unsigned long timepoint = millis();
    if(millis()-timepoint > 180000) { //time interval: 3 
    timepoint = millis();
    
    char sensorstring_array[30];                        //we make a char array
    char *EC;                                           //char pointer used in string parsing
    float FLOAT_EC;

    //Temperature
    sensors.requestTemperatures(); 
    temperature = sensors.getTempCByIndex(0);
    
    //EC
    sensorstring.toCharArray(sensorstring_array, 30);   //convert the string to a char array 
    EC = strtok(sensorstring_array, ",");               //let's pars the array at each comma               
    FLOAT_EC= atof(EC);                                 //uncomment this line to convert the char to a float

    //pH
    voltage = analogRead(PH_PIN) / 4095.0 * 3300;
    float slope = (7.0 - 4.0) / ((neutralVoltage - 1500) / 3.0 - (acidVoltage - 1500) / 3.0);
    float intercept = 7.0 - slope * (neutralVoltage - 1500) / 3.0;
    phValue = slope * (voltage - 1500) / 3.0 + intercept;
    
    //
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" Celc ");
    Serial.print(" pH: ");
    Serial.print(phValue);
    Serial.print(" ");
    Serial.print(" EC:");         
    Serial.print(FLOAT_EC);  
    Serial.print(" mS/cm ");
    Serial.print(" Timestamp: ");
    Serial.print(millis() / 1000);
    Serial.print("s");
    Serial.println();
    
    //send the values to the database
    Serial.println("Sending to database...");
    delay(5000); //5 seconds

    sendData(temperature, phValue, FLOAT_EC);
    delay(30000); //30 seconds
  }
}














