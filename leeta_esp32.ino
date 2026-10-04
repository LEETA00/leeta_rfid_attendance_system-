// Leeta Attendance: ESP32 + MFRC522 + 16x2 I2C LCD + buzzer
// Libraries: MFRC522 (miguelbalboa), LiquidCrystal_I2C
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASSWORD";
const char* SERVER    = "http://192.168.1.50:5000";  // PC running server/app.py
const char* API_KEY   = "change-me";                 // must match server

#define SS_PIN 5
#define RST_PIN 4   // not 22: that is the I2C SCL pin
#define BUZZER 15

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // try 0x3F if blank
unsigned long lastBeat = 0;

void show(const String& a, const String& b = "") {
  lcd.clear(); lcd.setCursor(0, 0); lcd.print(a);
  lcd.setCursor(0, 1); lcd.print(b);
}
void beep(int ms) { digitalWrite(BUZZER, HIGH); delay(ms); digitalWrite(BUZZER, LOW); }

int post(const String& path, const String& body, String* out = nullptr) {
  HTTPClient http;
  http.begin(String(SERVER) + path);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", API_KEY);
  int code = http.POST(body);
  if (out && code > 0) *out = http.getString();
  http.end();
  return code;
}

void setup() {
  pinMode(BUZZER, OUTPUT);
  lcd.init(); lcd.backlight();
  SPI.begin(); rfid.PCD_Init();
  show("Leeta", "Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(300);
  show("Ready", "Scan your card");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
  if (millis() - lastBeat > 30000) { post("/api/heartbeat", "{}"); lastBeat = millis(); }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();

  String res;
  int code = post("/api/scan", "{\"uid\":\"" + uid + "\"}", &res);
  if (code == 200) {
    bool known = res.indexOf("\"known\":true") >= 0;
    if (known) { show("Welcome!", "Attendance saved"); beep(100); }
    else       { show("Unknown card", uid);            beep(600); }
  } else {
    show("Server error", String(code)); beep(600);
  }
  delay(2000);
  show("Ready", "Scan your card");
}