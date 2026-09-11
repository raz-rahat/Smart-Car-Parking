#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

// ================= PIN CONFIGURATION =================
#define RFID_SS_PIN 5
#define RFID_RST_PIN 4

#define SERVO_PIN 13
#define GATE_CLOSED_ANGLE 0
#define GATE_OPEN_ANGLE 90

#define TRIG1_PIN 27
#define ECHO1_PIN 26

#define TRIG2_PIN 25
#define ECHO2_PIN 33

#define IR_SLOT1_PIN 32
#define IR_SLOT2_PIN 35
#define IR_SLOT3_PIN 34

#define BUZZER_PIN 14
#define LED_GREEN_PIN 12
#define LED_RED_PIN 15
#define LED_YELLOW_PIN 2   

#define LCD_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// ================= WiFi Access Point =================
const char* AP_SSID = "SmartParking";
const char* AP_PASSWORD = "12345678";

// ================= REGISTERED CARDS =================
const String CARD1_UID = "F0B5F052";
const String CARD2_UID = "A199F70A";

const String USER1_NAME = "RAZ";
const String USER2_NAME = "RAHAT";

// ================= SETTINGS =================
const int TOTAL_SLOTS = 3;                    
const int CAR_DETECT_DISTANCE_CM = 15;        

// ================= TIMING CONFIGURATION =================
const unsigned long VERIFY_TIMEOUT_MS = 15000;     
const unsigned long GATE_OPEN_TIME_MS = 10000;     
const unsigned long PASSAGE_TIMEOUT_MS = 15000;    
const unsigned long LED_TIME_MS = 9000;            
const unsigned long RESULT_MESSAGE_MS = 2500;      
const unsigned long WELCOME_MESSAGE_MS = 3000;     
const unsigned long SLOT_LCD_UPDATE_INTERVAL = 3000; 

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Servo gateServo;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
WebServer server(80);

// ================= STATE MACHINE =================
enum SystemState {
  IDLE,             
  ENTRY_VERIFY,     
  ENTRY_WELCOME,    
  ENTRY_WAIT_PASS,  
  EXIT_VERIFY,      
  EXIT_GOODBYE,     
  EXIT_WAIT_PASS,   
  SHOW_RESULT       
};

SystemState systemState = IDLE;

bool slotOccupied[TOTAL_SLOTS] = {false, false, false};
int occupiedCount = 0;              
int availableSlots = TOTAL_SLOTS;   

int totalEntries = 0;      
int totalExits = 0;        
int totalVerifyFail = 0;   

bool gateIsOpen = false;
bool manualGateMode = false;    
bool ledActive = false;
bool readyScreenDrawn = false;  

unsigned long verifyStartedAt = 0;
unsigned long cardVerifiedAt = 0;
unsigned long gateOpenedAt = 0;
unsigned long resultStartedAt = 0;
unsigned long welcomeStartedAt = 0;
unsigned long ledStartedAt = 0;
unsigned long lastSlotLCDUpdate = 0;

String lastCardUID = "-";
String lastAccessResult = "-";
String systemMessage = "System Ready";
String gateStatusText = "CLOSED";
String lastReadyLine = "";

// Function Declarations
void showStartupSplash();
String centerText(String text);
void showIPAddress();
void checkSystemFlow();
void checkRFID();
int getUser(String uid);
String getUserName(int userId);
void handleGateTimer();
void openGate();
void closeGate();
bool isCarDetected(int trigPin, int echoPin);
long readDistanceCM(int trigPin, int echoPin);
void updateSlotStatus();
void beep(int times, int durationMs);
void showAccessLED(bool granted);
void showFullLED();
void handleLEDTimer();
void forceReadyLCD();
void updateReadyLCD();
void showVerifyLCD(String text);
void showWelcomeLCD(String line1, String line2);
void showResultLCD(String text);
void showFullLCD(String name);
void showInvalidLCD(String uid);
void handleRoot();
void handleStatus();
void handleGateOpen();
void handleGateClose();
void handleReset();

void setup() {
  Serial.begin(115200);

  pinMode(TRIG1_PIN, OUTPUT);
  pinMode(ECHO1_PIN, INPUT);
  pinMode(TRIG2_PIN, OUTPUT);
  pinMode(ECHO2_PIN, INPUT);

  pinMode(IR_SLOT1_PIN, INPUT);
  pinMode(IR_SLOT2_PIN, INPUT);
  pinMode(IR_SLOT3_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);

  SPI.begin();
  rfid.PCD_Init();

  gateServo.setPeriodHertz(50);
  gateServo.attach(SERVO_PIN, 500, 2400);
  gateServo.write(GATE_CLOSED_ANGLE);

  lcd.init();
  lcd.backlight();

  showStartupSplash();

  WiFi.softAP(AP_SSID, AP_PASSWORD);

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/gate/open", handleGateOpen);
  server.on("/gate/close", handleGateClose);
  server.on("/reset", handleReset);

  server.begin();

  updateSlotStatus();
  showIPAddress();

  systemMessage = "System Ready";
  forceReadyLCD();

  Serial.println("Smart Car Parking System Started");
  Serial.print("Dashboard: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();       
  updateSlotStatus();          
  checkSystemFlow();           
  checkRFID();                 
  handleGateTimer();           
  handleLEDTimer();            

  if (systemState == IDLE || systemState == ENTRY_WAIT_PASS || systemState == EXIT_WAIT_PASS) {
    updateReadyLCD();
  }
}

void showStartupSplash() {
  const unsigned long SPLASH_STAGE_MS = 2000;   
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(centerText("SMART CAR"));
  lcd.setCursor(0, 1);
  lcd.print(centerText("PARKING SYSTEM"));
  delay(SPLASH_STAGE_MS);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(centerText("BY MD RAZ"));
  lcd.setCursor(0, 1);
  lcd.print(centerText("2026"));
  delay(SPLASH_STAGE_MS);
  lcd.clear();
}

String centerText(String text) {
  if (text.length() >= LCD_COLS) return text.substring(0, LCD_COLS);
  int totalPadding = LCD_COLS - text.length();
  int leftPadding = totalPadding / 2;
  String line = "";
  for (int i = 0; i < leftPadding; i++) line += " ";
  line += text;
  while (line.length() < LCD_COLS) line += " ";
  return line;
}

void showIPAddress() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP Address:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.softAPIP());
  delay(3000);
  lcd.clear();
}

void checkSystemFlow() {
  bool carAtSensor1 = isCarDetected(TRIG1_PIN, ECHO1_PIN);   
  bool carAtSensor2 = isCarDetected(TRIG2_PIN, ECHO2_PIN);   
  unsigned long now = millis();

  switch (systemState) {
    case IDLE:
      if (manualGateMode) break;   
      if (carAtSensor1) {
        systemState = ENTRY_VERIFY;
        verifyStartedAt = now;
        systemMessage = "Entry Verify";
        lastAccessResult = "Waiting Card";
        readyScreenDrawn = false;
        showVerifyLCD("Entry Verify");
      } else if (carAtSensor2) {
        systemState = EXIT_VERIFY;
        verifyStartedAt = now;
        systemMessage = "Exit Verify";
        lastAccessResult = "Waiting Card";
        readyScreenDrawn = false;
        showVerifyLCD("Exit Verify");
      }
      break;

    case ENTRY_VERIFY:
      if (!carAtSensor1) {
        systemState = IDLE;
        systemMessage = "System Ready";
        lastAccessResult = "-";
        forceReadyLCD();
      } else if (now - verifyStartedAt >= VERIFY_TIMEOUT_MS) {
        systemState = IDLE;
        systemMessage = "System Ready";
        lastAccessResult = "No Card";
        forceReadyLCD();
      }
      break;

    case EXIT_VERIFY:
      if (!carAtSensor2) {
        systemState = IDLE;
        systemMessage = "System Ready";
        lastAccessResult = "-";
        forceReadyLCD();
      } else if (now - verifyStartedAt >= VERIFY_TIMEOUT_MS) {
        systemState = IDLE;
        systemMessage = "System Ready";
        lastAccessResult = "No Card";
        forceReadyLCD();
      }
      break;

    case ENTRY_WELCOME:
      if (now - welcomeStartedAt >= WELCOME_MESSAGE_MS) {
        systemState = ENTRY_WAIT_PASS;
        cardVerifiedAt = now;
        systemMessage = "Waiting Entry";
        lastAccessResult = "Pass Sensor 2";
      }
      break;

    case ENTRY_WAIT_PASS:
      // Servo 1 অতিক্রম করে Sensor 2 তে আসলে Entry Successful হবে
      if (carAtSensor2 && (now - cardVerifiedAt <= PASSAGE_TIMEOUT_MS)) {
        totalEntries++;
        systemMessage = "Entry Success";
        lastAccessResult = "Entry Success";
        beep(2, 100);   
        showResultLCD("Entry Success");
        closeGate();
        resultStartedAt = now;
        systemState = SHOW_RESULT;
      } else if (now - cardVerifiedAt > PASSAGE_TIMEOUT_MS) {
        systemMessage = "Entry Failed";
        lastAccessResult = "Pass Timeout";
        showResultLCD("Entry Failed");
        closeGate();
        resultStartedAt = now;
        systemState = SHOW_RESULT;
      }
      break;

    case EXIT_GOODBYE:
      if (now - welcomeStartedAt >= WELCOME_MESSAGE_MS) {
        systemState = EXIT_WAIT_PASS;
        cardVerifiedAt = now;
        systemMessage = "Waiting Exit";
        lastAccessResult = "Pass Sensor 1";
      }
      break;

    case EXIT_WAIT_PASS:
      // Sensor 2 পার হয়ে Sensor 1 অতিক্রম করলে Exit Successful হবে
      if (carAtSensor1 && (now - cardVerifiedAt <= PASSAGE_TIMEOUT_MS)) {
        totalExits++;
        systemMessage = "Exit Success";
        lastAccessResult = "Exit Success";
        beep(2, 100);
        showResultLCD("Exit Success");
        closeGate();
        resultStartedAt = now;
        systemState = SHOW_RESULT;
      } else if (now - cardVerifiedAt > PASSAGE_TIMEOUT_MS) {
        systemMessage = "Exit Failed";
        lastAccessResult = "Pass Timeout";
        showResultLCD("Exit Failed");
        closeGate();
        resultStartedAt = now;
        systemState = SHOW_RESULT;
      }
      break;

    case SHOW_RESULT:
      if (now - resultStartedAt >= RESULT_MESSAGE_MS) {
        systemState = IDLE;
        systemMessage = "System Ready";
        forceReadyLCD();
      }
      break;
  }
}

void checkRFID() {
  if (systemState != ENTRY_VERIFY && systemState != EXIT_VERIFY) return;
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  lastCardUID = uid;

  int user = getUser(uid);

  if (user == 0) {
    totalVerifyFail++;
    systemMessage = "Warning-" + uid;
    lastAccessResult = "Not Registered";
    showAccessLED(false);
    beep(3, 100);
    showInvalidLCD(uid);
    resultStartedAt = millis();
    systemState = SHOW_RESULT;
  } else if (systemState == ENTRY_VERIFY) {
    String uname = getUserName(user);
    if (availableSlots <= 0) {
      systemMessage = "Sorry " + uname + " No Space";
      lastAccessResult = "Parking Full";
      showFullLED();
      beep(2, 150);
      showFullLCD(uname);
      resultStartedAt = millis();
      systemState = SHOW_RESULT;
    } else {
      systemMessage = "Welcome " + uname;
      lastAccessResult = "Entry Verified";
      showAccessLED(true);
      beep(1, 150);
      showWelcomeLCD("Welcome " + uname, "Pass Sensor 2");
      openGate();
      welcomeStartedAt = millis();
      gateOpenedAt = millis();
      systemState = ENTRY_WELCOME;
    }
  } else if (systemState == EXIT_VERIFY) {
    String uname = getUserName(user);
    systemMessage = "Goodbye " + uname;
    lastAccessResult = "Exit Verified";
    showAccessLED(true);
    beep(1, 150);
    showWelcomeLCD("Goodbye " + uname, "Pass Sensor 1");
    openGate();
    welcomeStartedAt = millis();
    gateOpenedAt = millis();
    systemState = EXIT_GOODBYE;
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

int getUser(String uid) {
  if (uid == CARD1_UID) return 1;
  if (uid == CARD2_UID) return 2;
  return 0;
}

String getUserName(int userId) {
  if (userId == 1) return USER1_NAME;
  if (userId == 2) return USER2_NAME;
  return "Guest";
}

void handleGateTimer() {
  if (!gateIsOpen || manualGateMode) return;
  if (millis() - gateOpenedAt >= GATE_OPEN_TIME_MS) closeGate();
}

void openGate() {
  gateServo.write(GATE_OPEN_ANGLE);
  gateIsOpen = true;
  gateStatusText = "OPEN";
}

void closeGate() {
  gateServo.write(GATE_CLOSED_ANGLE);
  gateIsOpen = false;
  gateStatusText = "CLOSED";
}

bool isCarDetected(int trigPin, int echoPin) {
  long distance = readDistanceCM(trigPin, echoPin);
  return (distance > 0 && distance <= CAR_DETECT_DISTANCE_CM);
}

long readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2;
}

void updateSlotStatus() {
  slotOccupied[0] = digitalRead(IR_SLOT1_PIN) == LOW;
  slotOccupied[1] = digitalRead(IR_SLOT2_PIN) == LOW;
  slotOccupied[2] = digitalRead(IR_SLOT3_PIN) == LOW;

  occupiedCount = 0;
  for (int i = 0; i < TOTAL_SLOTS; i++) {
    if (slotOccupied[i]) occupiedCount++;
  }
  availableSlots = TOTAL_SLOTS - occupiedCount;
}

void beep(int times, int durationMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(durationMs);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < times - 1) delay(durationMs);
  }
}

void showAccessLED(bool granted) {
  ledActive = true;
  ledStartedAt = millis();
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, granted ? HIGH : LOW);
  digitalWrite(LED_RED_PIN, granted ? LOW : HIGH);
}

void showFullLED() {
  ledActive = true;
  ledStartedAt = millis();
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, HIGH);
}

void handleLEDTimer() {
  if (ledActive && millis() - ledStartedAt >= LED_TIME_MS) {
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_YELLOW_PIN, LOW);
    ledActive = false;
  }
}

void forceReadyLCD() {
  readyScreenDrawn = false;
  lastReadyLine = "";
  lastSlotLCDUpdate = 0;
  updateReadyLCD();
}

// ================= VF পরিবর্তন করে F করা হয়েছে =================
void updateReadyLCD() {
  if (systemState != IDLE && systemState != ENTRY_WAIT_PASS && systemState != EXIT_WAIT_PASS) return;
  unsigned long now = millis();
  if (!readyScreenDrawn || now - lastSlotLCDUpdate >= SLOT_LCD_UPDATE_INTERVAL) {
    String newReadyLine = "E:" + String(totalEntries) + " X:" + String(totalExits) + " F:" + String(totalVerifyFail);
    if (!readyScreenDrawn) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System Ready");
      lcd.setCursor(0, 1);
      lcd.print(newReadyLine);
      readyScreenDrawn = true;
    } else if (newReadyLine != lastReadyLine) {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(newReadyLine);
    }
    lastReadyLine = newReadyLine;
    lastSlotLCDUpdate = now;
  }
}

void showVerifyLCD(String text) {
  readyScreenDrawn = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text);
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card");
}

void showWelcomeLCD(String line1, String line2) {
  readyScreenDrawn = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, min((int)line1.length(), 16)));
  lcd.setCursor(0, 1);
  lcd.print(line2.substring(0, min((int)line2.length(), 16)));
}

void showResultLCD(String text) {
  readyScreenDrawn = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text.length() > 16 ? text.substring(0, 16) : text);
  lcd.setCursor(0, 1);
  lcd.print("Please Wait");
}

void showFullLCD(String name) {
  readyScreenDrawn = false;
  lcd.clear();
  String line1 = "Sorry " + name;
  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, min((int)line1.length(), 16)));
  lcd.setCursor(0, 1);
  lcd.print("No Space Left");
}

void showInvalidLCD(String uid) {
  readyScreenDrawn = false;
  lcd.clear();
  String line1 = "Warning-" + uid;
  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, min((int)line1.length(), 16)));
  lcd.setCursor(0, 1);
  lcd.print("Not Registered");
}

void handleGateOpen() {
  manualGateMode = true;
  openGate();
  systemMessage = "Manual Gate Open";
  lastAccessResult = "Manual Open";
  server.send(200, "text/plain", "Gate Opened");
}

void handleGateClose() {
  manualGateMode = false;
  closeGate();
  systemState = IDLE;
  systemMessage = "System Ready";
  lastAccessResult = "Manual Close";
  forceReadyLCD();
  server.send(200, "text/plain", "Gate Closed");
}

void handleReset() {
  totalEntries = 0;
  totalExits = 0;
  totalVerifyFail = 0;
  systemState = IDLE;
  systemMessage = "System Ready";
  lastAccessResult = "Counter Reset";
  forceReadyLCD();
  server.send(200, "text/plain", "Reset Success");
}

void handleStatus() {
  String json = "{";
  json += "\"gate\":\"" + gateStatusText + "\",";
  json += "\"available\":" + String(availableSlots) + ",";
  json += "\"entries\":" + String(totalEntries) + ",";
  json += "\"exits\":" + String(totalExits) + ",";
  json += "\"verifyFail\":" + String(totalVerifyFail) + ",";
  json += "\"uid\":\"" + lastCardUID + "\",";
  json += "\"access\":\"" + lastAccessResult + "\",";
  json += "\"message\":\"" + systemMessage + "\",";
  json += "\"slots\":[";

  for (int i = 0; i < TOTAL_SLOTS; i++) {
    json += slotOccupied[i] ? "true" : "false";
    if (i < TOTAL_SLOTS - 1) json += ",";
  }
  json += "]}";

  server.send(200, "application/json", json);
}

// ================= DYNAMIC MODERN DASHBOARD =================
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="bn">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Parking — Dashboard</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@500;600;700&family=IBM+Plex+Sans:wght@400;500;600&family=IBM+Plex+Mono:wght@400;500;600&family=VT323&display=swap" rel="stylesheet">
<style>
  :root{
    --bg:#0d1013;
    --panel:#161b21;
    --border:#262e38;
    --amber:#f2a93b;
    --amber-dim:#7a5a24;
    --teal:#4fd1c5;
    --teal-dim:#1f4a46;
    --red:#ef5b5b;
    --red-dim:#5a2323;
    --text:#e7e9ec;
    --muted:#8b95a1;
    --lcd-bg:#0a1a0f;
    --lcd-fg:#6dff9a;
  }
  *{box-sizing:border-box;}
  body{
    margin:0;
    background:
      radial-gradient(circle at 20% 0%, #14191f 0%, var(--bg) 45%),
      repeating-linear-gradient(0deg, transparent, transparent 39px, #ffffff05 39px, #ffffff05 40px),
      repeating-linear-gradient(90deg, transparent, transparent 39px, #ffffff05 39px, #ffffff05 40px);
    color:var(--text);
    font-family:'IBM Plex Sans', sans-serif;
    min-height:100vh;
    padding:28px 16px 60px;
  }
  .wrap{max-width:760px;margin:0 auto;}

  header{display:flex;justify-content:space-between;align-items:flex-start;margin-bottom:18px;flex-wrap:wrap;gap:10px;}
  .brand{display:flex;align-items:center;gap:12px;}
  .brand-mark{
    width:42px;height:42px;border-radius:10px;
    background:linear-gradient(145deg,var(--amber),#c97f1f);
    display:flex;align-items:center;justify-content:center;
    font-size:20px; box-shadow:0 0 0 1px #ffffff22 inset, 0 6px 14px -6px #f2a93b66;
  }
  h1{font-family:'Space Grotesk',sans-serif;font-size:20px;margin:0;letter-spacing:0.2px;}
  .sub{color:var(--muted);font-size:12.5px;margin-top:2px;}
  .badge{
    display:flex;align-items:center;gap:8px;
    background:var(--panel);border:1px solid var(--border);
    padding:8px 12px;border-radius:999px;font-size:12.5px;color:var(--muted);
    font-family:'IBM Plex Mono',monospace;
  }
  .dot{width:7px;height:7px;border-radius:50%;background:var(--teal);box-shadow:0 0 8px var(--teal);animation:pulse 1.6s infinite;}
  .dot.offline{background:var(--red);box-shadow:0 0 8px var(--red);}
  @keyframes pulse{0%,100%{opacity:1;}50%{opacity:.35;}}

  .lcd-shell{
    background:linear-gradient(180deg,#232a32,#181d23);
    border:1px solid var(--border);
    border-radius:14px;
    padding:14px 16px;
    margin-bottom:18px;
    display:flex; align-items:center; gap:14px;
    box-shadow: 0 8px 24px -14px #000a;
  }
  .lcd-label{font-family:'IBM Plex Mono',monospace;font-size:10px;color:var(--muted);writing-mode:vertical-rl;text-orientation:mixed;letter-spacing:1px;}
  .lcd-screen{
    flex:1;
    background:var(--lcd-bg);
    border-radius:6px;
    padding:10px 14px;
    overflow:hidden;
    box-shadow: inset 0 0 12px #000, inset 0 0 0 1px #ffffff0d;
  }
  .lcd-line{
    font-family:'VT323', monospace;
    font-size:22px;
    line-height:1.25;
    color:var(--lcd-fg);
    text-shadow:0 0 6px #6dff9a99;
    white-space:nowrap;
    overflow:hidden;
    letter-spacing:1px;
  }

  .gate-card{
    background:var(--panel);border:1px solid var(--border);border-radius:16px;
    padding:20px; margin-bottom:16px; text-align:center; position:relative; overflow:hidden;
  }
  .gate-status-label{font-family:'IBM Plex Mono',monospace;font-size:12px;color:var(--muted);text-transform:uppercase;letter-spacing:1.5px;}
  .gate-status{font-family:'Space Grotesk',sans-serif;font-size:30px;font-weight:700;margin:2px 0 18px;transition:color .3s;}
  .gate-status.open{color:var(--teal);}
  .gate-status.closed{color:var(--amber);}
  .gate-scene{height:90px;display:flex;align-items:flex-end;justify-content:center;position:relative;}
  .post{width:10px;height:70px;background:linear-gradient(180deg,#3a4552,#232a32);border-radius:3px;position:relative;}
  .arm{
    position:absolute; left:10px; bottom:64px; width:130px; height:9px;
    background:repeating-linear-gradient(90deg,var(--amber) 0 16px, #1b1f24 16px 24px);
    border-radius:3px; transform-origin:left center;
    transform:rotate(0deg);
    transition:transform .7s cubic-bezier(.4,1.6,.5,1);
    box-shadow:0 2px 6px #0007;
  }
  .arm.open{transform:rotate(-78deg);}
  .car{
    position:absolute; bottom:2px; font-size:26px;
    transition: left 1.4s linear, opacity .3s;
    left:-40px; opacity:0;
  }
  .car.driving{opacity:1;}

  .leds{display:flex;gap:10px;justify-content:center;margin-top:14px;}
  .led{width:14px;height:14px;border-radius:50%;background:#2a2f36;border:1px solid #000;transition:.3s;}
  .led.green.on{background:#3ee06f;box-shadow:0 0 12px 3px #3ee06f99;}
  .led.red.on{background:var(--red);box-shadow:0 0 12px 3px #ef5b5b99;}
  .led.yellow.on{background:var(--amber);box-shadow:0 0 12px 3px #f2a93b99;}

  .stats{display:grid;grid-template-columns:repeat(4,1fr);gap:10px;margin-bottom:16px;}
  .stat{background:var(--panel);border:1px solid var(--border);border-radius:14px;padding:14px 8px;text-align:center;}
  .stat .num{font-family:'IBM Plex Mono',monospace;font-size:22px;font-weight:600;}
  .stat .lbl{color:var(--muted);font-size:10.5px;margin-top:4px;text-transform:uppercase;letter-spacing:1px;}
  .stat.avail .num{color:var(--teal);}
  .stat.fail .num{color:var(--red);}

  .section-title{font-family:'Space Grotesk',sans-serif;font-size:14px;color:var(--muted);margin:22px 2px 10px;text-transform:uppercase;letter-spacing:1.5px;}
  .slots{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;}
  .slot{
    background:var(--panel);border:1px solid var(--border);border-radius:12px;
    padding:14px 8px; text-align:center; transition:background .4s, border-color .4s, transform .3s;
  }
  .slot .icon{font-size:22px;display:block;margin-bottom:6px;}
  .slot .name{font-family:'IBM Plex Mono',monospace;font-size:11px;color:var(--muted);}
  .slot .state{font-size:11.5px;font-weight:600;margin-top:3px;}
  .slot.free{border-color:var(--teal-dim);}
  .slot.free .state{color:var(--teal);}
  .slot.occupied{border-color:var(--red-dim); background:linear-gradient(180deg,#1c1416,var(--panel));}
  .slot.occupied .state{color:var(--red);}
  .slot.flash{transform:scale(1.05);}

  .controls{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-top:18px;}
  button{
    border:none;padding:13px 8px;border-radius:10px;font-weight:600;color:white;cursor:pointer;
    font-family:'IBM Plex Sans',sans-serif;font-size:13px;transition:filter .2s, transform .1s;
  }
  button:active{transform:scale(0.97);}
  .btn-open{background:#16a34a;}
  .btn-close{background:#dc2626;}
  .btn-reset{background:#2563eb;}

  .event-card{
    background:var(--panel);border:1px solid var(--border);border-radius:14px;
    padding:16px; margin-top:20px;
  }
  .event-row{display:flex;justify-content:space-between;padding:7px 0;border-bottom:1px dashed var(--border);font-size:13px;gap:10px;}
  .event-row:last-child{border-bottom:none;}
  .event-row span:first-child{color:var(--muted);flex-shrink:0;}
  .event-row span:last-child{font-family:'IBM Plex Mono',monospace;text-align:right;word-break:break-word;}
  .access-granted{color:var(--teal);}
  .access-denied{color:var(--red);}
  .access-full{color:var(--amber);}

  footer{text-align:center;color:var(--muted);font-size:11.5px;margin-top:30px;line-height:1.6;}
  footer b{color:#b7c0cc;}

  @media (max-width:480px){
    .stats{grid-template-columns:repeat(4,1fr);}
    .stat .num{font-size:18px;}
  }
</style>
</head>
<body>
<div class="wrap">

  <header>
    <div class="brand">
      <div class="brand-mark">🅿️</div>
      <div>
        <h1>Smart Parking Dashboard</h1>
        <div class="sub">ESP32 · লাইভ কন্ট্রোল বোর্ড</div>
      </div>
    </div>
    <div class="badge"><span class="dot" id="connDot"></span> <span id="connText">LIVE CONNECTED</span></div>
  </header>

  <div class="lcd-shell">
    <div class="lcd-label">16×2 LCD</div>
    <div class="lcd-screen">
      <div class="lcd-line" id="lcd1">System Ready</div>
      <div class="lcd-line" id="lcd2">Initializing...</div>
    </div>
  </div>

  <div class="gate-card">
    <div class="gate-status-label">Gate Status</div>
    <div class="gate-status closed" id="gateStatus">CLOSED</div>
    <div class="gate-scene">
      <div class="post"></div>
      <div class="arm" id="arm"></div>
      <div class="car" id="car">🚗</div>
    </div>
    <div class="leds">
      <div class="led green" id="ledGreen"></div>
      <div class="led yellow" id="ledYellow"></div>
      <div class="led red" id="ledRed"></div>
    </div>
  </div>

  <div class="stats">
    <div class="stat avail"><div class="num" id="statAvail">-</div><div class="lbl">Free</div></div>
    <div class="stat"><div class="num" id="statEntries">0</div><div class="lbl">Entries</div></div>
    <div class="stat"><div class="num" id="statExits">0</div><div class="lbl">Exits</div></div>
    <div class="stat fail"><div class="num" id="statFail">0</div><div class="lbl">Fail</div></div>
  </div>

  <div class="section-title">Parking Slots</div>
  <div class="slots" id="slotsGrid"></div>

  <div class="controls">
    <button class="btn-open" onclick="fetch('/gate/open')">Open Gate</button>
    <button class="btn-close" onclick="fetch('/gate/close')">Close Gate</button>
    <button class="btn-reset" onclick="fetch('/reset')">Reset Count</button>
  </div>

  <div class="event-card">
    <div class="event-row"><span>Last Card UID</span><span id="evUid">-</span></div>
    <div class="event-row"><span>Access Result</span><span id="evAccess">-</span></div>
    <div class="event-row"><span>Last Event</span><span id="evMsg">System Ready</span></div>
  </div>

  <footer>
    <b>Smart Car Parking System</b> — Akash Ahmed Raj &amp; his team<br>
    ESP32 নিজস্ব WiFi Access Point থেকে লাইভ সেন্সর ডেটা সরাসরি এই পেজে দেখানো হচ্ছে।
  </footer>
</div>

<script>
const slotNames = ["Slot 1","Slot 2","Slot 3"];
let prevSlots = [false, false, false];
let prevGateOpen = null;
let prevUid = "-";

function renderSlots(slots, flashIndex){
  const grid = document.getElementById('slotsGrid');
  grid.innerHTML = "";
  slots.forEach((occ, i) => {
    const el = document.createElement('div');
    el.className = 'slot ' + (occ ? 'occupied' : 'free') + (i===flashIndex ? ' flash' : '');
    el.innerHTML = `
      <span class="icon">${occ ? '🚙' : '⬜'}</span>
      <div class="name">${slotNames[i]}</div>
      <div class="state">${occ ? 'Occupied' : 'Free'}</div>
    `;
    grid.appendChild(el);
  });
}

function setGate(open){
  const arm = document.getElementById('arm');
  arm.classList.toggle('open', open);
  const label = document.getElementById('gateStatus');
  label.textContent = open ? 'OPEN' : 'CLOSED';
  label.classList.toggle('open', open);
  label.classList.toggle('closed', !open);
}

function driveCar(direction){
  const car = document.getElementById('car');
  car.style.transition = 'none';
  car.style.left = direction === 'in' ? '-40px' : '260px';
  car.style.opacity = '0';
  void car.offsetWidth;
  car.style.transition = 'left 1.4s linear, opacity .3s';
  car.classList.add('driving');
  requestAnimationFrame(()=>{
    car.style.left = direction === 'in' ? '260px' : '-40px';
  });
  setTimeout(()=>{
    car.classList.remove('driving');
    car.style.opacity = '0';
  }, 1450);
}

function setLeds(access){
  const g = document.getElementById('ledGreen');
  const y = document.getElementById('ledYellow');
  const r = document.getElementById('ledRed');
  g.classList.remove('on'); y.classList.remove('on'); r.classList.remove('on');
  const a = (access || "").toLowerCase();
  if(a.includes('full')) { y.classList.add('on'); }
  else if(a.includes('verified') || a.includes('success')) { g.classList.add('on'); }
  else if(a.includes('not registered') || a.includes('timeout')) { r.classList.add('on'); }
}

function accessClass(access){
  const a = (access || "").toLowerCase();
  if(a.includes('full')) return 'access-full';
  if(a.includes('verified') || a.includes('success')) return 'access-granted';
  if(a.includes('not registered') || a.includes('timeout')) return 'access-denied';
  return '';
}

async function updateDashboard(){
  try {
    const res = await fetch('/status');
    const data = await res.json();

    document.getElementById('connDot').classList.remove('offline');
    document.getElementById('connText').textContent = 'LIVE CONNECTED';

    const isOpen = data.gate === 'OPEN';
    setGate(isOpen);
    if(prevGateOpen !== null && isOpen !== prevGateOpen){
      driveCar(isOpen ? 'in' : 'out');
    }
    prevGateOpen = isOpen;

    document.getElementById('statAvail').textContent = data.available;
    document.getElementById('statEntries').textContent = data.entries;
    document.getElementById('statExits').textContent = data.exits;
    document.getElementById('statFail').textContent = data.verifyFail;

    document.getElementById('lcd1').textContent = data.message;
    document.getElementById('lcd2').textContent = "E:" + data.entries + " X:" + data.exits + " F:" + data.verifyFail;

    document.getElementById('evUid').textContent = data.uid;
    const evAccess = document.getElementById('evAccess');
    evAccess.textContent = data.access;
    evAccess.className = accessClass(data.access);
    document.getElementById('evMsg').textContent = data.message;

    if(data.uid !== prevUid){
      setLeds(data.access);
      prevUid = data.uid;
    }

    let flashIndex = null;
    for(let i=0;i<data.slots.length;i++){
      if(data.slots[i] !== prevSlots[i]) flashIndex = i;
    }
    renderSlots(data.slots, flashIndex);
    prevSlots = data.slots.slice();

  } catch(e) {
    document.getElementById('connDot').classList.add('offline');
    document.getElementById('connText').textContent = 'CONNECTION LOST';
    console.error("Connection Error", e);
  }
}

setInterval(updateDashboard, 500);
updateDashboard();
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}