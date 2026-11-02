// Тесты AlashESPNow без железа (имитация радио ESP-NOW; ESP32, ядро 2.x, IDF 4.4):
//   g++ -std=c++17 -Wall -pthread -DESP32 -DESP_ARDUINO_VERSION_MAJOR=2 -DMOCK_IDF_MAJOR=4 -DMOCK_IDF_MINOR=4 -I mock -I ../../src test_espnow_esp32_core2.cpp ../../src/AlashESPNow.cpp && ./a.out
#include "espnow_cases.h"
