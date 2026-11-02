// Имитация класса WiFi (достаточно для ESP-NOW): режим, канал, MAC, подключение к роутеру.
#pragma once
#include "Arduino.h"
#include "radio.h"
enum WiFiMode_t { WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3 };
enum { WL_IDLE_STATUS = 0, WL_CONNECTED = 3, WL_DISCONNECTED = 6 };
struct WiFiClass {
  WiFiMode_t md = WIFI_OFF; bool connected = false; int modeCalls = 0;
  void mode(WiFiMode_t m) { md = m; modeCalls++; g_radio.wifiOn = (m != WIFI_OFF); if (m == WIFI_OFF) g_radio.inited = false; }
  WiFiMode_t getMode() { return md; }
  uint8_t status() { return connected ? WL_CONNECTED : WL_DISCONNECTED; }
  uint8_t channel() { return g_radio.channel; }
  String macAddress() { return String("24:6F:28:AA:BB:01"); }
};
inline WiFiClass WiFi;
