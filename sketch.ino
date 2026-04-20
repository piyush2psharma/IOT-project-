/*
 * =====================================================================
 *  SMART DOOR CONTROL SYSTEM
 *  B.Tech / M.Tech Level IoT Project
 * ---------------------------------------------------------------------
 *  Hardware  : ESP32 Dev Board
 *  Sensors   : RFID RC522, PIR (HC-SR501), Servo SG90
 *              I2C LCD 16x2, RGB LEDs, Active Buzzer
 *  Cloud     : ThingSpeak (4 Fields)
 *              Field 1 → Door Status  (1=Open, 0=Closed)
 *              Field 2 → Auth Result  (1=Granted, 0=Denied)
 *              Field 3 → Motion Alert (1=Detected, 0=Clear)
 *              Field 4 → Total Access Attempts
 * ---------------------------------------------------------------------
 *  Author    : Smart IoT Lab
 *  Date      : 2026
 * =====================================================================
 */

#include <ESP32Servo.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>

/* ─── WiFi Credentials ──────────────────────────────────────────────── */
const char *WIFI_SSID = "Wokwi-GUEST"; // Wokwi virtual WiFi
const char *WIFI_PASSWORD = "";        // Open network on Wokwi

/* ─── ThingSpeak Configuration ──────────────────────────────────────── */
const char *TS_SERVER = "http://api.thingspeak.com";
const char *TS_API_KEY = "GJTXKOL4MA0C0V7T"; // ← replace
const unsigned long TS_INTERVAL = 15000;     // 15s between uploads
unsigned long tsLastUpload = 0;

/* ─── Pin Definitions ───────────────────────────────────────────────── */
// ── RFID SPI pins (fixed by ESP32 SPI bus) ──
#define SS_PIN   5    // RFID SDA/SS  → GPIO 5
#define RST_PIN  4    // RFID RST     → GPIO 4  (was 22, conflicted with LCD SCL)

// ── Free GPIO pins (no SPI/I2C conflicts) ──
#define SERVO_PIN  13  // Servo signal → GPIO 13 (was 18 = SPI SCK!)
#define PIR_PIN    33  // PIR OUT      → GPIO 33 (was 19 = SPI MISO!)
#define BUZZER_PIN 32  // Active buzzer→ GPIO 32 (was 23 = SPI MOSI!)

// ── LED pins ─────────────────────────────────
#define LED_GREEN 26   // Green LED  (Access Granted)
#define LED_RED   27   // Red LED    (Access Denied / Alarm)
#define LED_BLUE  25   // Blue LED   (WiFi / ThingSpeak activity)

/* ─── Hardware Objects ──────────────────────────────────────────────── */
MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27

/* ─── Authorised RFID UIDs ──────────────────────────────────────────── */
// Add your card UIDs here (hex bytes)
byte authorisedUIDs[][4] = {
    {0xDE, 0xAD, 0xBE, 0xEF}, // Card 1 – Master
    {0xAA, 0xBB, 0xCC, 0xDD}, // Card 2 – User A
    {0x11, 0x22, 0x33, 0x44}  // Card 3 – User B
};
const uint8_t NUM_CARDS = sizeof(authorisedUIDs) / 4;

/* ─── System State ──────────────────────────────────────────────────── */
bool doorOpen = false;
bool motionDetected = false;
bool wifiConnected = false;
int totalAttempts = 0;
int lastAuthResult = 0; // 1=granted, 0=denied
unsigned long doorTimer = 0;
const unsigned long DOOR_TIMEOUT = 5000; // Auto-lock after 5 s

/* ─── Servo angles ──────────────────────────────────────────────────── */
#define DOOR_LOCKED 0
#define DOOR_UNLOCKED 90

/* ===================================================================== */
void setup() {
  Serial.begin(115200);
  Serial.println("\n========================================");
  Serial.println("   SMART DOOR CONTROL SYSTEM v1.0");
  Serial.println("========================================\n");

  /* GPIO */
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  allLedsOff();

  /* Servo – start locked */
  doorServo.attach(SERVO_PIN);
  doorServo.write(DOOR_LOCKED);
  delay(500);

  /* I2C LCD */
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcdSplash();

  /* SPI + RFID */
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("[RFID] RC522 initialised");

  /* WiFi */
  connectWiFi();
}

/* ===================================================================== */
void loop() {
  /* --- PIR Motion Detection ----------------------------------------- */
  handleMotion();

  /* --- RFID Scan ----------------------------------------------------- */
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    handleRFID();
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  /* --- Auto-lock Door ------------------------------------------------ */
  if (doorOpen && (millis() - doorTimer >= DOOR_TIMEOUT)) {
    lockDoor();
  }

  /* --- ThingSpeak Upload --------------------------------------------- */
  if (millis() - tsLastUpload >= TS_INTERVAL) {
    uploadToThingSpeak();
    tsLastUpload = millis();
  }
}

/* ===================================================================== */
/*                         CORE FUNCTIONS                                */
/* ===================================================================== */

/* --- RFID Handler ---------------------------------------------------- */
void handleRFID() {
  totalAttempts++;

  /* Build UID string */
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1)
      uid += ":";
  }
  uid.toUpperCase();
  Serial.println("[RFID] Card detected – UID: " + uid);

  if (isAuthorised(rfid.uid.uidByte, rfid.uid.size)) {
    lastAuthResult = 1;
    grantAccess(uid);
  } else {
    lastAuthResult = 0;
    denyAccess(uid);
  }

  /* Immediate cloud push on every card event */
  uploadToThingSpeak();
  tsLastUpload = millis();
}

/* --- Check UID against authorised list ------------------------------- */
bool isAuthorised(byte *uid, byte size) {
  for (uint8_t i = 0; i < NUM_CARDS; i++) {
    bool match = true;
    for (byte j = 0; j < 4; j++) {
      if (uid[j] != authorisedUIDs[i][j]) {
        match = false;
        break;
      }
    }
    if (match)
      return true;
  }
  return false;
}

/* --- Access Granted -------------------------------------------------- */
void grantAccess(String uid) {
  Serial.println("[DOOR] ✅ Access GRANTED – " + uid);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  ACCESS GRANTED");
  lcd.setCursor(0, 1);
  lcd.print("  Door Opening..");

  digitalWrite(LED_GREEN, HIGH);
  beep(100);
  delay(100);
  beep(100); // two short beeps

  openDoor();
  delay(2000);

  digitalWrite(LED_GREEN, LOW);
  lcdDoorStatus();
}

/* --- Access Denied --------------------------------------------------- */
void denyAccess(String uid) {
  Serial.println("[DOOR] ❌ Access DENIED – " + uid);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  ACCESS DENIED ");
  lcd.setCursor(0, 1);
  lcd.print("  Unknown Card! ");

  digitalWrite(LED_RED, HIGH);
  /* Three warning beeps */
  for (int i = 0; i < 3; i++) {
    beep(200);
    delay(200);
  }
  delay(1500);

  digitalWrite(LED_RED, LOW);
  lcdDoorStatus();
}

/* --- Open Door ------------------------------------------------------- */
void openDoor() {
  doorServo.write(DOOR_UNLOCKED);
  doorOpen = true;
  doorTimer = millis();
  Serial.println("[DOOR] Door UNLOCKED (auto-lock in 5 s)");
}

/* --- Lock Door ------------------------------------------------------- */
void lockDoor() {
  doorServo.write(DOOR_LOCKED);
  doorOpen = false;
  Serial.println("[DOOR] Door LOCKED");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  DOOR  LOCKED  ");
  lcd.setCursor(0, 1);
  lcd.print(" Scan Card 2 Open");
  delay(1000);
  lcdDoorStatus();
}

/* --- PIR Motion Handler --------------------------------------------- */
void handleMotion() {
  bool pir = digitalRead(PIR_PIN);

  if (pir && !motionDetected) {
    motionDetected = true;
    Serial.println("[PIR] ⚠ Motion DETECTED near door!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" MOTION DETECTED");
    lcd.setCursor(0, 1);
    lcd.print("  Scan Your Card");

    /* Fast red blink */
    for (int i = 0; i < 4; i++) {
      digitalWrite(LED_RED, HIGH);
      delay(150);
      digitalWrite(LED_RED, LOW);
      delay(150);
    }
    lcdDoorStatus();

  } else if (!pir && motionDetected) {
    motionDetected = false;
    Serial.println("[PIR] Motion cleared");
  }
}

/* ===================================================================== */
/*                         WIFI & THINGSPEAK                             */
/* ===================================================================== */

/* --- Connect WiFi ---------------------------------------------------- */
void connectWiFi() {
  Serial.print("[WiFi] Connecting to " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Connecting to ");
  lcd.setCursor(0, 1);
  lcd.print("     WiFi...    ");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
    digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    digitalWrite(LED_BLUE, HIGH);
    Serial.println("\n[WiFi] ✅ Connected! IP: " + WiFi.localIP().toString());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(2000);
  } else {
    wifiConnected = false;
    digitalWrite(LED_BLUE, LOW);
    Serial.println("\n[WiFi] ❌ Connection failed – offline mode");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" WiFi Failed!   ");
    lcd.setCursor(0, 1);
    lcd.print(" Offline Mode   ");
    delay(2000);
  }
  lcdDoorStatus();
}

/* --- Upload to ThingSpeak -------------------------------------------- */
void uploadToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[TS] Skipped – no WiFi");
    return;
  }

  int doorStatus = doorOpen ? 1 : 0;
  int motionAlert = motionDetected ? 1 : 0;

  String url = String(TS_SERVER) + "/update?api_key=" + TS_API_KEY +
               "&field1=" + doorStatus + "&field2=" + lastAuthResult +
               "&field3=" + motionAlert + "&field4=" + totalAttempts;

  Serial.println("[TS] Uploading → " + url);

  /* Blue LED on during upload */
  digitalWrite(LED_BLUE, HIGH);

  HTTPClient http;
  http.begin(url);
  int code = http.GET();

  if (code > 0) {
    Serial.println("[TS] ✅ HTTP " + String(code) +
                   " | Response: " + http.getString());
  } else {
    Serial.println("[TS] ❌ Error: " + http.errorToString(code));
  }
  http.end();

  digitalWrite(LED_BLUE, LOW);
}

/* ===================================================================== */
/*                         UI HELPERS                                    */
/* ===================================================================== */

void lcdSplash() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SMART DOOR SYS ");
  lcd.setCursor(0, 1);
  lcd.print("  Initialising..");
  delay(1500);
}

void lcdDoorStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (doorOpen)
    lcd.print("  DOOR :  OPEN  ");
  else
    lcd.print("  DOOR : LOCKED ");
  lcd.setCursor(0, 1);
  if (wifiConnected)
    lcd.print(" Cloud: Online  ");
  else
    lcd.print(" Cloud: Offline ");
}

void beep(int ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void allLedsOff() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, LOW);
}
