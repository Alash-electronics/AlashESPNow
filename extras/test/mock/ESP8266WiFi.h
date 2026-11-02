// ESP8266: тот же WiFi, плюс функции канала из user_interface.h
#pragma once
#include "WiFi.h"
inline bool wifi_set_channel(uint8_t ch) { if (!g_radio.wifiOn) return false; g_radio.channel = ch; return true; }
inline uint8_t wifi_get_channel() { return g_radio.channel; }
