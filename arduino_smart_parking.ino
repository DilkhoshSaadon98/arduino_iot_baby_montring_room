#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16x2 display

const int numParkingSpaces = 14;
const int doorServoPin = 9;
const int irSensorPins[numParkingSpaces] = {2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 16};

bool parkingSpaces[numParkingSpaces] = {false};  // false means empty, true means occupied

void setup() {
  lcd.begin(16, 2);
  lcd.print("Smart Parking");
  delay(2000);
  lcd.clear();

  pinMode(doorServoPin, OUTPUT);
  closeDoor();  // Initial state: door closed

  for (int i = 0; i < numParkingSpaces; i++) {
    pinMode(irSensorPins[i], INPUT);
  }
}

void loop() {
  updateParkingStatus();
  displayParkingStatus();

  if (isCarDetected()) {
    openDoor();
    delay(5000);  // Wait for 5 seconds to simulate car parking time
    closeDoor();
  }

  delay(1000);  // Adjust the delay according to your system requirements
}

void updateParkingStatus() {
  for (int i = 0; i < numParkingSpaces; i++) {
    parkingSpaces[i] = digitalRead(irSensorPins[i]);
  }
}

void displayParkingStatus() {
  lcd.clear();
  lcd.print("Parking Status:");

  for (int i = 0; i < numParkingSpaces; i++) {
    lcd.setCursor(0, i + 1);
    lcd.print("Space ");
    lcd.print(i + 1);
    lcd.print(": ");
    lcd.print(parkingSpaces[i] ? "Occupied" : "Empty");
  }

  if (isParkingFull()) {
    lcd.setCursor(0, numParkingSpaces + 1);
    lcd.print("Garage Full!");
  }
}

bool isCarDetected() {
  for (int i = 0; i < numParkingSpaces; i++) {
    if (parkingSpaces[i]) {
      return true;  // Car detected in any space
    }
  }
  return false;
}

bool isParkingFull() {
  for (int i = 0; i < numParkingSpaces; i++) {
    if (!parkingSpaces[i]) {
      return false;  // At least one space is empty
    }
  }
  return true;  // All spaces are occupied
}

void openDoor() {
  lcd.clear();
  lcd.print("Opening Door...");
  delay(1000);
  lcd.clear();
  lcd.print("Door Opened!");
  digitalWrite(doorServoPin, HIGH);
  delay(2000);  // Adjust the delay according to your servo motor
  digitalWrite(doorServoPin, LOW);
}

void closeDoor() {
  lcd.clear();
  lcd.print("Closing Door...");
  delay(1000);
  lcd.clear();
  lcd.print("Door Closed!");
  digitalWrite(doorServoPin, HIGH);
  delay(2000);  // Adjust the delay according to your servo motor
  digitalWrite(doorServoPin, LOW);
}
