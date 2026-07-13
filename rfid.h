#pragma once
#include <Arduino.h>

void rfidInit();
void rfidTask();
bool rfidHasNew();
String rfidGetUID();