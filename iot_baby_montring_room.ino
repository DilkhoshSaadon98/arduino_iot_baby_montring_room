// Gas Sensor
#include <MQ135.h>
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
#define WIFI_SSID "Tenda_79F720"          // WiFi SSID
#define WIFI_PASSWORD "dlo1223334444"     // WiFi Password

// Firebase Project API
#define API_KEY "y07OKVGpeeR0VMpbQM0ZVp5XnHrCBbUAXUJcT2R4"  // Firebase API Key
#define DATABASE_URL "https://iot---flutter-default-rtdb.firebaseio.com"  // Firebase Database URL
long interval_T1 = 0;
long interval_T2 = 0;
long interval_T3 = 0;
//All pins needed to connect to board
//Digital pins
#define LED 13
#define dhtPin 27
#define servoPin 14
#define aspritor 12
//Analogs pins
const int mq135Pin = 35;
const int soundPin = 34;
//Variable for Sensors Value:
int temreature = 0, humidity = 0, soundValue = 0, airQuality = 0;
//Variable for all Thresholds :
float airQualityThree = 0, soundThreShold = 0, humidityThreShold = 0;
DHT dht(dhtPin, DHT11);

//setup Function:
void setup() {
  Serial.begin(115200);
  servo.attach(servoPin);
  dht.begin();
  pinMode(LED, OUTPUT);
  pinMode(aspritor, OUTPUT);
  pinMode(servoPin, OUTPUT);
  pinMode(mq135Pin, INPUT);
  pinMode(soundPin, INPUT);

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
          servo.write(280);
  unsigned long currentTime = millis();
  if (currentTime - prevTime_T1 > interval_T1) {
    //Controlling Devices:
    // Bed :
    controllBed();
    // Led :
    deviceControll("LED", LED);
    // Aspritor :
    deviceControll("ASPRITOR", aspritor);
    //Get Sound and Air Quality Values:
    soundValue = analogRead(soundPin);
    airQuality = analogRead(mq135Pin) ;
    // Print Sound and Air Quality Values for Arduino IDE:
    Serial.print("Sound value: ");
    Serial.println(soundValue);
    Serial.print("MQ-135 Value: ");
    Serial.println(airQuality);
    // Send Sound and Air Quality Values to Firebase:
    Firebase.setString(firebaseData, "/ESP/SENSOR/SOUND", soundValue);
    Firebase.setString(firebaseData, "/ESP/SENSOR/AIR_QUALITY", airQuality);
    prevTime_T1 = currentTime;
  }
  if (currentTime - prevTime_T2 > interval_T2) {
    //Function to get Tempreature and Humadity Value:
    tempreatureHumditiySensor();
    prevTime_T2 = currentTime;
  }
  if (currentTime - prevTime_T3 > interval_T3) {
    // Get and Set Values of Thresholds :
    soundThreShold = getThreeshold("SOUND_THRESHOLD");
    airQualityThree = getThreeshold("AIR_QUALITY_THRESHOLD");
    humidityThreShold = getThreeshold("HUMIDITY_THRESHOLD");
    prevTime_T3 = currentTime;
  }
  // If Auto Control System enabled:
  if (Firebase.getString(firebaseData, "/ESP/DEVICES/autoControl")) {
    String autoControl = firebaseData.stringData();
    if (autoControl == "1") {
      // If air quality or Humadity greater than thresholds And soundValue greater than soundThreShold :
      // Turn on Aspritor and shock bed for 30 seconds:
      if ((airQuality >= airQualityThree || humidity >= humidityThreShold) && soundValue >= soundThreShold) {

        for (int i = 0; i <= 15; i++) {
          digitalWrite(aspritor, HIGH);
          servo.write(280);
          delay(500);
          digitalWrite(aspritor, HIGH);
          servo.write(0);
          delay(500);
        }
        digitalWrite(aspritor, LOW);
        servo.write(0);
      }
      // If air quality or Humadity greater than thresholds :
      // Turn on Aspritor for 15 seconds:
      else if (airQuality >= airQualityThree || humidity >= humidityThreShold) {
        digitalWrite(aspritor, HIGH);
        delay(15000);
        digitalWrite(aspritor, LOW);
      }
      // soundValue greater than soundThreShold :
      // shock bed for 30 seconds :
      else if (soundValue >= soundThreShold) {
        for (int i = 0; i <= 30; i++) {
          servo.write(280);
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
void controllBed() {
  if (Firebase.getString(firebaseData, "/ESP/DEVICES/BED")) {
    String bed = firebaseData.stringData();
    if (bed == "1") {
      for (int i = 0; i <= 5; i++) {
        delay(500);
        Serial.println(F("Bed ON "));
        servo.write(45);
        delay(500);
        servo.write(0);
      }
    } else if (bed == "0") {
      Serial.println(F("Bed OFF "));
      servo.write(0);
    }
  }
}
//Function to Get Threesholds from Firebase :
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


void tempreatureHumditiySensor() {
  // If Sensor not Connected => sensor value will become '2147483647'
  // and we will send error value to firebase to show message in mobile app:
  
    temreature = dht.readTemperature()  ;
    humidity = dht.readHumidity() ;
    //Print for arduino IDE:
    Serial.print("Tempreature Value : ");
    Serial.println(temreature);
    Serial.print("Humadity Value    : ");
    Serial.println(humidity);
    //Send to firebase
    Firebase.setString(firebaseData, "/ESP/SENSOR/TEMP", temreature);
    Firebase.setString(firebaseData, "/ESP/SENSOR/HUM", humidity);
  
}
// Function to Connect to Firebase :
// void connectToFirebase() {

//   const char* ssid = "Tenda_79F720";
//   const char* password = "dlo1223334444";
//   delay(10);
//   Serial.println();
//   Serial.print("Connecting with ");
//   Serial.println(ssid);
//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.println("Not Connected");
//   }
//   Serial.println("");
//   Serial.print("WiFi conected. IP: ");
//   Serial.println(WiFi.localIP());
//   // Firebase Project IDLink and passcode ;
//   Firebase.begin("https://iot---flutter-default-rtdb.firebaseio.com/", "ZUnNUfxZEMAIFrHSPW8KkYDG8hRqWgvRFhYWvSbP");
//   duration = millis();
// }