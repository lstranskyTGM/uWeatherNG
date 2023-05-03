/*
 * This is a test file for requesting an NTP (Network time protocol) Client and writing that time in the ESP32
 * integrated RTC (Real time clock).
 */

#include <WiFi.h>
#include "time.h"

const char* ssid     = "REPLACE_WITH_WIFI_SSID";
const char* password = "REPLACE_WITH_WIFI_PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
