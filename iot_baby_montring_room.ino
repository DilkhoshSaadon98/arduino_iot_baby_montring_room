// Temo Hum Sensor
#include <DHT.h>
// Servo Motor
#include <ESP32Servo.h>
// Firebase
#include <FirebaseESP32.h>
#include <Arduino.h>
#include <driver/adc.h>
Servo servo;
FirebaseData firebaseData;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long duration = 0;
unsigned long prevTime_T1 = millis();
unsigned long prevTime_T2 = millis();
unsigned long prevTime_T3 = millis();
#define WIFI_SSID "Tenda_79F720"       // WiFi SSID
#define WIFI_PASSWORD "dlo1223334444"  // WiFi Password

// Firebase Project API
#define API_KEY "y07OKVGpeeR0VMpbQM0ZVp5XnHrCBbUAXUJcT2R4"                // Firebase API Key
#define DATABASE_URL "https://iot---flutter-default-rtdb.firebaseio.com"  // Firebase Database URL
// Timing Variables:
long interval_T1 = 0;
long interval_T2 = 0;
long interval_T3 = 0;
unsigned long prevTime = 0;
const long thresholdUpdateInterval = 5000;  // Update thresholds every 5 seconds

//All pins needed to connect to board
//Digital pins
#define LED 13
#define dhtPin 27
const int servoPin = 14;
#define aspritor 12
//Analogs pins
const int soundPin = 35;
//Variable for Sensors Value:
int temreature = 0, humidity = 0, soundValue = 0;
//Variable for all Thresholds :
int soundThreShold = 0, humidityThreShold = 0;
int prevsoundThreShold = 0, prevhumidityThreShold = 0;
int servoAngle = 90;
DHT dht(dhtPin, DHT11);
String prevLedValue = "0", prevBedValue = "0", prevAspValue = "0";
//setup Function:
void setup() {
  Serial.begin(115200);
  servo.attach(servoPin);
  dht.begin();
  pinMode(LED, OUTPUT);
  pinMode(aspritor, OUTPUT);

  //Function To Connect To fireBase
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(10);
  }

  // Connected successfully => Turn off System Start LED (Red) and turn on Connection LED (Green)

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Connect to Firebase
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.signer.tokens.legacy_token = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.reconnectNetwork(true);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
}
//Loop Function:
void loop() {
  unsigned long currentTime = millis();
  soundValue = analogRead(soundPin);
  Serial.print("Sound value: ");
  Serial.println(soundValue);
  if (currentTime - prevTime > thresholdUpdateInterval) {
    tempreatureHumditiySensor();
    prevTime = currentTime;
  }
  // Assuming firebaseData is a FirebaseData object declared somewhere in your code

  Firebase.getString(firebaseData, "/ESP/DEVICES/BED");
  String currentBedValue = firebaseData.stringData();
  controllBed(currentBedValue);

  Firebase.getString(firebaseData, "/ESP/DEVICES/LED");
  String currentLedValue = firebaseData.stringData();
  if (currentLedValue != prevLedValue) {
    prevLedValue = currentLedValue;
    deviceControll("LED", LED);
  }
  Firebase.getString(firebaseData, "/ESP/DEVICES/ASPRITOR");
  String currentAspValue = firebaseData.stringData();
  if (currentAspValue != prevAspValue) {
    prevAspValue = currentAspValue;
    deviceControll("ASPRITOR", aspritor);
  }

  // If Auto Control System enabled:
  if (Firebase.getString(firebaseData, "/ESP/DEVICES/autoControl")) {
    String autoControl = firebaseData.stringData();
    if (autoControl == "1") {

      Firebase.setString(firebaseData, "/ESP/SENSOR/SOUND", soundValue);
      soundThreShold = getThreeshold("SOUND_THRESHOLD");
      humidityThreShold = getThreeshold("HUMIDITY_THRESHOLD");
      Serial.println(humidity);
      Serial.println(soundValue);
      Serial.println("----------------");
      Serial.println(humidityThreShold);
      Serial.println(soundThreShold);
      // If  quality or Humadity greater than thresholds And soundValue greater than soundThreShold :
      // Turn on Aspritor and shock bed for 30 seconds:
      if ((humidity > humidityThreShold) && soundValue > soundThreShold) {
        Serial.println("MODE ONE ");
        for (int i = 0; i <= 5; i++) {
          digitalWrite(aspritor, HIGH);
          servo.write(servoAngle);
          delay(500);
          digitalWrite(aspritor, HIGH);
          servo.write(0);
          delay(500);
        }
        digitalWrite(aspritor, LOW);
        servo.write(0);
      }
      // If  quality or Humadity greater than thresholds :
      // Turn on Aspritor for 5 seconds:
      else if (humidity > humidityThreShold) {
        Serial.println("MODE TOW ");
        digitalWrite(aspritor, HIGH);
        delay(5000);
        digitalWrite(aspritor, LOW);
      }
      // soundValue greater than soundThreShold :
      // shock bed for 10 seconds :
      else if (soundValue > soundThreShold) {
        Serial.println("MODE THREE ");
        for (int i = 0; i <= 10; i++) {
          servo.write(servoAngle);
          delay(400);
          servo.write(0);
          delay(400);
        }
        servo.write(0);
      }
    }
  }
}

//All Functions:
// Function for Controlling Devices (Led and Aspritor) :
void deviceControll(String deviceName, int pins) {
  if (Firebase.getString(firebaseData, "/ESP/DEVICES/" + deviceName)) {
    String deviceState = firebaseData.stringData();
    if (deviceState == "1") {
      Serial.println(deviceName + " ON ");
      digitalWrite(pins, 200);
    }
    if (deviceState == "0") {
      Serial.println(deviceName + " Off ");
      digitalWrite(pins, LOW);
    }
  }
}
// Function for Controlling Bed :
void controllBed(String bedValue) {
  // Your existing controllBed logic goes here
  // This function will be called only when the BED value changes
  Serial.print("BED value changed to: ");
  Serial.println(bedValue);

  // Add your controllBed function logic here
  if (bedValue == "1") {
    for (int i = 0; i <= 3; i++) {
      delay(500);
      Serial.println(F("Bed ON "));
      servo.write(servoAngle);  // Set servo to 90 degrees
      delay(500);
      servo.write(0);  // Set servo back to 0 degrees
    }
  } else if (bedValue == "0") {
    Serial.println(F("Bed OFF "));
    servo.write(0);
  }
}

void tempreatureHumditiySensor() {
  // If Sensor not Connected => sensor value will become '2147483647'
  // and we will send error value to firebase to show message in mobile app:

  temreature = dht.readTemperature();
  humidity = dht.readHumidity();
  //Print for arduino IDE:
  Serial.print("Tempreature Value : ");
  Serial.println(temreature);
  Serial.print("Humadity Value    : ");
  Serial.println(humidity);
  //Send to firebase
  Firebase.setString(firebaseData, "/ESP/SENSOR/TEMP", temreature);
  Firebase.setString(firebaseData, "/ESP/SENSOR/HUM", humidity);
}

int getThreeshold(String threShould) {
  int thresholdValue = 0;
  String path = "/ESP/THRESHOLDS/" + threShould;
  // Get value of Thresholds from Firebase :
  if (Firebase.getInt(firebaseData, path)) {
    thresholdValue = firebaseData.stringData().toInt();
  }
  //Print Value of Thresholds for Arduino IDE:
  Serial.print(threShould + " IS :");
  Serial.println(thresholdValue);
  return thresholdValue;
}
