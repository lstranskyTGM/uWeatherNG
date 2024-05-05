/*
 * Test file for sending data to an MQTT Broker.
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

// Define the serial console for debug prints, if needed
// #define TINY_GSM_DEBUG SerialMon

// set GSM PIN, if any
#define GSM_PIN ""

// GPRS credentials
const char apn[] = "YourAPN";
const char gprsUser[] = "YourGPRSUsername";
const char gprsPass[] = "YourGPRSPassword";

// MQTT Broker details
const char* broker = "mqtt.broker.net";
const int port = 8883;
const char* client_id = "YourClientID"; 
const char* mqtt_username = "YourMQTTUsername";
const char* mqtt_password = "YourMQTTPassword";

// Other Settings
const bool retain_messages = false;
const int qos_level = 2;

// MQTT Topics
const char* topicSensors = "sensors/data";

// Include Libraries
#include <TinyGsmClient.h>
#include <PubSubClient.h>

// Create the modem object
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);

void setup() {
  // Initalize serial ports
  Serial.begin(115200);
  SerialAT.begin(UART_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

  // Initalize the modem
  Serial.println("Initalizing modem...");
  initModem();

  // Fetch modem information
  getModemInfo();

  // Connect to GPRS network
  connectToGPRS();

  // Setup MQTT connection
  mqttConnect();

  // Subscribe to MQTT topics
  subscribeToTopics(topicSensors);
}

void loop() {
  if (!mqtt.connected()) {
    mqttConnect();
  }
  mqtt.loop();
  delay(10000);
}

// Initializes the modem and reports status
void initModem() {
  if (!modem.init()) {
    Serial.println("Modem initialization failed");
    return;
  }
  Serial.println("Modem initialized successfully");

  // Call to unlock the SIM card
  unlockSIM();
}

// Retrieves and displays modem information
void getModemInfo() {
  Serial.println("Fetching modem info...");
    String modemInfo = modem.getModemInfo();
    if (modemInfo.length() > 0) {
        Serial.println("Modem Info: " + modemInfo);
    } else {
        Serial.println("Failed to retrieve modem information");
    }
}

// Function to unlock the SIM card using a PIN
void unlockSIM() {
  if (GSM_PIN && strlen(GSM_PIN) > 0 && modem.getSimStatus() != 3) {
    Serial.print("Unlocking SIM card with PIN...");
    if (!modem.simUnlock(GSM_PIN)) {
      Serial.println("Failed to unlock SIM card.");
      return;
    }
    Serial.println("SIM card unlocked successfully.");
  } else {
    Serial.println("No GSM PIN provided or PIN is empty.");
  }
}

// Connects to the GPRS network using provided credentials
void connectToGPRS() {
  Serial.println("Connecting to mobile network...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println("Failed to connect to mobile network");
  } else {
    Serial.println("Connected to mobile network");
  }
}

// Establishes connection with the MQTT broker
boolean mqttConnect() {
  Serial.print("Connecting to MQTT Broker...");
  Serial.print(broker);

  // Setup MQTT Broker
  mqtt.setServer(broker, port);
  mqtt.setCallback(mqttCallback);

  // Connect to MQTT Broker
  if (!mqtt.connect(client_id, mqtt_username, mqtt_password)) {
    Serial.println("Failed to connect to MQTT Broker");
    return false;
  }
  Serial.println("Connected to MQTT Broker");
  return true;
}

// MQTT callback function to handle incoming messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Publishes data to a specified MQTT topic
void publishToTopic(const char* topic, const char* data) {
  Serial.print("Publishing data...");
  Serial.println("Topic: ");
  Serial.print(topic);
  Serial.println("Data: ");
  Serial.print(data);

  if (!mqtt.publish(topic, data, retain_messages, qos_level) {
    Serial.println("Failed to publish data");
  } else {
    Serial.println("Data published successfully");
  }
}

// Subscribes to a specified MQTT topic
void subscribeToTopic(const char* topic) {
  if (mqtt.subscribe(topic)) {
    Serial.print("Subscribed successfully:");
    Serial.println(topic);
  } else {
    Serial.print("Subscribe failed:");
    Serial.println(topic);
  }
}
