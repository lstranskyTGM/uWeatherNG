/*
 * This is a test file for the SPI-Reader-Micro Module
 */

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

  // Example usage with a filepath
  char filepath[] = "test.txt";

  // Write File
  writeFile(filepath);

  // Read File
  readFile(filepath);

  // Delete File
  // deleteFile(filepath);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// Checks if File exists (False: attempts to create file)
bool checkFileExist(const char* filepath) {
  if (SD.exists(filepath))
    return true;
  else
  {
    Serial.println(String(filepath) + " doesn't exist.");
    // Open and close new File to creat it
    Serial.println("Creating " + String(filepath));
    myFile = SD.open(filepath, FILE_WRITE);
    myFile.close();
    // Check again if File exists
    if (SD.exists(filepath)) {
      Serial.println(String(filepath) + " successfully created");
      return true;
    }
    else {
      Serial.println("Error: " + String(filepath) + " can not be created");
      return false;
    }
  }
}

void writeFile(const char* filepath) {
  if (checkFileExist(filepath)) {
    // Open file
    myFile = SD.open(filepath, FILE_WRITE);
    // Write file
    Serial.println("Writing to " + String(filepath));
    myFile.println("Lorem ipsum dolor sit");
    // Close file
    myFile.close();
    // Status message
    Serial.println("Writing finished");
  } else {
    Serial.println("Error: Can't write File that does not exist / can not be created");
  }
}

void readFile(const char* filepath) {
  if (checkFileExist(filepath)) {
    // Open file
    myFile = SD.open(filepath, FILE_READ);
    // Read from the file until there's nothing else in it:
    Serial.println("Reading " + String(filepath));
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // Status message
    Serial.println("Reading finished");
  } else {
    Serial.println("Error: Can't read File that does not exist / can not be created");
  }
}

void deleteFile(const char* filepath) {
  if (SD.exists(filepath)) {
    if (SD.remove(filepath)) {
      Serial.println(String(filepath) + " has been deleted.");
    } else {
      Serial.println("Error: Could not delete " + String(filepath));
    }
  } else {
    Serial.println(String(filepath) + " does not exist, so it cannot be deleted.");
  }
}