#include <SPI.h>
#include <SD.h>

File myFile;

#define chipSelect 34

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Serial.print("Initializing SD card...");

  // Check if Card is connected
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // Open File
  myFile = SD.open("test.txt", FILE_WRITE);

  // Read File
  readFile();

  // Write File
  writeFile();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void readFile() {

}
