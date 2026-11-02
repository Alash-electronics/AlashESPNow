// Тесты AlashESPNow без железа (имитация радио ESP-NOW; ESP8266):
//   g++ -std=c++17 -Wall -pthread -DESP8266 -I mock -I ../../src test_espnow_esp8266.cpp ../../src/AlashESPNow.cpp && ./a.out
#include "espnow_cases.h"
