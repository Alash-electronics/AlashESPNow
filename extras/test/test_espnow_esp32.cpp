// Тесты AlashESPNow без железа (имитация радио ESP-NOW; ESP32, ядро 3.x, IDF 5.3):
//   g++ -std=c++17 -Wall -pthread -DESP32 -DESP_ARDUINO_VERSION_MAJOR=3 -I mock -I ../../src test_espnow_esp32.cpp ../../src/AlashESPNow.cpp && ./a.out
#include "espnow_cases.h"
