
/*
SMART DOOR - ESP32-C6
*/

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID        "TMPL5u-utJgwZ"   // van je template
#define BLYNK_TEMPLATE_NAME      "SmartDoor"
#define BLYNK_DEVICE_NAME        "SmartDoorESP"

#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>


// --------------
// Pin definities
// --------------
#define SERVO_PIN     18
#define LED_GREEN     15
#define LED_RED       21
#define BUZZER        22

#define BTN1          6
#define BTN2          7
#define BTN3          5
#define BTN4          17

#define HALL_SENSOR   16
#define TOUCH_SENSOR  23

#define SDA_PIN       3
#define SCL_PIN       2


// ----------
// Variabelen
// ----------
Servo myServo;

String inputCode = "";
String correctCode = "1234";

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long doorOpenTime = 0;
unsigned long lastButtonTime = 0;
unsigned long lastHallCheck = 0;
unsigned long lastBellTime = 0;
unsigned long alarmTimer = 0;
unsigned long doorUnlockTime = 0;

bool doorIsOpen = false;
bool doorIsLocked = true;

int failedAttempts = 0;
bool alarmActive = false;

// -- Blynk --
char auth[] = "ddgrXLTzgACSejfWMvW3ZiWsoDXDOV9C";   // van je device
char ssid[] = "";
char pass[] = "";


// -----
// Setup
// -----
void setup() {
  Serial.begin(115200);
  delay(500);

  Blynk.begin(auth, ssid, pass);
  
  // -- LCD --
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();

  delay(800);

  // -- Servo --
  myServo.attach(SERVO_PIN);
  delay(200);
  myServo.write(0);

    // -- Hardware --
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);

  pinMode(HALL_SENSOR, INPUT_PULLUP);
  pinMode(TOUCH_SENSOR, INPUT);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

  Serial.println("Smart Door gestart");
  
  updateLCD();
}


// ----
// Loop
// ----
void loop() {
  Blynk.run();

  checkButton(BTN1, "1");
  checkButton(BTN2, "2");
  checkButton(BTN3, "3");
  checkButton(BTN4, "4");

  // -- Alarm reset --
  if (digitalRead(BTN1) == LOW && digitalRead(BTN2) == LOW && alarmActive) {
    if (millis() - lastButtonTime > 300) {
      resetAlarm();
      lastButtonTime = millis();
    }
  }

  // -- Deur openen langs binnen (1 touch) --
  if (digitalRead(TOUCH_SENSOR) == HIGH && !alarmActive) {
    if (millis() - lastBellTime > 500) {
      openDoor();
      lastBellTime = millis();
    }
  }

  // -- Hall sensor check elke 200ms --
  if (millis() - lastHallCheck > 200) {
    checkDoorLogic();
    lastHallCheck = millis();
  }


  if (alarmActive && millis() - alarmTimer > 200) {
    digitalWrite(LED_RED, !digitalRead(LED_RED));
    tone(BUZZER, 600, 150);

    alarmTimer = millis();

    updateLCD();
  }
}


// -------
// Knoppen
// -------
void checkButton(int pin, String value) {
  if (digitalRead(pin) == LOW && !alarmActive) {
    if (millis() - lastButtonTime > 200) {
      inputCode += value;
      lastButtonTime = millis();
      updateLCD();

      if (inputCode.length() >= 4) {
        checkCode();
        inputCode = "";
        updateLCD();
      }
    }
  }
}



// -----
// Alarm
// -----
void checkCode() {
  if (inputCode == correctCode) {
    failedAttempts = 0;
    openDoor();

  } else {
    failedAttempts++;

    if (failedAttempts >= 3) activateAlarm();
    else tone(BUZZER, 500, 150);
  }

  updateLCD();
}


// ------------
// Servo open u
// ------------
void openDoor() {
  myServo.write(90);  // ontgrendelen
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  tone(BUZZER, 1500, 300);

  doorIsOpen = true;
  doorIsLocked = false;
  doorOpenTime = millis();
  doorUnlockTime = millis();

  updateLCD();
}


// -----------------------------------
// "Fysieke" deur open/dicht (magneet)
// -----------------------------------
void checkDoorLogic() {
  bool magnetDetected = (digitalRead(HALL_SENSOR) == LOW);

  // -- Deur is dicht (magneet eraan) --
  if (magnetDetected) {

    // -- Nog binnen ontgrendel-tijd → deur nog NIET sluiten --
    if (millis() - doorUnlockTime < 5000) return;

    // -- Te lang gewacht → deur sluiten --
    if (doorIsOpen) closeAndLockDoor();
    return;
  }

  // -- Deur open --
  if (!doorIsOpen) {
    doorIsOpen = true;
    doorOpenTime = millis();
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    updateLCD();
  }

  // Deur te lang open (10s, zou leuker zijn met beweginssensor, ik heb er (nog) geen ): ) → waarschuwing
  if (millis() - doorOpenTime > 10000) {
    playWarningBeep();
    doorOpenTime = millis(); // Herhalend
    updateLCD();
  }
}


// ---------
// Servo def
// ---------
void closeAndLockDoor() {
  myServo.write(0);  // vergrendelen

  doorIsOpen = false;
  doorIsLocked = true;

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);

  updateLCD();
}


// ------------
// Waarschuwing
// ------------
void playWarningBeep() {
  tone(BUZZER, 800, 400);
}


// -----
// Alarm
// -----
void activateAlarm() {
  alarmActive = true;
  alarmTimer = millis();
  updateLCD();
}

void resetAlarm() {
  alarmActive = false;
  failedAttempts = 0;
  inputCode = "";
  noTone(BUZZER);
  digitalWrite(LED_RED, HIGH);
  updateLCD();
}


// -------------------
// LCD + Blynk updaten
// -------------------
void updateLCD() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Door:");

  if (doorIsLocked) lcd.print("LOCK ");
  else lcd.print("UNLOCK ");

  if (doorIsOpen) lcd.print("OPEN");
  else lcd.print("CLOSE");

  lcd.setCursor(0, 1);
  lcd.print("Alarm:");
  lcd.print(alarmActive ? "ON! " : "OFF ");

  lcd.print("Code:");
  lcd.print(failedAttempts);

  // -- Blynk updates --
  Blynk.virtualWrite(V0, doorIsLocked ? "LOCK" : "UNLOCK");
  Blynk.virtualWrite(V1, doorIsOpen ? "OPEN" : "CLOSE");
  Blynk.virtualWrite(V2, alarmActive ? 1 : 0);
  Blynk.virtualWrite(V3, failedAttempts);
}
