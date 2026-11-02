// AlashESPNow: простая связь плат ESP32 / ESP8266 по ESP-NOW (без роутера): send, broadcast, очередь приёма, понятные ошибки.
// Только встроенные ESP-NOW и Wi-Fi. MIT License. Alash Electronics.
#ifndef ALASH_ESPNOW_H
#define ALASH_ESPNOW_H

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#else
#error "AlashESPNow works only on ESP32 and ESP8266"
#endif

#ifndef ALASH_ESPNOW_QUEUE
#define ALASH_ESPNOW_QUEUE 6          // сколько принятых сообщений ждут в очереди (каждое до 250 байт)
#endif
#define ALASH_ESPNOW_MAX_LEN 250      // предел одного сообщения ESP-NOW
#define ALASH_ESPNOW_MAX_PEERS 20     // предел собеседников (с шифрованием на ESP32 только 6)

class AlashESPNow {
 public:
  typedef void (*ReceiveCallback)(const uint8_t *mac, const uint8_t *data, uint8_t len);
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NOT_BEGUN,     // begin() не вызван
    ERR_INIT,          // ESP-NOW не запустился
    ERR_CHANNEL,       // неверный канал или канал занят роутером
    ERR_BAD_MAC,       // MAC-адрес записан неверно
    ERR_BAD_KEY,       // ключ шифрования не 16 символов
    ERR_EMPTY,         // пустое сообщение
    ERR_TOO_LONG,      // сообщение длиннее 250 байт
    ERR_PEERS_FULL,    // список собеседников полон
    ERR_BUSY,          // отправка слишком частая
    ERR_SEND,          // радио не приняло сообщение
    ERR_NO_DELIVERY,   // получатель не подтвердил приём
    ERR_SIZE           // размер принятого сообщения не равен размеру структуры
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  static const uint8_t BROADCAST_MAC[6];       // FF:FF:FF:FF:FF:FF: все платы на канале

  AlashESPNow();
  ~AlashESPNow() { end(); }

  // ---- Запуск --------------------------------------------------------------
  bool begin(uint8_t channel = 0);              // channel 1..13; 0 = оставить текущий. У всех плат канал должен быть один
  void end();
  bool setKey(const char *key16);               // общий ключ (PMK), 16 символов; одинаковый на всех платах
  uint8_t channel() const;
  String myMac() const;                         // MAC этой платы, его вписывают в addPeer / send на другой плате

  // ---- Собеседники ---------------------------------------------------------
  bool addPeer(const char *mac, const char *key = nullptr);          // "AA:BB:CC:DD:EE:FF"; key: 16 символов для шифрования
  bool addPeer(const uint8_t *mac, const char *key = nullptr);
  bool removePeer(const char *mac);
  bool removePeer(const uint8_t *mac);
  uint8_t peerCount() const { return _npeers; }

  // ---- Отправка ------------------------------------------------------------
  bool send(const uint8_t *mac, const uint8_t *data, uint8_t len);
  bool send(const char *mac, const uint8_t *data, uint8_t len);
  bool send(const uint8_t *mac, const char *text);
  bool send(const char *mac, const char *text);
  bool broadcast(const uint8_t *data, uint8_t len);
  bool broadcast(const char *text);
  template <typename T> bool sendStruct(const uint8_t *mac, const T &v) { return send(mac, (const uint8_t *)&v, (uint8_t)sizeof(T)); }
  template <typename T> bool sendStruct(const char *mac, const T &v) { return send(mac, (const uint8_t *)&v, (uint8_t)sizeof(T)); }
  template <typename T> bool broadcastStruct(const T &v) { return broadcast((const uint8_t *)&v, (uint8_t)sizeof(T)); }
  bool wait(uint16_t timeoutMs = 100);          // дождаться результата последней отправки: true = получатель подтвердил
  bool sending() const { return _sendState == 1; }

  // ---- Приём ---------------------------------------------------------------
  void onReceive(ReceiveCallback cb) { _onRecv = cb; }   // вызывается из update(), а не из прерывания
  void update();                                // вызывайте в loop(), если используете onReceive
  uint8_t available() const;                    // сколько сообщений ждёт
  uint8_t length() const;                       // длина первого сообщения в очереди
  uint8_t read(uint8_t *buf, uint8_t maxLen);   // забрать сообщение; возвращает его длину (0 = очередь пуста)
  String readText();                            // забрать сообщение как текст
  template <typename T> bool readStruct(T &v) { return readExact((uint8_t *)&v, (uint8_t)sizeof(T)); }
  const uint8_t *lastMac() const { return _lastMac; }   // от кого последнее прочитанное
  String lastMacString() const;
  static bool parseMac(const char *text, uint8_t out[6]);
  static String macToString(const uint8_t *mac);

  // ---- Состояние и диагностика ---------------------------------------------
  void setLanguage(Language l) { _lang = l; }
  Error error() const { return _err; }
  const __FlashStringHelper *errorText();
  uint32_t sentCount() const { return _sent; }
  uint32_t deliveredCount() const { return _delivered; }
  uint32_t failedCount() const { return _failed; }
  uint32_t receivedCount() const { return _received; }
  uint32_t droppedCount() const { return _dropped; }
  void diagnose(Print &out);

  // вызываются из радио-обработчиков (не для пользователя)
  void _onRadioReceive(const uint8_t *mac, const uint8_t *data, int len);
  void _onRadioSent(bool ok);

 private:
  struct Msg { uint8_t mac[6]; uint8_t len; uint8_t data[ALASH_ESPNOW_MAX_LEN + 1]; };   // +1: ноль в конце для readText()
  Msg _q[ALASH_ESPNOW_QUEUE + 1];
  volatile uint8_t _head = 0, _tail = 0;
  uint8_t _peers[ALASH_ESPNOW_MAX_PEERS][6];
  uint8_t _npeers = 0;
  uint8_t _lastMac[6] = {0, 0, 0, 0, 0, 0};
  volatile uint8_t _sendState = 0;               // 0 нет отправки, 1 ждём, 2 доставлено, 3 не доставлено
  volatile uint32_t _delivered = 0, _failed = 0, _received = 0, _dropped = 0;
  uint32_t _sent = 0;
  bool _begun = false, _encrypted = false;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
  ReceiveCallback _onRecv = nullptr;

  bool fail(Error e) { _err = e; return false; }
  int findPeer(const uint8_t *mac) const;
  bool ensurePeer(const uint8_t *mac, const char *key);   // send() сам добавляет незнакомого получателя
  bool doSend(const uint8_t *mac, const uint8_t *data, uint8_t len);
  bool readExact(uint8_t *buf, uint8_t len);
  bool pop(Msg &m);
};

#endif  // ALASH_ESPNOW_H
