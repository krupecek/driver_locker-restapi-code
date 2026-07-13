#include "config.h"
#include <Wire.h>

#define IODIRA 0x00
#define GPIOA  0x12

byte mcpAddr[3] = {0x20, 0x21, 0x22};
byte state[3] = {0,0,0};

void writeMCP(byte addr, byte reg, byte val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void mcpInit() {

  for(int i=0;i<3;i++){
    writeMCP(mcpAddr[i], IODIRA, 0x00);
    writeMCP(mcpAddr[i], GPIOA, 0x00);
  }
}

void setRelay(int id, bool on) {
  if(id < 1 || id > 24) return;

  int i = id - 1;
  int chip = i / 8;
  int pin  = i % 8;

  if(on) state[chip] |= (1 << pin);
  else    state[chip] &= ~(1 << pin);

  writeMCP(mcpAddr[chip], GPIOA, state[chip]);
}