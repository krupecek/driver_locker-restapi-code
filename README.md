# driver_locker-restapi-code

Pliki dotyczące projektu sterownika opartego na esp32 

Obsluga wyswietlacza, obsługa rfid czytnik po i2c (0x24), obsługa modułu ethernet W6100, obsługa mcp23017 

bibioteka esp32 w wersji 3.3.10

Na prototypie testując mamy wersję 3.3.10 biblioteki esp32 

ESP32 pracuje z wiznet w6100 (ethernet) po SPI
MCP23017 po i2c
Rfid Reader PN532 - i2c (0x24)
Wyswietlacz po multiplexie SN74HCT245NE4 dziala na stanie logicznym 5V
