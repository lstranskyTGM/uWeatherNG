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
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

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
// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, 60000);

ESP32Time rtc;

#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 5 * 60  // Time ESP32 will go to sleep (in seconds)

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR boolean requestedNTP;

BH1750 lightMeter;
Adafruit_BME280 bme; 

void setup() {
  Serial.begin(115200);

  ++bootCount;
  print_Status("BootCount: "+String(bootCount));

  connect_to_WiFi();

  get_network_info();

  if (requestedNTP == false) {
    request_NTP_set_RTC();
    requestedNTP = true;
  }

  connect_to_MySQL();

  // Disconnect from MySQL server
  conn.close();
  print_Status("Disconnected from MySQL server");

  // Disconnect from WiFi
  WiFi.disconnect();
  print_Status("Disconnected from WiFi");

  start_DeepSleep();
}

void loop() {
  // Not executed because of Deep Sleep
}

// Connect to Wi-Fi network
void connect_to_WiFi() {
  WiFi.begin(wifi_ssid, wifi_password);
  // Stuck in loop (needs to be fixed)
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    print_Status("Connecting to WiFi...");
  }
  print_Status("Connected to WiFi");
}

// Establish a MySQL connection
void connect_to_MySQL() {
  Serial.print("Connecting to MySQL Server...");
  if (conn.connect(db_server_addr, db_port, db_user, db_password)) {
    print_Status("Connected to MySQL server");
  }
  else {
    print_Status("MySQL connection failed");
    while (1);    // Stuck in loop (needs to be fixed)
  }
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
  // Set the rules for daylight saving time settings
  // timeClient.setDST("CEST", Last, Sun, Mar, 2, 120); // Last Sunday in March 2:00, timezone +120min (+1 GMT + 1h summertime offset)
  // timeClient.setSTD("CET", Last, Sun, Oct, 3, 60);  // Last Sunday in October 3:00, timezone +60min (+1 GMT)
  // ntp.updateInterval(1000); // update every second

  print_Status("Requesting NTP Server...");
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  print_Status("Request successfull");

  print_Status("Writing Time into RTC Module...");
  // Set rtc time using NTP
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
      rtc.setTimeStruct(timeinfo); 
  }
  print_Status("Write successfull");
}

// Starts DeepSleep and sets wakeup event
void start_DeepSleep() {
  print_Status("Setting DeepSleep WakeUp");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  print_Status("Starting DeepSleep...");
  Serial.flush(); // Waits for the transmisson of outgoing serial data to complete
  esp_deep_sleep_start();
}

// Initalizes and setups all Sensors
void initalize_Sensors() {
  Wire.begin();

  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23);

  // Stuck in loop (needs to be fixed)
  bool status;
  status = bme.begin(0x76); 
  if (!status) {
    print_Status("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void transfer_SensorData() {
  // Read KY018
  float lux = lightMeter.readLightLevel();

  // Read BME280
  bme.readTemperature();
  bme.readPressure() / 100.0F;
  bme.readHumidity();


}

void print_Status(String statusText) {
  Serial.println(statusText);
  // Status on display
}

