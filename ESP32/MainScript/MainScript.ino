/*
 * This is the main script for the weather datalogging Project
 * Projekt: uWeather
 */

// Libraries
#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

// Replace with your network login data
RTC_DATA_ATTR const char* wifi_ssid = "REPLACE_WITH_WIFI_SSID";
RTC_DATA_ATTR const char* wifi_password = "REPLACE_WITH_WIFI_PASSWORD";

// Replace with your MySQL server login data
RTC_DATA_ATTR IPAddress db_server_addr(0, 0, 0, 0);
RTC_DATA_ATTR int db_port = 3306; 
RTC_DATA_ATTR char db_user[] = "REPLACE_WITH_DB_USER"; 
RTC_DATA_ATTR char db_password[] = "REPLACE_WITH_DB_PASSWORD"; 

RTC_DATA_ATTR WiFiClient client;
RTC_DATA_ATTR MySQL_Connection conn(&client);
RTC_DATA_ATTR MySQL_Cursor* cursor;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  get_network_info();

  // Establish a MySQL connection
  Serial.print("Connecting to MySQL Server...");
  if (conn.connect(db_server_addr, db_port, db_user, db_password)) {
    Serial.println("Connected to MySQL server");
  }
  else {
    Serial.println("MySQL connection failed");
    while (1);
  }



  // Disconnect from MySQL server
  conn.close();
  Serial.println("Disconnected from MySQL server");

  // Disconnect from WiFi
  WiFi.disconnect();
  Serial.println("Disconnected from WiFi");
}

void loop() {
  // empty
}

// Gets all the network info
void get_network_info(){
    if(WiFi.status() == WL_CONNECTED) {
        Serial.print("[*] Network information for ");
        Serial.println(wifi_ssid);

        Serial.println("[+] BSSID : " + WiFi.BSSIDstr());
        Serial.print("[+] Gateway IP : ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("[+] Subnet Mask : ");
        Serial.println(WiFi.subnetMask());
        Serial.println((String)"[+] RSSI : " + WiFi.RSSI() + " dB");
        Serial.print("[+] ESP32 IP : ");
        Serial.println(WiFi.localIP());
    }
}
