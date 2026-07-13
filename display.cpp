#include "config.h"

static int digits[4] = {0,0,0,0};
static int cur = 0;

void split(int v) {
  digits[0] = (v / 1000) % 10;
  digits[1] = (v / 100) % 10;
  digits[2] = (v / 10) % 10;
  digits[3] = v % 10;
}

void setDisplayValue(int v) {
  split(v);
}

void displayInit() {
  for (int i=0;i<8;i++) {
    pinMode(segPins[i], OUTPUT);
    digitalWrite(segPins[i], HIGH);
  }

  for (int i=0;i<4;i++) {
    pinMode(digPins[i], OUTPUT);
    digitalWrite(digPins[i], LOW);
  }
}

static void allOff() {
  for (int i=0;i<4;i++) digitalWrite(digPins[i], LOW);
}

static void setSeg(int n) {
  for (int i=0;i<8;i++) {
    digitalWrite(segPins[i], numbers[n][i] ? LOW : HIGH);
  }
}

void displayTask() {
  allOff();
  setSeg(digits[cur]);
  digitalWrite(digPins[cur], HIGH);

  cur++;
  if(cur >= 4) cur = 0;

  // Używamy zwykłego delay(), żeby FreeRTOS mógł zarządzać rdzeniem
  delay(2); 
}