/*
 * This is a test file for the SPI-Reader-Micro Module
 */

#include <SPI.h>
#include <SD.h>

File myFile;

// Change this to Module CS pin
#define chipSelect 34

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.print("Initializing SD card...");

  // Check if Card is connected
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // Example usage with a filepath
  char filepath[] = "test.txt";
  char textToWrite[] = "Lorem ipsum dolor sit";

  // Write File
  writeFile(filepath, textToWrite);

  // Read File
  readFile(filepath);

  // Delete File
  deleteFile(filepath);
}

void loop() {
  // Not used in this script
}

void writeFile(const char* filepath, const char* text) {
  // Open file for writing
  myFile = SD.open(filepath, FILE_WRITE);
  if (myFile) {
    Serial.println("Writing to " + String(filepath));
    myFile.println(text); // Write text to file
    myFile.close(); // Close the file
    Serial.println("Writing finished.");
  } else {
    Serial.println("Error opening " + String(filepath) + " for writing.");
  }
}

void readFile(const char* filepath) {
  // Open file for reading
  myFile = SD.open(filepath, FILE_READ);
  if (myFile) {
    Serial.println("Reading " + String(filepath));
    while (myFile.available()) {
      Serial.write(myFile.read()); // Read and print the content
    }
    myFile.close(); // Close the file
    Serial.println("Reading finished."); 
  } else {
    Serial.println("Error opening " + String(filepath) + " for reading.");
  }
}

void deleteFile(const char* filepath) {
  // Remove file
  if (SD.remove(filepath)) {
    Serial.println(String(filepath) + " has been deleted.");
  } else {
    Serial.println("Error: Could not delete " + String(filepath));
  }
}