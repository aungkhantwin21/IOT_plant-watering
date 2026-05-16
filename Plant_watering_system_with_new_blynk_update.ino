/************** Blynk Setup **************/
#define BLYNK_TEMPLATE_ID "TMPL6CNewrKoa"
#define BLYNK_TEMPLATE_NAME "venian"

#define BLYNK_AUTH_TOKEN "Mu911co212tOO3u_2FbBoUqWfmmU-DtQ"

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

/************** LCD Setup **************/
LiquidCrystal_PCF8574 lcd(0x27); // I2C address may vary: 0x27 or 0x3F

/************** Pin Definitions **************/
const int soilMoisturePin = A0;  // Soil Moisture Sensor
const int relayPin = D5;         // Relay (Motor Control)

/************** WiFi Credentials **************/
char auth[] = "Mu911co212tOO3u_2FbBoUqWfmmU-DtQ";
char ssid[] = "iPhone";
char pass[] = "11111110";

/************** Variables **************/
bool motorState = false;
bool autoMode = true; // Default Auto Mode

/************** Function Prototypes **************/
int readMoisture();
void sendMotorNotification(bool state);

/************** Setup **************/
void setup() {
  Serial.begin(115200);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.setCursor(0, 0);
  lcd.print("Let's water");
  lcd.setCursor(0, 1);
  lcd.print("System Start...");
  delay(2000);
  lcd.clear();

  // Initialize Relay
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Motor OFF at start

  // Connect WiFi & Blynk
  WiFi.begin(ssid, pass);
  Blynk.begin(auth, ssid, pass);
}

/************** Loop **************/
void loop() {
  Blynk.run();

  int moisturePercentage = readMoisture();
  Blynk.virtualWrite(V2, moisturePercentage); // Send soil data to Blynk app

  static bool lastMotorState = false; // Track previous state

  // --- Auto Mode ---
  if (autoMode) {
    if (moisturePercentage <= 40 && !motorState) { // Dry
      motorState = true;
      digitalWrite(relayPin, LOW);  // Turn ON motor
    } 
    else if (moisturePercentage >= 80 && motorState) { // Wet
      motorState = false;
      digitalWrite(relayPin, HIGH); // Turn OFF motor
    }
  }

  // Send notifications if motor state changes
  if (motorState != lastMotorState) {
    sendMotorNotification(motorState);
    lastMotorState = motorState;
  }

  // --- Display on LCD ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moisturePercentage);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Motor: ");
  lcd.print(motorState ? "ON " : "OFF");

  delay(1000);
}

/************** Read Moisture Function **************/
int readMoisture() {
  int sensorValue = analogRead(soilMoisturePin);
  int percentage = map(sensorValue, 1023, 0, 0, 100); // Adjust mapping
  return percentage;
}

/************** Blynk Virtual Pins **************/
// V3 → Mode Switch (Auto/Manual)
BLYNK_WRITE(V3) {
  autoMode = param.asInt(); // 1 = Auto, 0 = Manual
}

// V1 → Manual Motor Control
BLYNK_WRITE(V1) {
  if (!autoMode) { // Only in Manual Mode
    bool newState = param.asInt(); // 1 = ON, 0 = OFF
    if (newState != motorState) {
      motorState = newState;
      digitalWrite(relayPin, motorState ? LOW : HIGH);
      sendMotorNotification(motorState);
    }
  }
}

/************** Notification Function **************/
void sendMotorNotification(bool state) {
  if (state) {
    Blynk.logEvent("motor_on", "Motor has been turned ON");
  } else {
    Blynk.logEvent("motor_off", "Motor has been turned OFF");
  }
}
