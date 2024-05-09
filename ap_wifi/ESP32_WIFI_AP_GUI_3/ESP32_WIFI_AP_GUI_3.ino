#include <WiFi.h>
#include "esp_wifi.h"
#include <WiFiClient.h>
#include <WebServer.h>

WebServer server(80);

const char* ssid = "SOLWER_MONITOR"; // Change to your desired SSID
const char* password = "smartagri_iot"; // Change to your desired password
int period = 1000; // Update period for listing connected clients

char ip[IP4ADDR_STRLEN_MAX];

IPAddress local_IP(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  Serial.begin(115200);

  Serial.print("Setting up Access Point ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Starting Access Point ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  Serial.print("IP address => ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String page = "<!DOCTYPE html><html><head><style>table {font-family: Arial, sans-serif; border-collapse: collapse; width: 100%;} th, td {border: 1px solid #ddd; padding: 8px;} th {background-color: #f2f2f2;} td {text-align: left;}</style></head><body>";
  page += "<h2>Connected Devices</h2><table><tr><th>MAC Address</th><th>IP Address</th></tr>";

  wifi_sta_list_t wifi_sta_list;
  tcpip_adapter_sta_list_t adapter_sta_list;

  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
  memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

  esp_wifi_ap_get_sta_list(&wifi_sta_list);
  tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

  for (int i = 0; i < adapter_sta_list.num; i++) {
    
    tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             station.mac[0], station.mac[1], station.mac[2],
             station.mac[3], station.mac[4], station.mac[5]);

    char ipStr[16];
    IPAddress ipAddr(station.ip.addr);
    snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);

    //Printing
    Serial.print("station nr ");
    Serial.println(i);
    Serial.print("MAC: ");

    for(int i = 0; i< 6; i++){
      Serial.printf("%02X", station.mac[i]);  
      if(i<5)Serial.print(":");
    }
    Serial.print("\n ***IP: ");  
    esp_ip4addr_ntoa(&station.ip, ip, IP4ADDR_STRLEN_MAX);
    Serial.print(ip);
    Serial.print("\n"); 
    //Printing

    page += "<tr><td>";
    page += macStr;
    page += "</td><td>";
    page += ipStr;
    page += "</td></tr>";
  }

  page += "</table></body></html>";

  server.send(200, "text/html", page);

  Serial.println("-----------");
}

Can you modify this code? where it gives the mac address fa:f2:32:98:as:db a static ip address of 192.168.1.24 and that ip address cant be accessed by other devices