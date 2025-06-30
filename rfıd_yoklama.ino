#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#define RST_PIN 9
#define SS_PIN 10
#define trigPin 7
#define echoPin 8
#define buzzerPin 4
#define greenLED 5
#define redLED 6

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

String sonOkunanUID = "";
bool kartOkundu = false;
unsigned long sonOkumaZamani = 0;
const int mesafeLimiti = 50;
const int kartGecerlilikSuresi = 5000;

String uidToIsim(String uid) {
  if (uid == "2b86b15") return "A.Bera Kiziloglu";
  else if (uid == "1316add9") return "Kivilcim Ozdemir";
  else if (uid == "a6caf84") return "Enes Gozen";
  else if (uid == "4373f74") return "Beril Uzuncan";
  else return "";
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  rtc.begin();

  // ⏰ BU SATIRI SADECE İLK YÜKLEMEDE AÇ ➜ YÜKLEDİKTEN SONRA TEKRAR // İLE KAPAT
  //rtc.adjust(DateTime(2025, 6, 27, 20, 55, 0));  // YYYY, MM, DD, HH, MM, SS

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Sistem Hazir");
  lcd.setCursor(0, 1);
  lcd.print("Kart Bekleniyor");

  Serial.println("Sistem Hazir. Kart Bekleniyor...");
}

void loop() {
  mesafeKontrol();

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    kartOkundu = true;
    sonOkumaZamani = millis();

    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }

    uid.toLowerCase();

    String isim = uidToIsim(uid);

    DateTime now = rtc.now();
    String saat = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

    lcd.clear();

    if (isim != "") {
      // BUZZER VE LED HEMEN ÇALSIN
      digitalWrite(greenLED, HIGH);
      tone(buzzerPin, 1000, 300);
      delay(300);
      tone(buzzerPin, 1000, 300);
      delay(300);
      noTone(buzzerPin);
      digitalWrite(greenLED, LOW);

      lcd.setCursor(0, 0);
      lcd.print(isim.substring(0, 16));

      String mesaj = "Giris Saat: " + saat;
      for (int i = 0; i <= mesaj.length() - 16; i++) {
        lcd.setCursor(0, 1);
        lcd.print(mesaj.substring(i, i + 16));
        delay(300);
      }

      Serial.print("Kart Okundu - ");
      Serial.print(isim);
      Serial.print(" UID: ");
      Serial.println(uid);
      Serial.print("Giris Saati: ");
      Serial.println(saat);
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Tanimsiz Kart!");
      lcd.setCursor(0, 1);
      lcd.print("Yetkisiz Giris");

      digitalWrite(redLED, HIGH);
      tone(buzzerPin, 500, 500);
      delay(500);
      digitalWrite(redLED, LOW);
      noTone(buzzerPin);
    }

    rfid.PICC_HaltA();
  }

  if (kartOkundu && millis() - sonOkumaZamani > kartGecerlilikSuresi) {
    kartOkundu = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kart Bekleniyor");
  }
}

void mesafeKontrol() {
  long sure;
  int mesafe;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  sure = pulseIn(echoPin, HIGH);
  mesafe = sure * 0.034 / 2;

  if (!kartOkundu && mesafe > 0 && mesafe <= mesafeLimiti) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lutfen Kartinizi");
    lcd.setCursor(0, 1);
    lcd.print("Okutunuz!");
    digitalWrite(redLED, HIGH);
    tone(buzzerPin, 1000);
    delay(1000);
    digitalWrite(redLED, LOW);
    noTone(buzzerPin);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kart Bekleniyor");
  }
}