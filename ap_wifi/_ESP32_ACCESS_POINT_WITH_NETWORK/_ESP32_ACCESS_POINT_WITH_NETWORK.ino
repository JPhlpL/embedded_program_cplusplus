#include <WiFi.h>
 
const char* wifi_network_ssid = "PH1";
const char* wifi_network_password =  "123123yes";
 
const char *soft_ap_ssid = "SOLWER_MONITOR";
const char *soft_ap_password = "smartagri_iot";
 
void setup() {
 
  Serial.begin(115200);
   
  WiFi.mode(WIFI_MODE_APSTA);
 
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  WiFi.begin(wifi_network_ssid, wifi_network_password);
 
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());
 
  Serial.print("ESP32 IP on the WiFi network: ");
  Serial.println(WiFi.localIP());
     
}
 
void loop() {}