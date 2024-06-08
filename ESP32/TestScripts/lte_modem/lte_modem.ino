/**
 * Modified example sketch to test mqtt, gps and ntp functions for the lte modem
 */

/******* ORIGINAL BOTLETICS EXAMPLE SKETCH COMMENT *******/
/*  This is an example sketch to test the core functionalities of the SIM7000/7070/7500/7600 modules.
 *  Please see the "LTE_Demo" sketch which supports many other SIMCom 2G, 3G modules; this sketch
 *  takes up less memory than the LTE_Demo sketch and is therefore suitable for microcontrollers
 *  like the ATmega32u4.
 *  
 *  Author: Timothy Woo (www.botletics.com)
 *  Github: https://github.com/botletics/SIM7000-LTE-Shield
 *  Last Updated: 11/22/2022
 *  License: GNU GPL v3.0
 */

#include "BotleticsSIM7000.h" // https://github.com/botletics/Botletics-SIM7000/tree/main/src

#define SIMCOM_7000
#define BOTLETICS_PWRKEY 18
#define RST 5
#define TX 16 // ESP32 hardware serial RX2 (GPIO16)
#define RX 17 // ESP32 hardware serial TX2 (GPIO17)

#define MQTT_SERVER      "mqtt.broker.net"
#define MQTT_PORT        1883
#define MQTT_USERNAME    "YourMQTTUsername"
#define MQTT_PASSWORD    "YourMQTTPassword"
#define MQTT_CLIENTID    "YourClientID"

// For ESP32 hardware serial
#include <HardwareSerial.h>
HardwareSerial modemSS(1);

Botletics_modem_LTE modem = Botletics_modem_LTE();

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char replybuffer[255]; // this is a large buffer for replies
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!

void setup() {
  //  while (!Serial);

  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH); // Default state

  // Turn on the module by pulsing PWRKEY low for a little bit
  // This amount of time depends on the specific module that's used
  modem.powerOn(BOTLETICS_PWRKEY); // Power on the module

  Serial.begin(9600);
  Serial.println(F("ESP32 SIMCom Basic Test"));
  Serial.println(F("Initializing....(May take several seconds)"));

  // SIM7000 takes about 3s to turn on and SIM7500 takes about 15s
  // Press reset button if the module is still turning on and the board doesn't find it.
  // When the module is on it should communicate right after pressing reset
  
  // Start at default SIM7000 shield baud rate
  modemSS.begin(115200, SERIAL_8N1, TX, RX); // baud rate, protocol, ESP32 RX pin, ESP32 TX pin

  Serial.println(F("Configuring to 9600 baud"));
  modemSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  modemSS.begin(9600, SERIAL_8N1, TX, RX); // Switch to 9600
  if (! modem.begin(modemSS)) {
    Serial.println(F("Couldn't find modem"));
    while (1); // Don't proceed if it couldn't find the device
  }

  type = modem.type();
  Serial.println(F("Modem is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case SIM7000:
      Serial.println(F("SIM7000")); break;
    default:
      Serial.println(F("???")); break;
  }

  // Print module IMEI number.
  uint8_t imeiLen = modem.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  // Set modem to full functionality
  modem.setFunctionality(1); // AT+CFUN=1

  modem.setNetworkSettings(F("webLGaut")); // For HoT IoT Simcard

  printMenu();
}

void printMenu(void) {
  Serial.println(F("-------------------------------------"));
  // General
  Serial.println(F("[?] Print this menu"));

  // Time
  Serial.println(F("[y] Enable local time stamp (SIM800/808/70X0)"));
  Serial.println(F("[Y] Enable NTP time sync (SIM800/808/70X0)")); // Need to use "G" command first!
  Serial.println(F("[t] Get network time")); // Works just by being connected to network

   // MQTT
  Serial.println(F("[Q] Post via old MQTT methods"));
  Serial.println(F("[q] Post via new MQTT methods"));

  // GPS
  if (type >= SIM808_V1) {
    Serial.println(F("[O] Turn GPS on (SIM808/5320/7XX0)"));
    Serial.println(F("[o] Turn GPS off (SIM808/5320/7XX0)"));
    Serial.println(F("[x] GPS fix Status (SIM808/5320/7XX0)"));
    Serial.println(F("[L] Query GPS location (SIM808/5320/7XX0)"));
  }
  
  Serial.println(F("-------------------------------------"));
  Serial.println(F(""));
}

void loop() {
  Serial.print(F("modem> "));
  while (! Serial.available() ) {
    if (modem.available()) {
      Serial.write(modem.read());
    }
  }

  char command = Serial.read();
  Serial.println(command);


  switch (command) {
    case '?': {
        printMenu();
        break;
      }

// ===================================================================================================================

// MQTT Test

case 'Q': {       
        Serial.print(F("Connecting TCP to MQTT..."));
        
        if (!modem.TCPconnect(MQTT_SERVER, MQTT_PORT)) Serial.println(F("Failed to connect to TCP/IP!"));

        Serial.print(F("Connecting MQTT Broker..."));
        
        if (! modem.MQTTconnect("MQIsdp", MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) Serial.println(F("Failed to connect to broker!"));
              
        // Read voltage value
        uint16_t vbat1;
        if (! modem.getBattVoltage(&vbat1)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VBat = ")); Serial.print(vbat1); Serial.println(F(" mV"));
        }

        char battBuff[12];
        dtostrf(vbat1, 1, 0, battBuff);
        
        // Publish message
        if (! modem.MQTTpublish("voltage", battBuff)) Serial.println(F("Failed to publish!"));

        modem.MQTTdisconnect();
        modem.TCPclose();

        break;
}
 
// MQTT Test2

case 'q': {       
        Serial.print(F("Connecting to MQTT Broker..."));
        

        modem.openWirelessConnection(true);

        // Set up MQTT parameters (see MQTT app note for explanation of parameter values)
        modem.MQTT_setParameter("URL", MQTT_SERVER, MQTT_PORT);
        // Set up MQTT username and password if necessary
        modem.MQTT_setParameter("USERNAME", MQTT_USERNAME);
        modem.MQTT_setParameter("PASSWORD", MQTT_PASSWORD);
        modem.MQTT_setParameter("CLIENTID", MQTT_CLIENTID);
        
    
        Serial.println(F("Connecting to MQTT broker..."));
        if (! modem.MQTT_connect(true)) Serial.println(F("Failed to connect to broker!"));
      
        // Read voltage value
        uint16_t vbat1;
        if (! modem.getBattVoltage(&vbat1)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VBat = ")); Serial.print(vbat1); Serial.println(F(" mV"));
        }

        char battBuff[12];
        dtostrf(vbat1, 1, 0, battBuff);

        if (!modem.MQTT_publish("voltage2", battBuff, strlen(battBuff), 1, 0)) Serial.println(F("Failed to publish!")); 
  
        modem.MQTT_connect(false);
        modem.openWirelessConnection(false);

        break;
}
    
// ==========================================================================================

    /*** Time ***/

    case 'y': {
        // enable network time sync
        if (!modem.enableRTC(true))
          Serial.println(F("Failed to enable"));
        break;
      }

    case 'Y': {
        // enable NTP time sync
        if (!modem.enableNTPTimeSync(true, F("pool.ntp.org")))
          Serial.println(F("Failed to enable"));
        break;
      }

    case 't': {
        // read the time
        char buffer[23];

        modem.getTime(buffer, 23);  // make sure replybuffer is at least 23 bytes!
        Serial.print(F("Time = ")); Serial.println(buffer);
        break;
      }


    /*********************************** GPS */

    case 'o': {
        // turn GPS off
        if (!modem.enableGPS(false))
          Serial.println(F("Failed to turn off"));
        break;
      }
    case 'O': {
        // turn GPS on
        if (!modem.enableGPS(true))
          Serial.println(F("Failed to turn on"));
        break;
      }
    case 'x': {
        int8_t stat;
        // check GPS fix
        stat = modem.GPSstatus();
        if (stat < 0)
          Serial.println(F("Failed to query"));
        if (stat == 0) Serial.println(F("GPS off"));
        if (stat == 1) Serial.println(F("No fix"));
        if (stat == 2) Serial.println(F("2D fix"));
        if (stat == 3) Serial.println(F("3D fix"));
        break;
      }

    case 'L': {
        /*
        // Uncomment this block if all you want to see is the AT command response
        // check for GPS location
        char gpsdata[120];
        modem.getGPS(0, gpsdata, 120);
        if (type == SIM808_V1)
          Serial.println(F("Reply in format: mode,longitude,latitude,altitude,utctime(yyyymmddHHMMSS),ttff,satellites,speed,course"));
        else if ( (type == SIM5320A) || (type == SIM5320E) || (type == SIM7500) || (type == SIM7600) )
          Serial.println(F("Reply in format: [<lat>],[<N/S>],[<lon>],[<E/W>],[<date>],[<UTC time>(yyyymmddHHMMSS)],[<alt>],[<speed>],[<course>]"));
        else
          Serial.println(F("Reply in format: mode,fixstatus,utctime(yyyymmddHHMMSS),latitude,longitude,altitude,speed,course,fixmode,reserved1,HDOP,PDOP,VDOP,reserved2,view_satellites,used_satellites,reserved3,C/N0max,HPA,VPA"));
        
        Serial.println(gpsdata);

        break;
        */

        float latitude, longitude, speed_kph, heading, altitude; 
        // Comment out the stuff below if you don't care about UTC time
        /*        float second;
        uint16_t year;
        uint8_t month, day, hour, minute;
        */
        // Use the top line if you want to parse UTC time data as well, the line below it if you don't care
//        if (modem.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude, &year, &month, &day, &hour, &minute, &second)) {
        if (modem.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) { // Use this line instead if you don't want UTC time
          Serial.println(F("---------------------"));
          Serial.print(F("Latitude: ")); Serial.println(latitude, 6);
          Serial.print(F("Longitude: ")); Serial.println(longitude, 6);
          Serial.print(F("Speed: ")); Serial.println(speed_kph);
          Serial.print(F("Heading: ")); Serial.println(heading);
          Serial.print(F("Altitude: ")); Serial.println(altitude);
          // Comment out the stuff below if you don't care about UTC time
          /*
          Serial.print(F("Year: ")); Serial.println(year);
          Serial.print(F("Month: ")); Serial.println(month);
          Serial.print(F("Day: ")); Serial.println(day);
          Serial.print(F("Hour: ")); Serial.println(hour);
          Serial.print(F("Minute: ")); Serial.println(minute);
          Serial.print(F("Second: ")); Serial.println(second);
          Serial.println(F("---------------------"));
          */
        }

        break;
      }


    default: {
        Serial.println(F("Unknown command"));
        printMenu();
        break;
      }
  }
  // flush input
  flushSerial();
  while (modem.available()) {
    Serial.write(modem.read());
  }

}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}
uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
