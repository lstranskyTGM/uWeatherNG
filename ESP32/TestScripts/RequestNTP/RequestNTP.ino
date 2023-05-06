/*
 * This is a test file for requesting an NTP (Network time protocol) Client and writing that time in the ESP32
 * integrated RTC (Real time clock).
 */

#include <WiFi.h>
#include "time.h"
#include <ESP32Time.h>

const char* SSID = "REPLACE_WITH_WIFI_SSID";
const char* PASS = "REPLACE_WITH_WIFI_PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

ESP32Time rtc(7200);

WiFiClient client;

void setup() {
  Serial.begin(115200);

  connectToWifi();

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
      rtc.setTimeStruct(timeinfo); 
  }

}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }

  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));

  delay(1000);
}

void connectToWifi() {
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}


