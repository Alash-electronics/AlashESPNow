#include "AlashESPNow.h"
#if defined(ESP32)
#include <esp_idf_version.h>
#endif

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))
#define QN (ALASH_ESPNOW_QUEUE + 1)

const uint8_t AlashESPNow::BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static AlashESPNow *g_self = nullptr;      // одна плата = один ESP-NOW, обработчики радио обращаются к нему

// ---- Радио-обработчики (на ESP32 работают в задаче Wi-Fi, поэтому только кладут сообщение в очередь) ----
#if defined(ESP32)
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
static void radioRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) { if (g_self && info) g_self->_onRadioReceive(info->src_addr, data, len); }
#else
static void radioRecv(const uint8_t *mac, const uint8_t *data, int len) { if (g_self) g_self->_onRadioReceive(mac, data, len); }
#endif
#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
static void radioSent(const wifi_tx_info_t *, esp_now_send_status_t st) { if (g_self) g_self->_onRadioSent(st == ESP_NOW_SEND_SUCCESS); }
#else
static void radioSent(const uint8_t *, esp_now_send_status_t st) { if (g_self) g_self->_onRadioSent(st == ESP_NOW_SEND_SUCCESS); }
#endif
#else
static void radioRecv(uint8_t *mac, uint8_t *data, uint8_t len) { if (g_self) g_self->_onRadioReceive(mac, data, len); }
static void radioSent(uint8_t *, uint8_t st) { if (g_self) g_self->_onRadioSent(st == 0); }
#endif

// ---- Обёртки над двумя разными API ------------------------------------------------------------
static bool hwSetChannel(uint8_t ch) {
#if defined(ESP32)
  return esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE) == ESP_OK;
#else
  return wifi_set_channel(ch);
#endif
}

static bool hwInit() {
#if defined(ESP32)
  if (esp_now_init() != ESP_OK) return false;
  esp_now_register_recv_cb(radioRecv);
  esp_now_register_send_cb(radioSent);
  return true;
#else
  if (esp_now_init() != 0) return false;
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(radioRecv);
  esp_now_register_send_cb(radioSent);
  return true;
#endif
}

static void hwDeinit() { esp_now_deinit(); }

static AlashESPNow::Error hwAddPeer(const uint8_t *mac, const char *key) {
#if defined(ESP32)
  esp_now_peer_info_t p;
  memset(&p, 0, sizeof(p));
  memcpy(p.peer_addr, mac, 6);
  p.channel = 0;                                   // 0 = канал, на котором сейчас плата
  p.ifidx = (WiFi.getMode() == WIFI_AP) ? WIFI_IF_AP : WIFI_IF_STA;
  if (key) { p.encrypt = true; memcpy(p.lmk, key, 16); } else p.encrypt = false;
  esp_err_t r = esp_now_add_peer(&p);
  if (r == ESP_OK || r == ESP_ERR_ESPNOW_EXIST) return AlashESPNow::ERR_OK;
  if (r == ESP_ERR_ESPNOW_FULL) return AlashESPNow::ERR_PEERS_FULL;
  if (r == ESP_ERR_ESPNOW_NOT_INIT) return AlashESPNow::ERR_NOT_BEGUN;
  return AlashESPNow::ERR_INIT;
#else
  uint8_t m[6];
  memcpy(m, mac, 6);
  int r = esp_now_add_peer(m, ESP_NOW_ROLE_COMBO, WiFi.channel(), key ? (uint8_t *)key : nullptr, key ? 16 : 0);
  return r == 0 ? AlashESPNow::ERR_OK : AlashESPNow::ERR_PEERS_FULL;
#endif
}

static void hwDelPeer(const uint8_t *mac) {
#if defined(ESP32)
  esp_now_del_peer(mac);
#else
  uint8_t m[6];
  memcpy(m, mac, 6);
  esp_now_del_peer(m);
#endif
}

static AlashESPNow::Error hwSend(const uint8_t *mac, const uint8_t *data, uint8_t len) {
#if defined(ESP32)
  esp_err_t r = esp_now_send(mac, data, len);
  if (r == ESP_OK) return AlashESPNow::ERR_OK;
  if (r == ESP_ERR_ESPNOW_NO_MEM) return AlashESPNow::ERR_BUSY;
  if (r == ESP_ERR_ESPNOW_NOT_INIT) return AlashESPNow::ERR_NOT_BEGUN;
  return AlashESPNow::ERR_SEND;
#else
  uint8_t m[6];
  memcpy(m, mac, 6);
  int r = esp_now_send(m, (uint8_t *)data, len);
  return r == 0 ? AlashESPNow::ERR_OK : AlashESPNow::ERR_SEND;
#endif
}

// ---- MAC-адреса ---------------------------------------------------------------------------------
static int hexVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

bool AlashESPNow::parseMac(const char *text, uint8_t out[6]) {
  if (!text || strlen(text) != 17) return false;
  uint8_t tmp[6];
  for (int i = 0; i < 6; i++) {
    int hi = hexVal(text[i * 3]), lo = hexVal(text[i * 3 + 1]);
    if (hi < 0 || lo < 0) return false;
    if (i < 5 && text[i * 3 + 2] != ':' && text[i * 3 + 2] != '-') return false;
    tmp[i] = (uint8_t)(hi * 16 + lo);
  }
  memcpy(out, tmp, 6);
  return true;
}

String AlashESPNow::macToString(const uint8_t *mac) {
  char b[18];
  snprintf(b, sizeof(b), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(b);
}

String AlashESPNow::lastMacString() const { return macToString(_lastMac); }

// ---- Запуск -------------------------------------------------------------------------------------
AlashESPNow::AlashESPNow() { memset(_q, 0, sizeof(_q)); memset(_peers, 0, sizeof(_peers)); }

bool AlashESPNow::begin(uint8_t ch) {
  _err = ERR_OK;
  if (ch > 13) return fail(ERR_CHANNEL);
  if (_begun) end();
  if (WiFi.getMode() == WIFI_OFF) WiFi.mode(WIFI_STA);
  if (ch) {
    if (WiFi.status() == WL_CONNECTED) {                     // канал задаёт роутер, свой выбрать нельзя
      if (WiFi.channel() != ch) return fail(ERR_CHANNEL);
    } else if (!hwSetChannel(ch)) return fail(ERR_CHANNEL);
  }
  g_self = this;
  if (!hwInit()) { g_self = nullptr; return fail(ERR_INIT); }
  _begun = true;
  _head = _tail = 0;
  _npeers = 0;
  _sendState = 0;
  return true;
}

void AlashESPNow::end() {
  if (_begun) hwDeinit();
  _begun = false;
  _npeers = 0;
  if (g_self == this) g_self = nullptr;
}

bool AlashESPNow::setKey(const char *key) {
  if (!_begun) return fail(ERR_NOT_BEGUN);
  if (!key || strlen(key) != 16) return fail(ERR_BAD_KEY);
#if defined(ESP32)
  if (esp_now_set_pmk((const uint8_t *)key) != ESP_OK) return fail(ERR_INIT);
#else
  if (esp_now_set_kok((uint8_t *)key, 16) != 0) return fail(ERR_INIT);
#endif
  _encrypted = true;
  return true;
}

uint8_t AlashESPNow::channel() const { return (uint8_t)WiFi.channel(); }

String AlashESPNow::myMac() const { return WiFi.macAddress(); }

// ---- Собеседники --------------------------------------------------------------------------------
int AlashESPNow::findPeer(const uint8_t *mac) const {
  for (uint8_t i = 0; i < _npeers; i++) if (memcmp(_peers[i], mac, 6) == 0) return i;
  return -1;
}

bool AlashESPNow::ensurePeer(const uint8_t *mac, const char *key) {
  if (key && strlen(key) != 16) return fail(ERR_BAD_KEY);
  int i = findPeer(mac);
  if (i >= 0) {
    if (!key) return true;
    hwDelPeer(mac);                                          // новый ключ для известного собеседника
    Error e = hwAddPeer(mac, key);
    if (e != ERR_OK) { memmove(_peers[i], _peers[i + 1], (size_t)(_npeers - i - 1) * 6); _npeers--; return fail(e); }
    return true;
  }
  if (_npeers >= ALASH_ESPNOW_MAX_PEERS) return fail(ERR_PEERS_FULL);
  Error e = hwAddPeer(mac, key);
  if (e != ERR_OK) return fail(e);
  memcpy(_peers[_npeers++], mac, 6);
  return true;
}

bool AlashESPNow::addPeer(const uint8_t *mac, const char *key) {
  _err = ERR_OK;
  if (!_begun) return fail(ERR_NOT_BEGUN);
  if (!mac) return fail(ERR_BAD_MAC);
  return ensurePeer(mac, key);
}

bool AlashESPNow::addPeer(const char *mac, const char *key) {
  uint8_t m[6];
  _err = ERR_OK;
  if (!parseMac(mac, m)) return fail(ERR_BAD_MAC);
  return addPeer(m, key);
}

bool AlashESPNow::removePeer(const uint8_t *mac) {
  _err = ERR_OK;
  if (!_begun) return fail(ERR_NOT_BEGUN);
  if (!mac) return fail(ERR_BAD_MAC);
  int i = findPeer(mac);
  if (i < 0) return false;
  hwDelPeer(mac);
  memmove(_peers[i], _peers[i + 1], (size_t)(_npeers - i - 1) * 6);
  _npeers--;
  return true;
}

bool AlashESPNow::removePeer(const char *mac) {
  uint8_t m[6];
  _err = ERR_OK;
  if (!parseMac(mac, m)) return fail(ERR_BAD_MAC);
  return removePeer(m);
}

static uint8_t textLen(const char *t) { size_t n = t ? strlen(t) : 0; return n > 255 ? 255 : (uint8_t)n; }   // длиннее 250 всё равно отклонит doSend

// ---- Отправка -----------------------------------------------------------------------------------
bool AlashESPNow::doSend(const uint8_t *mac, const uint8_t *data, uint8_t len) {
  _err = ERR_OK;
  if (!_begun) return fail(ERR_NOT_BEGUN);
  if (!mac) return fail(ERR_BAD_MAC);
  if (!data || len == 0) return fail(ERR_EMPTY);
  if (len > ALASH_ESPNOW_MAX_LEN) return fail(ERR_TOO_LONG);
  if (!ensurePeer(mac, nullptr)) return false;
  _sendState = 1;
  Error e = hwSend(mac, data, len);
  if (e != ERR_OK) { _sendState = 0; return fail(e); }
  _sent++;
  return true;
}

bool AlashESPNow::send(const uint8_t *mac, const uint8_t *data, uint8_t len) { return doSend(mac, data, len); }

bool AlashESPNow::send(const char *mac, const uint8_t *data, uint8_t len) {
  uint8_t m[6];
  _err = ERR_OK;
  if (!parseMac(mac, m)) return fail(ERR_BAD_MAC);
  return doSend(m, data, len);
}

bool AlashESPNow::send(const uint8_t *mac, const char *text) { return doSend(mac, (const uint8_t *)text, textLen(text)); }

bool AlashESPNow::send(const char *mac, const char *text) { return send(mac, (const uint8_t *)text, textLen(text)); }

bool AlashESPNow::broadcast(const uint8_t *data, uint8_t len) { return doSend(BROADCAST_MAC, data, len); }

bool AlashESPNow::broadcast(const char *text) { return doSend(BROADCAST_MAC, (const uint8_t *)text, textLen(text)); }

bool AlashESPNow::wait(uint16_t timeoutMs) {
  uint32_t t0 = millis();
  while (_sendState == 1 && (uint32_t)(millis() - t0) < timeoutMs) delay(1);
  if (_sendState == 2) return true;
  if (_sendState == 0) return false;                         // ничего не отправляли
  _err = ERR_NO_DELIVERY;
  return false;
}

void AlashESPNow::_onRadioSent(bool ok) {
  if (ok) _delivered++; else _failed++;
  _sendState = ok ? 2 : 3;
}

// ---- Приём --------------------------------------------------------------------------------------
void AlashESPNow::_onRadioReceive(const uint8_t *mac, const uint8_t *data, int len) {
  if (!mac || !data || len <= 0 || len > ALASH_ESPNOW_MAX_LEN) { _dropped++; return; }
  uint8_t h = _head, next = (uint8_t)((h + 1) % QN);
  if (next == _tail) { _dropped++; return; }                 // очередь полна: новое сообщение теряем
  memcpy(_q[h].mac, mac, 6);
  _q[h].len = (uint8_t)len;
  memcpy(_q[h].data, data, (size_t)len);
  _q[h].data[len] = 0;
  __sync_synchronize();
  _head = next;
  _received++;
}

bool AlashESPNow::pop(Msg &m) {
  uint8_t t = _tail;
  if (t == _head) return false;
  m = _q[t];
  __sync_synchronize();
  _tail = (uint8_t)((t + 1) % QN);
  memcpy(_lastMac, m.mac, 6);
  return true;
}

uint8_t AlashESPNow::available() const { return (uint8_t)((_head + QN - _tail) % QN); }

uint8_t AlashESPNow::length() const { return _head == _tail ? 0 : _q[_tail].len; }

uint8_t AlashESPNow::read(uint8_t *buf, uint8_t maxLen) {
  Msg m;
  if (!buf || !pop(m)) return 0;
  uint8_t n = m.len < maxLen ? m.len : maxLen;
  memcpy(buf, m.data, n);
  return n;
}

String AlashESPNow::readText() {
  Msg m;
  if (!pop(m)) return String();
  return String((const char *)m.data);
}

bool AlashESPNow::readExact(uint8_t *buf, uint8_t len) {
  Msg m;
  _err = ERR_OK;
  if (!pop(m)) return false;
  if (m.len != len) return fail(ERR_SIZE);                   // сообщение уже забрано из очереди
  memcpy(buf, m.data, len);
  return true;
}

void AlashESPNow::update() {
  if (!_onRecv) return;
  Msg m;
  while (pop(m)) _onRecv(m.mac, m.data, m.len);
}

// ---- Тексты ошибок и диагностика ----------------------------------------------------------------
const __FlashStringHelper *AlashESPNow::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NOT_BEGUN:
      return T("Сначала вызовите begin() в setup().", "Алдымен setup() ішінде begin() шақырыңыз.", "Call begin() in setup() first.");
    case ERR_INIT:
      return T("ESP-NOW не запустился. Перезагрузите плату; если повторяется, выберите другую схему питания / плату в меню.", "ESP-NOW іске қосылмады. Тақтаны қайта қосыңыз; қайталанса, қоректі тексеріңіз.",
               "ESP-NOW did not start. Restart the board; if it repeats, check the power supply and the board choice.");
    case ERR_CHANNEL:
      return T("Канал должен быть от 1 до 13. Если плата подключена к роутеру, канал задаёт роутер: вызовите begin() до WiFi.begin() или без номера канала.",
               "Арна 1-ден 13-ке дейін болуы керек. Тақта роутерге қосылған болса, арнаны роутер береді: begin() WiFi.begin()-ге дейін не арна нөмірінсіз шақырыңыз.",
               "The channel must be 1 to 13. If the board is connected to a router the router sets the channel: call begin() before WiFi.begin() or without a channel number.");
    case ERR_BAD_MAC:
      return T("MAC-адрес записан неверно. Нужен вид AA:BB:CC:DD:EE:FF (шесть пар символов 0-9, A-F).", "MAC-мекенжай қате жазылған. AA:BB:CC:DD:EE:FF түрінде болуы керек (алты жұп 0-9, A-F).",
               "The MAC address is wrong. Use the form AA:BB:CC:DD:EE:FF (six pairs of 0-9, A-F).");
    case ERR_BAD_KEY:
      return T("Ключ шифрования должен быть ровно 16 символов, одинаковый на обеих платах.", "Шифрлау кілті дәл 16 таңба болуы керек, екі тақтада бірдей.", "The encryption key must be exactly 16 characters, the same on both boards.");
    case ERR_EMPTY:
      return T("Пустое сообщение: нечего отправлять.", "Бос хабар: жіберетін ештеңе жоқ.", "Empty message: nothing to send.");
    case ERR_TOO_LONG:
      return T("Сообщение длиннее 250 байт. Разбейте его на части.", "Хабар 250 байттан ұзын. Бөліктерге бөліңіз.", "The message is longer than 250 bytes. Split it into parts.");
    case ERR_PEERS_FULL:
      return T("Список собеседников полон (до 20, с шифрованием до 6 на ESP32). Уберите ненужных через removePeer() или пользуйтесь broadcast().",
               "Серіктестер тізімі толы (20-ға дейін, ESP32-де шифрлаумен 6-ға дейін). removePeer() қолданыңыз не broadcast() жіберіңіз.",
               "The peer list is full (up to 20, up to 6 with encryption on ESP32). Remove some with removePeer() or use broadcast().");
    case ERR_BUSY:
      return T("Отправка слишком частая: подождите несколько миллисекунд между сообщениями (delay(10)) или вызовите wait().", "Жіберу тым жиі: хабарлар арасында бірнеше миллисекунд күтіңіз (delay(10)) не wait() шақырыңыз.",
               "Sending too often: wait a few milliseconds between messages (delay(10)) or call wait().");
    case ERR_SEND:
      return T("Радио не приняло сообщение. Проверьте, что begin() выполнен и Wi-Fi не выключен.", "Радио хабарды қабылдамады. begin() орындалғанын және Wi-Fi өшірілмегенін тексеріңіз.",
               "The radio did not accept the message. Check that begin() was done and Wi-Fi is not turned off.");
    case ERR_NO_DELIVERY:
      return T("Получатель не подтвердил приём. Проверьте: MAC получателя верный, он включён и у него вызван begin(), канал одинаковый, ключ одинаковый, плата рядом.",
               "Алушы қабылдағанын растамады. Тексеріңіз: алушы MAC дұрыс, ол қосулы және begin() шақырылған, арна бірдей, кілт бірдей, тақта жақын.",
               "The receiver did not confirm. Check: the receiver MAC is right, it is on and called begin(), the channel is the same, the key is the same, the board is close.");
    case ERR_SIZE:
      return T("Размер принятого сообщения не равен размеру структуры. На обеих платах структура должна быть одинаковой.", "Қабылданған хабар өлшемі құрылым өлшеміне тең емес. Екі тақтада құрылым бірдей болуы керек.",
               "The received message size does not match the struct size. The struct must be identical on both boards.");
  }
  return F("?");
}

void AlashESPNow::diagnose(Print &out) {
  out.println(T("AlashESPNow: диагностика", "AlashESPNow: диагностика", "AlashESPNow: diagnostics"));
  out.print(T("Моя плата (MAC): ", "Менің тақтам (MAC): ", "This board (MAC): "));
  out.println(myMac());
  out.print(T("Канал: ", "Арна: ", "Channel: "));
  out.println((int)channel());
  out.print(T("Роутер: ", "Роутер: ", "Router: "));
  out.println(WiFi.status() == WL_CONNECTED ? T("подключён (канал задаёт он)", "қосылған (арнаны сол береді)", "connected (it sets the channel)") : T("нет", "жоқ", "no"));
  if (!_begun) {
    out.println(T("begin() не вызван или завершился ошибкой.", "begin() шақырылмаған не қатемен аяқталған.", "begin() was not called or failed."));
    if (_err != ERR_OK) out.println(errorText());
    return;
  }
  out.print(T("Собеседников: ", "Серіктестер: ", "Peers: "));
  out.println((int)_npeers);
  for (uint8_t i = 0; i < _npeers; i++) { out.print("  "); out.println(macToString(_peers[i])); }
  out.print(T("Шифрование: ", "Шифрлау: ", "Encryption: "));
  out.println(_encrypted ? T("включено", "қосулы", "on") : T("нет", "жоқ", "off"));
  out.print(T("Отправлено / доставлено / не доставлено: ", "Жіберілді / жеткізілді / жеткізілмеді: ", "Sent / delivered / failed: "));
  out.print((int)_sent); out.print(" / "); out.print((int)_delivered); out.print(" / "); out.println((int)_failed);
  out.print(T("Принято / потеряно: ", "Қабылданды / жоғалды: ", "Received / dropped: "));
  out.print((int)_received); out.print(" / "); out.println((int)_dropped);
  if (_err != ERR_OK) out.println(errorText());
  if (_failed > 0 && _delivered == 0)
    out.println(T("Ничего не доходит: проверьте MAC получателя (он печатается строкой «Моя плата» на его экране Serial), одинаковый канал и что у получателя вызван begin().",
                  "Ештеңе жетпейді: алушы MAC-ын (оның Serial-ында «Менің тақтам» жолы), бірдей арнаны және алушыда begin() бар екенін тексеріңіз.",
                  "Nothing arrives: check the receiver MAC (the line \"This board\" in its Serial), the same channel and that the receiver called begin()."));
  if (_dropped > 0)
    out.println(T("Сообщения теряются: читайте очередь чаще (read / update в loop) или увеличьте ALASH_ESPNOW_QUEUE.", "Хабарлар жоғалады: кезекті жиі оқыңыз (loop ішінде read / update) не ALASH_ESPNOW_QUEUE үлкейтіңіз.",
                  "Messages are lost: read the queue more often (read / update in loop) or raise ALASH_ESPNOW_QUEUE."));
  out.println(T("Все платы должны быть на одном канале. Дальность около 100 м на открытом месте.", "Барлық тақталар бір арнада болуы керек. Ашық жерде қашықтығы шамамен 100 м.",
                "All boards must use the same channel. Range is about 100 m in the open."));
}
