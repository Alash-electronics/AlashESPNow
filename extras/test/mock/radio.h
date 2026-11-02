// Имитация радио ESP-NOW: список собеседников, отправленные кадры, подтверждения с задержкой, входящие сообщения.
#pragma once
#include <array>
#include <functional>
#include <vector>
#include "Arduino.h"
struct Frame { uint8_t mac[6]; std::vector<uint8_t> data; };
struct Pending { uint32_t due; bool ok; };
struct Radio {
  bool wifiOn = false, initOk = true, inited = false;
  int initCalls = 0, deinitCalls = 0;
  uint8_t channel = 1;
  int maxPeers = 20, maxEnc = 6;
  std::vector<std::array<uint8_t, 6>> peers; std::vector<bool> enc;
  bool pmkSet = false; uint8_t pmk[16] = {0};
  std::vector<Frame> sent;
  bool ackOk = true, ackNever = false, noMem = false; int forceRc = 0; uint32_t ackDelayMs = 3;   // forceRc: принудительный код ошибки esp_now_send
  std::vector<Pending> pend; uint8_t lastDst[6] = {0};
  std::function<void(const uint8_t *, const uint8_t *, int)> recvFn;
  std::function<void(bool)> sendFn;
  static bool isBcast(const uint8_t *m) { for (int i = 0; i < 6; i++) if (m[i] != 0xFF) return false; return true; }
  int find(const uint8_t *m) { for (size_t i = 0; i < peers.size(); i++) if (memcmp(peers[i].data(), m, 6) == 0) return (int)i; return -1; }
  int encCount() { int n = 0; for (bool e : enc) n += e; return n; }
  // 0 ok, 1 уже есть, 2 список полон, 3 ESP-NOW не запущен
  int addPeer(const uint8_t *m, bool e) {
    if (!inited) return 3;
    if (find(m) >= 0) return 1;
    if ((int)peers.size() >= maxPeers) return 2;
    if (e && encCount() >= maxEnc) return 2;
    std::array<uint8_t, 6> a; memcpy(a.data(), m, 6); peers.push_back(a); enc.push_back(e); return 0;
  }
  void delPeer(const uint8_t *m) { int i = find(m); if (i >= 0) { peers.erase(peers.begin() + i); enc.erase(enc.begin() + i); } }
  // 0 ok, 1 не запущен, 2 плохие аргументы, 3 нет такого собеседника, 4 нет памяти
  int send(const uint8_t *m, const uint8_t *d, int len) {
    if (!inited) return 1;
    if (forceRc) return forceRc;
    if (len <= 0 || len > 250) return 2;
    if (find(m) < 0) return 3;
    if (noMem) return 4;
    Frame f; memcpy(f.mac, m, 6); f.data.assign(d, d + len); sent.push_back(f);
    memcpy(lastDst, m, 6);
    if (!ackNever) pend.push_back({g_ms + ackDelayMs, isBcast(m) ? true : ackOk});
    return 0;
  }
  void tick() {
    for (size_t i = 0; i < pend.size();) {
      if (pend[i].due <= g_ms) { bool ok = pend[i].ok; pend.erase(pend.begin() + i); if (sendFn) sendFn(ok); } else i++;
    }
  }
  void deliver(const uint8_t *mac, const uint8_t *d, int len) { if (inited && recvFn) recvFn(mac, d, len); }
  void deliver(const uint8_t *mac, const char *text) { deliver(mac, (const uint8_t *)text, (int)strlen(text)); }
  void reset() { *this = Radio(); }
};
inline Radio g_radio;
