// Тесты AlashESPNow без железа (имитация радио ESP-NOW; ESP32, ядро 3.x, IDF 5.5):
//   g++ -std=c++17 -Wall -pthread -DESP32 -DESP_ARDUINO_VERSION_MAJOR=3 -DMOCK_IDF_MAJOR=5 -DMOCK_IDF_MINOR=5 -I mock -I ../../src test_espnow_esp32_idf55.cpp ../../src/AlashESPNow.cpp && ./a.out
#include "espnow_cases.h"
