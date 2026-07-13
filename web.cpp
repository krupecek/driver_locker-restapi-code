#include "config.h"
#include "mcp23017.h"
#include "display.h"
extern String lastUID;
#include <SPI.h>
#include <Ethernet_Generic.h>
#include <Update.h> 

byte mac[] = {0xDE,0xAD,0xBE,0xEF,0xFE,0x01};
IPAddress ip(192,168,1,50);

EthernetServer server(80);

void webInit() {
  pinMode(W6100_RST, OUTPUT);
  digitalWrite(W6100_RST, LOW);
  delay(200);
  digitalWrite(W6100_RST, HIGH);
  delay(1000);

  SPI.begin();
  Ethernet.init(W6100_CS);
  Ethernet.begin(mac, ip);

  server.begin();
}

static void handle(EthernetClient c) {
  String req = c.readStringUntil('\n');

  // =========================================================================
  // 1. STRONA FORMULARZA AKTUALIZACJI (GET /update)
  // =========================================================================
  if(req.indexOf("GET /update ") >= 0) {
    c.println("HTTP/1.1 200 OK");
    c.println("Content-Type: text/html");
    c.println("Connection: close\n");
    c.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 OTA</title></head><body>");
    c.println("<h2>Aktualizacja Oprogramowania Sterownika</h2>");
    c.println("<p>Wybierz plik skompilowany do formatu <b>.bin</b> z Arduino IDE.</p>");
    
    // Formularz oparty o nowoczesny JavaScript - wysyła CZYSTE bajty pliku, bez śmieci HTTP
    c.println("<input type='file' id='fileInput' accept='.bin'><br><br>");
    c.println("<button onclick='performUpdate()'>Uruchom aktualizacje</button>");
    c.println("<p id='status'></p>");

    c.println("<script>");
    c.println("function performUpdate() {");
    c.println("  var fileInput = document.getElementById('fileInput');");
    c.println("  if(fileInput.files.length === 0) { alert('Wybierz plik!'); return; }");
    c.println("  var file = fileInput.files[0];");
    c.println("  document.getElementById('status').innerText = 'Wysyłanie pliku... Proszę czekać...';");
    c.println("  fetch('/doUpdate', { method: 'POST', body: file })"); // Wysyłamy surowe bajty
    c.println("  .then(response => {");
    c.println("    if(response.ok) {");
    c.println("      document.getElementById('status').innerText = 'Sukces! Restart sterownika, wracam za 5 sek...';");
    c.println("      setTimeout(window.location.href='/', 5000);");
    c.println("    } else { document.getElementById('status').innerText = 'Błąd serwera podczas aktualizacji!'; }");
    c.println("  }).catch(e => { document.getElementById('status').innerText = 'Błąd sieci!'; });");
    c.println("}");
    c.println("</script>");

    c.println("<br><br><a href='/'>Powrot do panelu głównego</a>");
    c.println("</body></html>");
    c.stop();
    return;
  }

  // =========================================================================
  // 2. OBSŁUGA POSTRZYMANIA I WGRANIA PLIKU BIN (POST /doUpdate)
  // =========================================================================
  if(req.indexOf("POST /doUpdate") >= 0) {
    int contentLength = 0;

    // Przetwarzamy nagłówki HTTP, żeby wiedzieć, ile bajtów leci
    while (c.connected()) {
      String line = c.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) break; // Koniec nagłówków, teraz lecą czyste bajty pliku

      if (line.startsWith("Content-Length: ")) {
        contentLength = line.substring(16).toInt();
      }
    }

    if (contentLength > 0) {
      // Inicjalizacja flashowania - podajemy dokładny rozmiar pliku z nagłówka Content-Length
      if (Update.begin(contentLength)) { 
        // Przepisujemy CZYSTY strumień bajtów prosto z sieci (dzięki JS nie ma tu śmieci)
        size_t written = Update.writeStream(c);
        
        if (Update.end()) {
          if (Update.isFinished()) {
            c.println("HTTP/1.1 200 OK");
            c.println("Content-Type: text/plain");
            c.println("Connection: close\n");
            c.println("OK"); // Odpowiedź dla JavaScriptu, że wszystko przeszło
            c.stop();
            
            delay(1000);
            ESP.restart(); // Twardy restart mikrokontrolera
            return;
          }
        } else {
          Serial.print("Blad OTA nr: ");
          Serial.println(Update.getError());
        }
      }
    }

    c.println("HTTP/1.1 500 Internal Server Error");
    c.println("Connection: close\n");
    c.stop();
    return;
  }

  // =========================================================================
  // 3. ENDPOINT AJAX DLA AUTOPODMIANY UID (GET /uid)
  // =========================================================================
  if(req.indexOf("GET /uid ") >= 0) {
    c.println("HTTP/1.1 200 OK");
    c.println("Content-Type: text/plain");
    c.println("Connection: close\n");
    c.print(lastUID);
    c.stop();
    return;
  }

  // =========================================================================
  // 4. ENDPOINTY STEROWANIA
  // =========================================================================
  if(req.indexOf("GET /set?") >= 0) {
    int p = req.indexOf("v=");
    if(p > 0) {
      int v = req.substring(p+2).toInt();
      if(v >= 0 && v <= 9999) setDisplayValue(v);
    }
  }

  if(req.indexOf("GET /on?i=") >= 0) {
    int id = req.substring(req.indexOf("i=")+2).toInt();
    setRelay(id,true);
  }

  if(req.indexOf("GET /off?i=") >= 0) {
    int id = req.substring(req.indexOf("i=")+2).toInt();
    setRelay(id,false);
  }

  // =========================================================================
  // 5. STRONA GŁÓWNA PANELU
  // =========================================================================
  c.println("HTTP/1.1 200 OK");
  c.println("Content-Type: text/html");
  c.println("Connection: close\n");
  
  c.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Panel</title></head><body>");

  c.println("<h2>Ostatnia karta RFID</h2>");
  c.print("<b><span id='karta'>");
  c.print(lastUID);
  c.println("</span></b><br>");

  c.println("<script>");
  c.println("setInterval(function() {");
  c.println("  fetch('/uid').then(r => r.text()).then(nowyUID => {");
  c.println("    let staryUID = document.getElementById('karta').innerText;");
  c.println("    if(nowyUID !== staryUID && nowyUID.length > 0) {");
  c.println("      document.getElementById('karta').innerText = nowyUID;"); 
  c.println("    }");
  c.println("  });");
  c.println("}, 1000);");
  c.println("</script>");

  c.println("<h2>ESP32 PANEL</h2>");

  c.println("<form action='/set'>");
  c.println("<input name='v' type='number'>");
  c.println("<input type='submit' value='SET'>");
  c.println("</form><hr>");

  for(int i=1;i<=24;i++){
    c.print("Relay ");
    c.print(i);
    c.print(" <a href='/on?i=");
    c.print(i);
    c.print("'>ON</a> ");
    c.print("<a href='/off?i=");
    c.print(i);
    c.println("'>OFF</a><br>");
  }
  
  c.println("<br><hr><p><a href='/update'>Idz do panelu aktualizacji systemu jozef (OTA)</a></p>");
  c.println("</body></html>");
  c.stop();
}

void webTask() {
  EthernetClient client = server.available();
  if(client) handle(client);
}