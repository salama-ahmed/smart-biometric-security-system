#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

// ---------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Fingerprint Sensor ----------
SoftwareSerial mySerial(2, 3);   // Rx, Tx
Adafruit_Fingerprint finger(&mySerial);

// ---------- Pins ----------
int relayPin = 8;     // Relay (Active LOW)
int buzzer = 7;       // Buzzer (Passive)
int ledGreen = 5;
int ledRed = 6;

// ---------------------
//  NAME FUNCTION
// ---------------------
String getName(int id) {
  if (id == 1) return "Aya";
  if (id == 2) return "Aml";
  if (id == 3) return "Salama";
  if (id == 4) return "Alaa";
  if (id == 5) return "Kiro";
  return "Unknown";
}

// ---------------------
//  SETUP
// ---------------------
void setup() {

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Starting...");
  delay(1500);

  Serial.begin(9600);
  mySerial.begin(57600);

  pinMode(relayPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);

  // default OFF
  digitalWrite(relayPin, HIGH);   // Relay Active LOW → HIGH = OFF
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, LOW);
  digitalWrite(buzzer, LOW);

  Serial.println("Connecting to fingerprint sensor...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  if (!finger.verifyPassword()) {
    Serial.println("Sensor Error!");

    lcd.clear();
    lcd.print("Sensor Error!");

    while (1);
  }

  Serial.println("Ready!");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fingerprint");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");

  delay(1500);

  Serial.println("Press 'e' to Enroll");
}

// ---------------------
//  LOOP
// ---------------------
void loop() {

  // Check if user wants to enroll new finger
  if (Serial.available()) {
    char c = Serial.read();

    if (c == 'e') {
      enrollFingerprint();
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger...");

  delay(300);

  int id = getFingerprintID();

  if (id > 0) {
    grantAccess(id);
  }
  else if (id == -2) {
    denyAccess();
  }
}

// ---------------------
//  ACCESS GRANTED
// ---------------------
void grantAccess(int id) {

  Serial.print("Welcome: ");
  Serial.println(getName(id));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome:");
  lcd.setCursor(0, 1);
  lcd.print(getName(id));

  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);

  digitalWrite(relayPin, LOW);  // Relay ON

  tone(buzzer, 2000, 200);

  delay(4000);   // Door open time

  digitalWrite(relayPin, HIGH); // Relay OFF
  digitalWrite(ledGreen, LOW);
}

// ---------------------
//  ACCESS DENIED
// ---------------------
void denyAccess() {

  Serial.println("Access Denied!");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access");
  lcd.setCursor(0, 1);
  lcd.print("Denied!");

  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, HIGH);

  tone(buzzer, 500, 600);

  delay(1500);

  digitalWrite(ledRed, LOW);
}

// ---------------------
//  FINGERPRINT SEARCH
// ---------------------
int getFingerprintID() {

  uint8_t p = finger.getImage();

  if (p == FINGERPRINT_NOFINGER)
    return -1;

  if (p != FINGERPRINT_OK)
    return -2;

  p = finger.image2Tz();

  if (p != FINGERPRINT_OK)
    return -2;

  p = finger.fingerFastSearch();

  if (p != FINGERPRINT_OK)
    return -2;

  return finger.fingerID;
}

// ---------------------
// ENROLL NEW FINGERPRINT
// ---------------------
void enrollFingerprint() {

  int id;

  Serial.println("Enter ID (1-127):");

  while (!Serial.available());

  id = Serial.parseInt();

  if (id <= 0) {
    Serial.println("Invalid ID");
    return;
  }

  Serial.print("Enrolling ID: ");
  Serial.println(id);

  lcd.clear();
  lcd.print("Enrolling ID:");
  lcd.setCursor(0, 1);
  lcd.print(id);

  delay(1500);

  while (finger.getImage() != FINGERPRINT_OK) {

    lcd.clear();
    lcd.print("Place Finger");

    delay(500);
  }

  finger.image2Tz(1);

  lcd.clear();
  lcd.print("Remove Finger");

  delay(1500);

  while (finger.getImage() != FINGERPRINT_OK) {

    lcd.clear();
    lcd.print("Again Finger");

    delay(500);
  }

  finger.image2Tz(2);

  finger.createModel();
  finger.storeModel(id);

  Serial.println("Saved!");

  lcd.clear();
  lcd.print("Saved!");

  delay(1500);
}
