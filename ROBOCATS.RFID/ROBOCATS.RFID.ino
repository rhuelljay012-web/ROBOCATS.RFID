#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

// Initialize LCD (modern syntax for 16x2 I2C LCD at address 0x3F)
LiquidCrystal_I2C lcd(0x3F, 16, 2);

#define SS_PIN 10
#define RST_PIN 9
#define RELAY 3      // Relay pin (HIGH = locked, LOW = unlocked)
#define BUZZER 2     // Buzzer pin
#define LED_G 4      // Green LED pin (granted access)
#define LED_R 5      // Red LED pin (denied access)
#define ACCESS_DELAY 2000  // Delay for unlocked state (ms)
#define DENIED_DELAY 1000  // Delay for denied feedback (ms)

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  Serial.begin(9600);   // Initiate serial communication
  SPI.begin();          // Initiate SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  noTone(BUZZER);       // Ensure buzzer is silent initially
  digitalWrite(RELAY, HIGH);  // Lock relay initially (assuming HIGH = locked)
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_R, LOW);
  lcd.init();           // Initialize LCD
  lcd.backlight();      // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Put card on rdr");
  Serial.println("Put your card to the reader...");
  Serial.println();
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  // Show UID on serial monitor
  Serial.print("UID tag: ");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message: ");
  content.toUpperCase();

  // Check for authorized UIDs (cleaned up for accuracy)
  if (content.substring(1) == "79 1F 63 12" || content.substring(1) == "D9 F2 90 11" || content.substring(1) == "47 65 9A 9E") {
    Serial.println("Authorized access");
    Serial.println();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Granted");
    scrollLCD("Welcome!", 1);  // Added scrolling welcome message
    digitalWrite(RELAY, LOW);    // Unlock relay
    digitalWrite(LED_G, HIGH);   // Turn on green LED
    tone(BUZZER, 1000, 200);     // 1 beep on open (1000Hz, 200ms)
    delay(ACCESS_DELAY);          // Keep unlocked
    digitalWrite(RELAY, HIGH);    // Lock relay
    digitalWrite(LED_G, LOW);     // Turn off green LED
    tone(BUZZER, 1000, 200);     // 2 beeps on close (1000Hz, 200ms each)
    delay(200);
    tone(BUZZER, 1000, 200);
  } else {
    Serial.println("Access denied");
    Serial.println();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied");
    scrollLCD("Try again later", 1);  // Added scrolling denial message
    digitalWrite(LED_R, HIGH);    // Turn on red LED
    for (int i = 0; i < 3; i++) {  // 3 rapid beeps for denial
      tone(BUZZER, 500, 150);
      delay(200);
    }
    delay(DENIED_DELAY);
    digitalWrite(LED_R, LOW);     // Turn off red LED
  }

  // Reset LCD to initial state
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Put card on rdr");

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

// Scroll text on LCD (added for better user feedback)
void scrollLCD(String text, int row) {
  if (text.length() <= 16) {
    lcd.setCursor(0, row);
    lcd.print(text);
    delay(1000);  // Brief pause for short texts
  } else {
    for (int i = 0; i <= text.length() - 16; i++) {
      lcd.setCursor(0, row);
      lcd.print(text.substring(i, i + 16));
      delay(300);
    }
  }
}