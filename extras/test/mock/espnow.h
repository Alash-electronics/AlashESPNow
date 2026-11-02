// Имитация ESP8266 espnow.h: сигнатуры повторяют настоящий заголовок (u8, int)
#pragma once
#include "radio.h"
typedef uint8_t u8;
enum esp_now_role { ESP_NOW_ROLE_IDLE = 0, ESP_NOW_ROLE_CONTROLLER, ESP_NOW_ROLE_SLAVE, ESP_NOW_ROLE_COMBO, ESP_NOW_ROLE_MAX };
typedef void (*esp_now_recv_cb_t)(u8 *mac_addr, u8 *data, u8 len);
typedef void (*esp_now_send_cb_t)(u8 *mac_addr, u8 status);
inline int esp_now_init() { g_radio.initCalls++; if (!g_radio.initOk || !g_radio.wifiOn) return -1; g_radio.inited = true; return 0; }
inline int esp_now_deinit() { g_radio.deinitCalls++; g_radio.inited = false; g_radio.peers.clear(); g_radio.enc.clear(); g_radio.recvFn = nullptr; g_radio.sendFn = nullptr; return 0; }
inline int esp_now_set_self_role(u8) { return 0; }
inline int esp_now_register_recv_cb(esp_now_recv_cb_t cb) {
  g_radio.recvFn = [cb](const uint8_t *mac, const uint8_t *d, int len) { uint8_t m[6]; memcpy(m, mac, 6); std::vector<uint8_t> c(d, d + len); cb(m, c.data(), (u8)len); };
  return 0;
}
inline int esp_now_register_send_cb(esp_now_send_cb_t cb) {
  g_radio.sendFn = [cb](bool ok) { uint8_t m[6]; memcpy(m, g_radio.lastDst, 6); cb(m, ok ? 0 : 1); };
  return 0;
}
inline int esp_now_add_peer(u8 *mac, u8 role, u8 channel, u8 *key, u8 key_len) {
  (void)role; (void)channel;
  if (key && key_len != 16) return -1;
  int r = g_radio.addPeer(mac, key != nullptr && key_len == 16);
  return (r == 0 || r == 1) ? 0 : -1;
}
inline int esp_now_del_peer(u8 *mac) { g_radio.delPeer(mac); return 0; }
inline int esp_now_set_kok(u8 *k, u8 len) { if (len != 16) return -1; memcpy(g_radio.pmk, k, 16); g_radio.pmkSet = true; return 0; }
inline int esp_now_send(u8 *da, u8 *data, int len) { return g_radio.send(da, data, len) == 0 ? 0 : -1; }
