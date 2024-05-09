#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"

const char* ssid = "SOLWER_MONITOR"; // Change to your desired SSID
const char* password = "smartagri_iot"; // Change to your desired password
int period = 1000; // Update period for listing connected clients

IPAddress local_IP(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80); // Create a WebServer instance listening on port 80
String connectedClients; // Global variable to store connected clients

void setup() {
  Serial.begin(115200);
  

  Serial.print("Setting up Access Point ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Starting Access Point ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  Serial.print("IP address => ");
  Serial.println(WiFi.softAPIP());
  
  // Route for serving the HTML page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getIndexHtml());
  });

  // Route for getting the list of connected clients
  server.on("/clients", HTTP_GET, []() {
    server.send(200, "application/json", connectedClients);
  });

  server.begin();
}

void loop() {
  server.handleClient(); // Handle incoming client requests

  // Update connected clients every specified period
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= period) {
    lastUpdate = millis();
    updateConnectedClients();
  }
}

String getIndexHtml() {
  String html = "<!DOCTYPE html>\n";
  html += "<html lang=\"en\">\n";
  html += "<head>\n";
  html += "  <meta charset=\"UTF-8\">\n";
  html += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  html += "  <title>Connected Clients</title>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "  <h1>Connected Clients</h1>\n";
  html += "  <ul id=\"client-list\"></ul>\n";
  html += "  <script>\n";
  html += "    function fetchClients() {\n";
  html += "      fetch('/clients')\n";
  html += "        .then(response => response.json())\n";
  html += "        .then(clients => {\n";
  html += "          const clientList = document.getElementById('client-list');\n";
  html += "          clientList.innerHTML = '';\n";
  html += "          clients.forEach(client => {\n";
  html += "            const listItem = document.createElement('li');\n";
  html += "            listItem.textContent = client;\n";
  html += "            clientList.appendChild(listItem);\n";
  html += "          });\n";
  html += "        })\n";
  html += "        .catch(error => console.error('Error fetching clients:', error));\n";
  html += "    }\n";
  html += "    fetchClients();\n";
  html += "    setInterval(fetchClients, 5000);\n"; // Update client list every 5 seconds
  html += "  </script>\n";
  html += "</body>\n";
  html += "</html>\n";
  return html;
}

void updateConnectedClients() {
  connectedClients = "[";
  wifi_sta_list_t wifi_sta_list;
  esp_wifi_ap_get_sta_list(&wifi_sta_list);
  for (int i = 0; i < wifi_sta_list.num; i++) {
    if (i > 0) connectedClients += ",";
    connectedClients += "\"" + WiFi.BSSIDstr(i) + "\""; // Update to correct format for BSSID
  }
  connectedClients += "]";
}

