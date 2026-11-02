// Общие проверки AlashESPNow: подключаются обёртками test_espnow_*.cpp для разных ядер (ESP32 core 2 / 3, IDF 5.5, ESP8266).
#include <assert.h>
#include <stdio.h>
#include <atomic>
#include <string>
#include <thread>
#include "Arduino.h"
#include "AlashESPNow.h"

typedef AlashESPNow N;
static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)

static const uint8_t MA[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01};
static const uint8_t MB[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02};
static const char *SA = "AA:BB:CC:DD:EE:01";
static const char *SB = "AA:BB:CC:DD:EE:02";

static void fresh() { g_radio.reset(); WiFi = WiFiClass(); g_ms = 100000; g_onDelay = [] { g_radio.tick(); }; }
static void mk(int i, uint8_t *m) { m[0] = 0x02; m[1] = m[2] = m[3] = m[4] = 0; m[5] = (uint8_t)i; }
static bool has(const Print &p, const char *s) { return p.log.find(s) != std::string::npos; }

static int cbCalls = 0; static uint8_t cbLen = 0, cbMac[6]; static std::string cbText;
static void onMsg(const uint8_t *mac, const uint8_t *data, uint8_t len) { cbCalls++; cbLen = len; memcpy(cbMac, mac, 6); cbText.assign((const char *)data, len); }

struct Sample { int16_t a; float b; };

int main() {
  // 1) begin: режим STA, канал, ESP-NOW запущен
  fresh();
  {
    N n;
    CHECK(n.begin(6));
    CHECK(WiFi.getMode() == WIFI_STA && g_radio.inited && g_radio.channel == 6 && n.channel() == 6);
    CHECK(n.peerCount() == 0 && n.error() == N::ERR_OK);
    CHECK(n.myMac() == "24:6F:28:AA:BB:01");
  }
  CHECK(!g_radio.inited);                                   // деструктор остановил ESP-NOW
  fresh(); g_radio.channel = 3;
  { N n; CHECK(n.begin() && n.channel() == 3); }            // без номера канал не трогаем
  fresh(); WiFi.md = WIFI_AP_STA; g_radio.wifiOn = true;
  { N n; CHECK(n.begin() && WiFi.modeCalls == 0 && WiFi.md == WIFI_AP_STA); }   // свой режим Wi-Fi не меняем

  // 2) ошибки запуска и вызовы до begin()
  fresh();
  {
    N n;
    CHECK(!n.begin(14) && n.error() == N::ERR_CHANNEL && !g_radio.inited);
    CHECK(!n.send(SA, "x") && n.error() == N::ERR_NOT_BEGUN);
    CHECK(!n.addPeer(SA) && n.error() == N::ERR_NOT_BEGUN);
    CHECK(!n.setKey("0123456789abcdef") && n.error() == N::ERR_NOT_BEGUN);
    CHECK(!n.broadcast("x") && n.error() == N::ERR_NOT_BEGUN);
  }
  fresh(); WiFi.md = WIFI_STA; g_radio.wifiOn = true; WiFi.connected = true; g_radio.channel = 6;
  {
    N n;
    CHECK(!n.begin(11) && n.error() == N::ERR_CHANNEL && g_radio.channel == 6);   // канал занят роутером
    CHECK(n.begin(6));
    CHECK(n.begin());
  }
  fresh(); g_radio.initOk = false;
  {
    N n; Print p;
    CHECK(!n.begin() && n.error() == N::ERR_INIT);
    n.diagnose(p);
    CHECK(has(p, "begin()") && has(p, "ESP-NOW"));
  }

  // 3) MAC-адреса
  {
    uint8_t m[6];
    CHECK(N::parseMac("aa:bb:cc:dd:ee:0f", m) && m[0] == 0xAA && m[5] == 0x0F);
    CHECK(N::parseMac("AA-BB-CC-DD-EE-01", m) && m[4] == 0xEE);
    CHECK(!N::parseMac("AA:BB:CC:DD:EE", m));
    CHECK(!N::parseMac("AA:BB:CC:DD:EE:0", m));
    CHECK(!N::parseMac("AA:BB:CC:DD:EE:GG", m));
    CHECK(!N::parseMac("AA:BB:CC:DD:EE;01", m));
    CHECK(!N::parseMac("AABBCCDDEE01", m));
    CHECK(!N::parseMac("", m) && !N::parseMac(nullptr, m));
    uint8_t keep[6] = {1, 2, 3, 4, 5, 6};
    memcpy(m, keep, 6); N::parseMac("AA:BB:CC:DD:EE:ZZ", m);
    CHECK(memcmp(m, keep, 6) == 0);                         // при ошибке результат не портится
    CHECK(N::macToString(MA) == "AA:BB:CC:DD:EE:01");
  }

  // 4) собеседники
  fresh();
  {
    N n; n.begin(1);
    CHECK(n.addPeer(SA) && n.peerCount() == 1 && g_radio.peers.size() == 1);
    CHECK(n.addPeer(MA) && n.peerCount() == 1 && g_radio.peers.size() == 1);   // повтор не плодит записи
    CHECK(!n.addPeer("bad") && n.error() == N::ERR_BAD_MAC);
    CHECK(!n.addPeer(SB, "short") && n.error() == N::ERR_BAD_KEY && n.peerCount() == 1);
    CHECK(n.addPeer(SB, "0123456789abcdef") && g_radio.encCount() == 1 && n.peerCount() == 2);
    CHECK(n.removePeer(SA) && n.peerCount() == 1 && g_radio.peers.size() == 1);
    CHECK(!n.removePeer(SA) && n.error() == N::ERR_OK);
    CHECK(!n.removePeer("zz") && n.error() == N::ERR_BAD_MAC);
    CHECK(n.removePeer(MB) && n.peerCount() == 0);
  }
  fresh();
  {
    N n; n.begin(1); uint8_t m[6];
    for (int i = 0; i < ALASH_ESPNOW_MAX_PEERS; i++) { mk(i, m); CHECK(n.addPeer(m)); }
    mk(99, m);
    CHECK(!n.addPeer(m) && n.error() == N::ERR_PEERS_FULL && n.peerCount() == ALASH_ESPNOW_MAX_PEERS);
    CHECK(!n.send(m, "x") && n.error() == N::ERR_PEERS_FULL);                  // автодобавление тоже упирается в предел
    mk(3, m); CHECK(n.removePeer(m)); mk(99, m); CHECK(n.addPeer(m));          // освободили место
  }
  fresh();
  {
    N n; n.begin(1); uint8_t m[6];
    for (int i = 0; i < g_radio.maxEnc; i++) { mk(i, m); CHECK(n.addPeer(m, "0123456789abcdef")); }
    mk(50, m);
    CHECK(!n.addPeer(m, "0123456789abcdef") && n.error() == N::ERR_PEERS_FULL && n.peerCount() == g_radio.maxEnc);
    CHECK(n.addPeer(m));                                                       // без шифрования ещё можно
  }
  fresh();
  {
    N n; n.begin(1);
    CHECK(n.addPeer(SA) && g_radio.encCount() == 0);
    CHECK(n.addPeer(SA, "0123456789abcdef") && g_radio.encCount() == 1 && n.peerCount() == 1);   // ключ для известного
  }

  // 5) отправка: байты, текст, структура; автодобавление; подтверждение
  fresh();
  {
    N n; n.begin(1);
    CHECK(n.send(SA, (const uint8_t *)"\x01\x02\x03", 3));
    CHECK(n.peerCount() == 1);
    CHECK(g_radio.sent.size() == 1 && g_radio.sent[0].data.size() == 3 && g_radio.sent[0].data[2] == 3 && memcmp(g_radio.sent[0].mac, MA, 6) == 0);
    CHECK(n.sending());
    CHECK(n.wait() && !n.sending());
    CHECK(n.sentCount() == 1 && n.deliveredCount() == 1 && n.failedCount() == 0);
    CHECK(n.send(MB, "hello") && n.wait());
    CHECK(g_radio.sent[1].data.size() == 5 && memcmp(g_radio.sent[1].data.data(), "hello", 5) == 0 && memcmp(g_radio.sent[1].mac, MB, 6) == 0);
    Sample s = {7, 1.5f};
    CHECK(n.sendStruct(SA, s) && n.wait());
    CHECK(g_radio.sent[2].data.size() == sizeof(Sample) && memcmp(g_radio.sent[2].data.data(), &s, sizeof(s)) == 0);
    CHECK(n.sendStruct(MA, s) && n.wait() && g_radio.sent.size() == 4);
    CHECK(n.send(SA, "text") && n.wait() && n.deliveredCount() == 5);
  }
  fresh();
  {
    N n; n.begin(1); uint8_t big[251] = {0};
    CHECK(!n.send(SA, (const uint8_t *)"", 0) && n.error() == N::ERR_EMPTY);
    CHECK(!n.send(SA, "") && n.error() == N::ERR_EMPTY);
    CHECK(!n.send(SA, (const char *)nullptr) && n.error() == N::ERR_EMPTY);
    CHECK(!n.send(SA, big, 251) && n.error() == N::ERR_TOO_LONG);
    CHECK(!n.send("zz", "a") && n.error() == N::ERR_BAD_MAC);
    CHECK(n.sentCount() == 0 && !n.sending() && g_radio.sent.empty());     // отклонённые не уходят в эфир
    CHECK(n.send(SA, big, 250) && n.error() == N::ERR_OK && n.wait());     // предел ровно 250
    CHECK(n.send(SA, "ok") && n.error() == N::ERR_OK);                     // успех сбрасывает прошлую ошибку
  }

  // 6) результаты доставки
  fresh();
  {
    N n; n.begin(1);
    g_radio.ackOk = false;
    CHECK(n.send(SA, "x") && !n.wait() && n.error() == N::ERR_NO_DELIVERY && n.failedCount() == 1 && n.deliveredCount() == 0);
    g_radio.ackOk = true;
    CHECK(n.send(SA, "x") && n.wait() && n.deliveredCount() == 1);
    g_radio.ackNever = true;
    uint32_t t0 = millis();
    CHECK(n.send(SA, "x") && !n.wait(50) && n.error() == N::ERR_NO_DELIVERY);
    CHECK(millis() - t0 >= 50 && millis() - t0 <= 60);                     // ждали ровно заданное время
  }
  fresh();
  { N n; n.begin(1); CHECK(!n.wait() && n.error() == N::ERR_OK); }         // ждать нечего

  // 7) широковещание
  fresh();
  {
    N n; n.begin(1);
    g_radio.ackOk = false;                                                  // на широковещание подтверждений нет, это не ошибка
    CHECK(n.broadcast("all") && n.wait());
    CHECK(g_radio.sent.size() == 1 && g_radio.isBcast(g_radio.sent[0].mac) && n.peerCount() == 1);
    CHECK(n.broadcast((const uint8_t *)"ab", 2) && n.wait() && n.peerCount() == 1);
    Sample s = {1, 2.0f};
    CHECK(n.broadcastStruct(s) && n.wait() && g_radio.sent[2].data.size() == sizeof(Sample));
    CHECK(n.failedCount() == 0 && n.deliveredCount() == 3);
    CHECK(!n.broadcast("") && n.error() == N::ERR_EMPTY);
  }

  // 8) сбои радио
  fresh();
  {
    N n; n.begin(1);
    g_radio.noMem = true;
#if defined(ESP32)
    CHECK(!n.send(SA, "x") && n.error() == N::ERR_BUSY);
#else
    CHECK(!n.send(SA, "x") && n.error() == N::ERR_SEND);
#endif
    CHECK(!n.sending() && n.sentCount() == 0);
    g_radio.noMem = false; g_radio.forceRc = 2;
    CHECK(!n.send(SA, "x") && n.error() == N::ERR_SEND);
    g_radio.forceRc = 0;
    CHECK(n.send(SA, "x") && n.wait());                                     // после сбоя снова работает
  }

  // 9) приём
  fresh();
  {
    N n; n.begin(1); uint8_t buf[255];
    CHECK(n.available() == 0 && n.length() == 0 && n.read(buf, 255) == 0 && n.readText().length() == 0);
    uint8_t d[3] = {1, 2, 3};
    g_radio.deliver(MA, "one"); g_radio.deliver(MB, "two2"); g_radio.deliver(MA, d, 3);
    CHECK(n.available() == 3 && n.length() == 3);
    CHECK(n.read(buf, 255) == 3 && memcmp(buf, "one", 3) == 0);
    CHECK(memcmp(n.lastMac(), MA, 6) == 0 && n.lastMacString() == "AA:BB:CC:DD:EE:01");
    CHECK(n.length() == 4 && n.readText() == "two2" && n.lastMacString() == "AA:BB:CC:DD:EE:02");
    uint8_t small[2];
    CHECK(n.read(small, 2) == 2 && small[0] == 1 && small[1] == 2 && n.available() == 0);   // короткий буфер: берём сколько влезет
    CHECK(n.receivedCount() == 3 && n.droppedCount() == 0);
    Sample s = {5, 2.5f}, r = {0, 0};
    g_radio.deliver(MB, (const uint8_t *)&s, sizeof(s));
    CHECK(n.readStruct(r) && r.a == 5 && r.b == 2.5f && n.error() == N::ERR_OK);
    g_radio.deliver(MB, "abc");
    CHECK(!n.readStruct(r) && n.error() == N::ERR_SIZE && n.available() == 0);   // неподходящее сообщение забрано
    CHECK(!n.readStruct(r) && n.error() == N::ERR_OK);                           // пусто: не ошибка
    uint8_t full[250]; for (int i = 0; i < 250; i++) full[i] = (uint8_t)i;
    g_radio.deliver(MA, full, 250);
    CHECK(n.length() == 250 && n.read(buf, 255) == 250 && buf[249] == 249);
    g_radio.deliver(MA, d, 0); g_radio.deliver(MA, buf, 251);                   // пустые и слишком длинные теряются
    CHECK(n.available() == 0 && n.droppedCount() == 2);
  }

  // 10) очередь: переполнение сохраняет старые, потом работает дальше; обход по кругу
  fresh();
  {
    N n; n.begin(1); const int Q = ALASH_ESPNOW_QUEUE; uint8_t b;
    for (int i = 0; i < Q + 3; i++) { b = (uint8_t)i; g_radio.deliver(MA, &b, 1); }
    CHECK(n.available() == Q && n.droppedCount() == 3 && n.receivedCount() == (uint32_t)Q);
    bool order = true;
    for (int i = 0; i < Q; i++) { uint8_t r = 255; n.read(&r, 1); if (r != i) order = false; }
    CHECK(order && n.available() == 0);
    b = 77; g_radio.deliver(MA, &b, 1);
    CHECK(n.available() == 1);
    n.read(&b, 1);
    order = true;
    for (int round = 0; round < 100; round++) {
      int k = 1 + round % Q;
      for (int i = 0; i < k; i++) { uint8_t v = (uint8_t)(round + i); g_radio.deliver(MA, &v, 1); }
      for (int i = 0; i < k; i++) { uint8_t r = 0; n.read(&r, 1); if (r != (uint8_t)(round + i)) order = false; }
    }
    CHECK(order && n.available() == 0 && n.droppedCount() == 3);
  }

  // 11) onReceive вызывается только из update()
  fresh(); cbCalls = 0;
  {
    N n; n.begin(1);
    n.onReceive(onMsg);
    g_radio.deliver(MA, "x1"); g_radio.deliver(MB, "yy");
    CHECK(cbCalls == 0 && n.available() == 2);
    n.update();
    CHECK(cbCalls == 2 && n.available() == 0 && cbText == "yy" && cbLen == 2 && memcmp(cbMac, MB, 6) == 0 && n.lastMacString() == "AA:BB:CC:DD:EE:02");
  }
  fresh();
  { N n; n.begin(1); g_radio.deliver(MA, "x"); n.update(); CHECK(n.available() == 1); }   // без обратного вызова очередь остаётся

  // 12) шифрование
  fresh();
  {
    N n; n.begin(1);
    CHECK(!n.setKey("short") && n.error() == N::ERR_BAD_KEY);
    CHECK(!n.setKey(nullptr) && n.error() == N::ERR_BAD_KEY);
    CHECK(n.setKey("0123456789abcdef") && g_radio.pmkSet && memcmp(g_radio.pmk, "0123456789abcdef", 16) == 0);
  }

  // 13) end() и повторный begin()
  fresh();
  {
    N n; n.begin(1); n.addPeer(SA); g_radio.deliver(MA, "q");
    n.end();
    CHECK(g_radio.deinitCalls == 1 && !g_radio.inited);
    CHECK(!n.send(SA, "x") && n.error() == N::ERR_NOT_BEGUN);
    CHECK(n.begin(1) && n.peerCount() == 0 && g_radio.peers.empty());
    CHECK(n.send(SA, "x") && n.peerCount() == 1);
    g_radio.deliver(MA, "q2");
    CHECK(n.begin(1) && n.available() == 0 && g_radio.deinitCalls == 2);       // повторный begin начинает заново
  }

  // 14) тексты ошибок на трёх языках
  for (int code = 1; code <= 12; code++) {
    fresh();
    N n; uint8_t m[6]; uint8_t big[251] = {0};
    switch (code) {
      case N::ERR_NOT_BEGUN: n.send(SA, "x"); break;
      case N::ERR_INIT: g_radio.initOk = false; n.begin(); break;
      case N::ERR_CHANNEL: n.begin(14); break;
      case N::ERR_BAD_MAC: n.begin(); n.addPeer("x"); break;
      case N::ERR_BAD_KEY: n.begin(); n.setKey("x"); break;
      case N::ERR_EMPTY: n.begin(); n.send(SA, ""); break;
      case N::ERR_TOO_LONG: n.begin(); n.send(SA, big, 251); break;
      case N::ERR_PEERS_FULL: n.begin(); for (int i = 0; i < ALASH_ESPNOW_MAX_PEERS + 1; i++) { mk(i, m); n.addPeer(m); } break;
      case N::ERR_BUSY: n.begin(); g_radio.noMem = true; n.send(SA, "x"); break;
      case N::ERR_SEND: n.begin(); g_radio.forceRc = 2; n.send(SA, "x"); break;
      case N::ERR_NO_DELIVERY: n.begin(); g_radio.ackOk = false; n.send(SA, "x"); n.wait(); break;
      case N::ERR_SIZE: { n.begin(); g_radio.deliver(MA, "abc"); uint32_t t[4]; n.readStruct(t); break; }
    }
    int expect = code;
#if !defined(ESP32)
    if (code == N::ERR_BUSY) expect = N::ERR_SEND;
#endif
    CHECK((int)n.error() == expect);
    n.setLanguage(N::LANG_RU); std::string ru = (const char *)n.errorText();
    n.setLanguage(N::LANG_KK); std::string kk = (const char *)n.errorText();
    n.setLanguage(N::LANG_EN); std::string en = (const char *)n.errorText();
    CHECK(ru.size() > 20 && kk.size() > 20 && en.size() > 20 && ru != kk && kk != en && ru != en);
  }
  { N n; CHECK(std::string((const char *)n.errorText()) == "OK"); }

  // 15) diagnose
  fresh();
  {
    N n; Print p0; n.setLanguage(N::LANG_EN);
    n.diagnose(p0);
    CHECK(has(p0, "begin() was not called"));
    n.begin(6); n.addPeer(SA);
    Print p1; n.diagnose(p1);
    CHECK(has(p1, "24:6F:28:AA:BB:01") && has(p1, "AA:BB:CC:DD:EE:01") && has(p1, "Channel: 6") && has(p1, "Peers: 1"));
    g_radio.ackOk = false; n.send(SA, "x"); n.wait();
    Print p2; n.diagnose(p2);
    CHECK(has(p2, "Nothing arrives") && has(p2, "0 / 1"));
    for (int i = 0; i < ALASH_ESPNOW_QUEUE + 2; i++) g_radio.deliver(MA, "m");
    Print p3; n.diagnose(p3);
    CHECK(has(p3, "Messages are lost"));
    n.setLanguage(N::LANG_RU); Print r; n.diagnose(r);
    n.setLanguage(N::LANG_KK); Print k; n.diagnose(k);
    CHECK(r.log.size() > 100 && k.log.size() > 100 && r.log != k.log && r.log != p3.log && has(r, "Канал") && has(k, "Арна"));
  }

  // 16) приём из другого потока (как в задаче Wi-Fi на ESP32): порядок и целостность
  fresh();
  {
    N n; n.begin(1);
    std::atomic<bool> done(false);
    const uint32_t TOTAL = 20000;
    std::thread prod([&] {
      for (uint32_t i = 0; i < TOTAL; i++) { uint8_t b[4]; memcpy(b, &i, 4); g_radio.deliver(MA, b, 4); if (i % 7 == 0) std::this_thread::yield(); }
      done = true;
    });
    uint32_t last = 0, got = 0; bool first = true, ordered = true, intact = true;
    auto drain = [&] {
      uint8_t b[8]; uint8_t l;
      while ((l = n.read(b, 8)) != 0) { uint32_t v; memcpy(&v, b, 4); if (l != 4) intact = false; if (!first && v <= last) ordered = false; first = false; last = v; got++; }
    };
    while (!done) drain();
    prod.join();
    drain();
    CHECK(ordered && intact);
    CHECK(got + n.droppedCount() == TOTAL && got == n.receivedCount());
    printf("thread test: got %u dropped %u\n", got, (unsigned)n.droppedCount());
  }

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
