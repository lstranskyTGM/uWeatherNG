/*
 * This is the main script for the weather datalogging Project
 * Projekt: uWeather
 */

// Libraries
#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "time.h"
#include <ESP32Time.h>

// Replace with your network login data
 const char* wifi_ssid = "REPLACE_WITH_WIFI_SSID";
 const char* wifi_password = "REPLACE_WITH_WIFI_PASSWORD";

// Replace with your MySQL server login data
IPAddress db_server_addr(0, 0, 0, 0);
int db_port = 3306; 
char db_user[] = "REPLACE_WITH_DB_USER"; 
char db_password[] = "REPLACE_WITH_DB_PASSWORD"; 

WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;  // Does the daylightOffset change by itself?

ESP32Time rtc(0);

int period = 1000;
unsigned long time_now = 0;

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */ 
#define TIME_TO_SLEEP 300/* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR boolean requestedNTP;

void setup() {
  Serial.begin(115200);

  ++bootCount;
  Serial.println(bootCount);

  // Connect to Wi-Fi network
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  get_network_info();

  if (requestedNTP == false) {
    request_NTP_set_RTC();
    requestedNTP = true;
  }

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

// Connects to an NTP and sets the time of the integrated RTC Module
void request_NTP_set_RTC() {
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Set rtc time using NTP
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
      rtc.setTimeStruct(timeinfo); 
  }
}

// Starts DeepSleep nad sets wakeup event
void start_DeepSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  esp_deep_sleep_start();
}


