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
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;  // Does the daylightOffset change by itself?

ESP32Time rtc(0);

WiFiClient client;

int period = 1000;
unsigned long time_now = 0;

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */ 
#define TIME_TO_SLEEP 60/* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

void setup() {
  Serial.begin(115200);

  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  connectToWifi();

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
      rtc.setTimeStruct(timeinfo); 
  }

  // time_now = millis();

  /*
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }
  */

  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  esp_deep_sleep_start();

}

void loop() {
  /*
  // time_now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }

  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));

  esp_deep_sleep_start();

  while(millis() < time_now + period) {
    // Wait for 1000 ms
  }
  // Non-blocking
  while(millis() - time_now < period) {
    // Wait for 100 ms
  }
  */
}

void connectToWifi() {
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}


