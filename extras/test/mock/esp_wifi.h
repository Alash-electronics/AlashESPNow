#pragma once
#include "WiFi.h"
typedef int esp_err_t;
#ifndef ESP_OK
#define ESP_OK 0
#endif
typedef enum { WIFI_SECOND_CHAN_NONE = 0, WIFI_SECOND_CHAN_ABOVE, WIFI_SECOND_CHAN_BELOW } wifi_second_chan_t;
typedef enum { WIFI_IF_STA = 0, WIFI_IF_AP = 1 } wifi_interface_t;
typedef struct { int rssi; } wifi_pkt_rx_ctrl_t;
typedef struct { int dummy; } wifi_tx_info_t;
inline esp_err_t esp_wifi_set_channel(uint8_t ch, wifi_second_chan_t) { if (!g_radio.wifiOn) return 0x3001; g_radio.channel = ch; return ESP_OK; }
