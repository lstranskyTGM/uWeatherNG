/*
 * This is a test file for connecting to a wifi and then to a MySQL server.
 * There is also a short test query that gets executed.
 */

#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

const char* SSID = "REPLACE_WITH_WIFI_SSID";
const char* PASS = "REPLACE_WITH_WIFI_PASSWORD";

WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

IPAddress server_addr(0, 0, 0, 0);
char user[] = "REPLACE_WITH_DB_USER"; 
char password[] = "REPLACE_WITH_DB_PASSWORD";

void setup() {
  Serial.begin(9600);
}

void loop() {
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  conn.connect(server_addr, 3306, user, password);
  delay(1000);
  get_network_info();
  cursor = new MySQL_Cursor(&conn);
  char statementChar[256];
  String statementStr = "INSERT INTO usr_web204_3.Messorte VALUES('Test');";
  statementStr.toCharArray (statementChar, statementStr.length());
  cursor->execute(statementChar);
  conn.close();
  WiFi.disconnect();
  delay(5000);
}

// Gets all the network info
void get_network_info(){
    if(WiFi.status() == WL_CONNECTED) {
        Serial.print("[*] Network information for ");
        Serial.println(SSID);

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
