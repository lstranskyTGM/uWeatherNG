/*
 * This is a test file for connecting to a wifi and then to a MySQL server.
 * There is also a short test query that gets executed.
 */

#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

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

void setup() {
  Serial.begin(9600);

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

  testSQL();

  // Disconnect from MySQL server
  conn.close();
  Serial.println("Disconnected from MySQL server");
}

void loop() {
  // Do nothing
}

// Tests if an SQL Command can be executed
void testSQL() {
  // Send an SQL command
  MySQL_Cursor *cursor = new MySQL_Cursor(&conn);
  char query = "INSERT INTO usr_web204_3.Messorte VALUES('Test');";
  cursor->execute(query);
  delete cursor;
}

// Gets all the network info
void get_network_info(){
    if(WiFi.status() == WL_CONNECTED) {
        Serial.print("[*] Network information for ");
        Serial.println(ssid);

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
