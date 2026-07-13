#include "config.h"
#include "display.h"
#include "mcp23017.h"
#include "web.h"
#include "rfid.h"
#include <Wire.h>

String lastUID = "Brak";

// --- FREERTOS ZADANIE DLA WYŚWIETLACZA ---
void displayTaskLoop(void * parameter) {
  for(;;) {
    // To kręci się w nieskończoność na rdzeniu 0
    displayTask();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); 

  Serial.println("\n--- START PROGRAMU ---");

  Wire.setPins(19, 20);
  Wire.begin(); 
  Wire.setClock(100000);
  Serial.println("1. I2C odpalone");

  rfidInit();
  Serial.println("2. Czytnik RFID OK");

  displayInit();
  Serial.println("3. Wyswietlacz OK");

  // ODPALAMY WYŚWIETLACZ NA DRUGIM RDZENIU (Core 0)
  xTaskCreatePinnedToCore(
    displayTaskLoop,   /* Funkcja zadania */
    "DisplayTask",     /* Nazwa zadania */
    2048,              /* Rozmiar stosu w bajtach */
    NULL,              /* Parametry wejściowe */
    1,                 /* Priorytet (1 - standardowy) */
    NULL,              /* Uchwyt (nie potrzebujemy go tu) */
    0                  /* Numer rdzenia (0) */
  );
  Serial.println(" -> Wyswietlacz przypiety do rdzenia 0!");

  mcpInit();
  Serial.println("4. Ekspandery MCP OK");

  webInit();
  Serial.println("5. Webserver (W6100) OK");

  setDisplayValue(1234);
  Serial.println("--- SETUP ZAKONCZONY ---");
}

void loop(){
  // UWAGA: displayTask() zostało stąd usunięte, bo działa w tle na Core 0!
  
  rfidTask();

  if(rfidHasNew()){
    lastUID = rfidGetUID();
    Serial.print("WEB UID: ");
    Serial.println(lastUID);
  }

  webTask();
}