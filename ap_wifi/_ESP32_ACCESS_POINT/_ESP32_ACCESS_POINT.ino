#include <WiFi.h>                                     // needed to connect to setup an accesspoint

// SSID and password that are going to be used for the Access Point you will create -> DONT use the SSID/Password of your router:
const char* ssid = "SOLWER_MONITOR";
const char* password = "smartagri_iot";
int period = 1000;
unsigned long time_now = 0;

// Configure IP addresses of the local access point
IPAddress local_IP(192,168,1,22);
IPAddress gateway(192,168,1,5);
IPAddress subnet(255,255,255,0);

void setup() {
  Serial.begin(115200);                               // init serial port for debugging
 
  Serial.print("Setting up Access Point ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Starting Access Point ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  Serial.print("IP address => ");
  Serial.println(WiFi.softAPIP());
  
}

void loop() {
	if(millis() >= time_now + period){
		time_now += period;
		  Serial.print("IP address => ");
      Serial.println(WiFi.softAPIP());
      Serial.print("SSID => ");
      Serial.println(ssid);
	}
}