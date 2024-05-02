/*
 * Test file for reading the GPS Data from the SIM7000E module.
 */

// Set Hardware serial for AT commands (to the module)
#define SerialAT Serial1
// Define RX and TX pins for SIM7000E connection
#define UART_BAUD 115200
#define RX_PIN 16
#define TX_PIN 17
//#define DTR_PIN 25
//#define PWR_PIN 23

// Select your modem:
//#define TINY_GSM_MODEM_SIM7000 // (8 simultaneous connections)
#define TINY_GSM_MODEM_SIM7000SSL // Supports SSL (2 simultaneous connections)

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Include Libraries
#include <TinyGsmClient.h>

// Create the modem object
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

void setup() {
  // Initalize serial ports
  Serial.begin(115200);
  SerialAT.begin(UART_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

  // Initalize the modem
  initModem();

  // Enable GPS
  enableGPS();
}

void loop() {
  // Get GPS Data
  getGPSData();
  // Delay for 10 seconds before the next GPS read
  delay(10000);
}

// Initializes the modem and reports status
void initModem() {
  if (!modem.init()) {
    Serial.println("Modem initialization failed!");
    return;
  }
  Serial.println("Modem initialized successfully.");
}

// Enables the GPS power, required for data acquisition
void enableGPS() {
  modem.sendAT("+CGNSPWR=1");
  if (modem.waitResponse(10000) != 1) {
    Serial.println("Failed to turn on GPS.");
    return;
  }
  Serial.println("GPS is powered on.");
}

// Disables the GPS to save power
void disableGPS() {
  modem.sendAT("+CGNSPWR=0");
  if (modem.waitResponse(10000) != 1) {
    Serial.println("Failed to turn off GPS.");
    return;
  }
  Serial.println("GPS is powered off.");
}

// Gets the GPS data
void getGPSData() {
  float lat, lon, speed, altitude, accuracy;
  int visibleSatellites, usedSatellites;
  bool isValid = false;
  
  if (modem.getGPS(&lat, &lon, &speed, &altitude, &visibleSatellites, &usedSatellites, &accuracy, &isValid)) {
    if (isValid) {
      displayGPSData(lat, lon, speed, altitude, visibleSatellites, usedSatellites, accuracy);
    } else {
      Serial.println("GPS data not valid yet.");
    }
  } else {
    Serial.println("Failed to get GPS data.");
  }
}

// Outputs the GPS data to the serial monitor
void displayGPSData(float lat, float lon, float speed, float altitude, int visibleSatellites, int usedSatellites, float accuracy) {
  Serial.print("Latitude: "); Serial.println(lat, 6);
  Serial.print("Longitude: "); Serial.println(lon, 6);
  Serial.print("Speed: "); Serial.println(speed);
  Serial.print("Altitude: "); Serial.println(altitude);
  Serial.print("Visible Satellites: "); Serial.println(visibleSatellites);
  Serial.print("Used Satellites: "); Serial.println(usedSatellites);
  Serial.print("Accuracy: "); Serial.println(accuracy, 6);
}

// Fetches the GPS data
void fetchGPSData() {
  // modem.sendAT("+CGNSINF");
}

// Parses the fetched GPS data to the serial monitor
void parseFetchedData() {
  // Implement Here
}

// Maybe save fetched data
// For example in SD

// The GPS Start Methods
// Cold Start (<35s): No prior data, acquires all info from scratch.
// Hot Start (<1s): Uses stored data for quick satellite reacquisition.
// Warm Start: Mix of cold and hot, some stored data used.
// A-GPS: Assists with network resources for faster, more accurate data.

// Function for Hot Start
void hotStartGPS() {
  // modem.sendAT("+CGNSHOT");
}

// Function for A-GPS (Assisted GPS)
void enableAGPS() {
  // Implement Here
}