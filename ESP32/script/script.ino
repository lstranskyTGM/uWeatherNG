#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

IPAddress server_addr(192, 168, 1, 100); // Replace with your MySQL server IP address
char user[] = "your_USERNAME"; // Replace with your MySQL username
char password[] = "your_PASSWORD"; // Replace with your MySQL password
char db_name[] = "your_DATABASE"; // Replace with your MySQL database name
void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  get_network_info();

  // Establish a MySQL connection
  MySQL_Connection conn((Client *)&WiFiClient());
  if (conn.connect(server_addr, 3306, user, password)) {
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
  char query[] = "INSERT INTO table_name (column1, column2) VALUES ('value1', 'value2')";
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
