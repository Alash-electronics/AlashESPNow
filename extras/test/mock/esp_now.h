// Имитация ESP32 esp_now.h: типы и сигнатуры обработчиков повторяют настоящий заголовок для выбранной версии ядра (см. ESP_ARDUINO_VERSION_MAJOR, MOCK_IDF_*).
#pragma once
#include "esp_wifi.h"
#include "esp_idf_version.h"
#define ESP_ERR_ESPNOW_BASE 0x3064
#define ESP_ERR_ESPNOW_NOT_INIT (ESP_ERR_ESPNOW_BASE + 1)
#define ESP_ERR_ESPNOW_ARG (ESP_ERR_ESPNOW_BASE + 2)
#define ESP_ERR_ESPNOW_NO_MEM (ESP_ERR_ESPNOW_BASE + 3)
#define ESP_ERR_ESPNOW_FULL (ESP_ERR_ESPNOW_BASE + 4)
#define ESP_ERR_ESPNOW_NOT_FOUND (ESP_ERR_ESPNOW_BASE + 5)
#define ESP_ERR_ESPNOW_INTERNAL (ESP_ERR_ESPNOW_BASE + 6)
#define ESP_ERR_ESPNOW_EXIST (ESP_ERR_ESPNOW_BASE + 7)
#define ESP_NOW_ETH_ALEN 6
#define ESP_NOW_KEY_LEN 16
#define ESP_NOW_MAX_DATA_LEN 250
typedef struct esp_now_peer_info {
  uint8_t peer_addr[ESP_NOW_ETH_ALEN]; uint8_t lmk[ESP_NOW_KEY_LEN]; uint8_t channel; wifi_interface_t ifidx; bool encrypt; void *priv;
} esp_now_peer_info_t;
typedef struct esp_now_recv_info { uint8_t *src_addr; uint8_t *des_addr; wifi_pkt_rx_ctrl_t *rx_ctrl; } esp_now_recv_info_t;
typedef enum { ESP_NOW_SEND_SUCCESS = 0, ESP_NOW_SEND_FAIL } esp_now_send_status_t;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
typedef void (*esp_now_recv_cb_t)(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len);
#else
typedef void (*esp_now_recv_cb_t)(const uint8_t *mac_addr, const uint8_t *data, int data_len);
#endif
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
typedef void (*esp_now_send_cb_t)(const wifi_tx_info_t *tx_info, esp_now_send_status_t status);
#else
typedef void (*esp_now_send_cb_t)(const uint8_t *mac_addr, esp_now_send_status_t status);
#endif
inline esp_err_t esp_now_init() { g_radio.initCalls++; if (!g_radio.initOk || !g_radio.wifiOn) return ESP_ERR_ESPNOW_INTERNAL; g_radio.inited = true; return ESP_OK; }
inline esp_err_t esp_now_deinit() { g_radio.deinitCalls++; g_radio.inited = false; g_radio.peers.clear(); g_radio.enc.clear(); g_radio.recvFn = nullptr; g_radio.sendFn = nullptr; return ESP_OK; }
inline esp_err_t esp_now_register_recv_cb(esp_now_recv_cb_t cb) {
  if (!g_radio.inited) return ESP_ERR_ESPNOW_NOT_INIT;
  g_radio.recvFn = [cb](const uint8_t *mac, const uint8_t *d, int len) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    esp_now_recv_info_t info; uint8_t m[6]; memcpy(m, mac, 6); info.src_addr = m; info.des_addr = nullptr; info.rx_ctrl = nullptr; cb(&info, d, len);
#else
    cb(mac, d, len);
#endif
  };
  return ESP_OK;
}
inline esp_err_t esp_now_register_send_cb(esp_now_send_cb_t cb) {
  if (!g_radio.inited) return ESP_ERR_ESPNOW_NOT_INIT;
  g_radio.sendFn = [cb](bool ok) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    wifi_tx_info_t t; cb(&t, ok ? ESP_NOW_SEND_SUCCESS : ESP_NOW_SEND_FAIL);
#else
    cb(g_radio.lastDst, ok ? ESP_NOW_SEND_SUCCESS : ESP_NOW_SEND_FAIL);
#endif
  };
  return ESP_OK;
}
inline esp_err_t esp_now_add_peer(const esp_now_peer_info_t *p) {
  int r = g_radio.addPeer(p->peer_addr, p->encrypt);
  return r == 0 ? ESP_OK : r == 1 ? ESP_ERR_ESPNOW_EXIST : r == 2 ? ESP_ERR_ESPNOW_FULL : ESP_ERR_ESPNOW_NOT_INIT;
}
inline esp_err_t esp_now_del_peer(const uint8_t *mac) { g_radio.delPeer(mac); return ESP_OK; }
inline esp_err_t esp_now_set_pmk(const uint8_t *k) { memcpy(g_radio.pmk, k, 16); g_radio.pmkSet = true; return ESP_OK; }
inline esp_err_t esp_now_send(const uint8_t *mac, const uint8_t *d, size_t len) {
  int r = g_radio.send(mac, d, (int)len);
  return r == 0 ? ESP_OK : r == 1 ? ESP_ERR_ESPNOW_NOT_INIT : r == 2 ? ESP_ERR_ESPNOW_ARG : r == 3 ? ESP_ERR_ESPNOW_NOT_FOUND : ESP_ERR_ESPNOW_NO_MEM;
}
