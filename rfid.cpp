#include "config.h"
#include "rfid.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   -1
#define PN532_RESET -1

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

static bool newCard = false;
static String currentUID = "";
static unsigned long lastCheckTime = 0;

void rfidInit() {
  nfc.begin();
  
  // Wymuszamy Twoje piny po tym, jak biblioteka Adafruit próbowała je zresetować.
  // To ustawienie działało wcześniej w 100%!
  Wire.begin(19, 20); 
  Wire.setClock(100000);

  // TRIK ODBLOKOWUJĄCY: Wysyłamy ślepą komendę. 
  // Jeśli PN532 zawiesił się na szukaniu karty z poprzedniego resetu, to go obudzi.
  nfc.getFirmwareVersion();
  delay(50); 

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("BŁĄD: Nie znaleziono czytnika PN532 na szynie I2C!");
    return;
  }

  Serial.print("Znaleziono układ PN5"); 
  Serial.println((versiondata >> 24) & 0xFF, HEX);

  nfc.SAMConfig();
  
  // Wyrzuciłem modyfikację "Retries", która wcześniej zepsuła nam sprawę. 
  // Zostawiamy ustawienia fabryczne, a czas kontrolujemy w rfidTask.
}

void rfidTask() {
  // Dajemy pętli głównej i wyświetlaczowi odetchnąć - sprawdzamy co 200ms
  if (millis() - lastCheckTime < 200) {
    return;
  }
  lastCheckTime = millis();

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  // Funkcja czeka maksymalnie 100 ms na kartę. Jak nie ma, idzie dalej.
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);

  if (success) {
    String tempUID = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) tempUID += "0"; 
      tempUID += String(uid[i], HEX);
    }
    tempUID.toUpperCase();
    
    Serial.print("BINGO! SUKCES - Odczytano UID karty: ");
    Serial.println(tempUID);

    if (tempUID != currentUID || !newCard) {
        currentUID = tempUID;
        newCard = true;
    }
  }
}

bool rfidHasNew() {
  if (newCard) {
    newCard = false;
    return true;
  }
  return false;
}

String rfidGetUID() {
  return currentUID;
}