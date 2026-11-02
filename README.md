# AlashESPNow

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **связи плат ESP32 и ESP8266 напрямую, без роутера** (ESP-NOW): отправили текст или структуру на MAC-адрес, на другой плате прочитали из очереди. Собеседники добавляются сами, доставку подтверждает `wait()`. Один и тот же код для обоих чипов. Только встроенный ESP-NOW. Сделана для уроков и наборов Alash Electronics.

```cpp
#include <AlashESPNow.h>
AlashESPNow now;

void setup() {
  Serial.begin(115200);
  now.begin();
  Serial.println(now.myMac());                 // MAC этой платы: вписать на другой плате
}
void loop() {
  now.send("AA:BB:CC:DD:EE:FF", "привет");   // на другой плате: now.readText()
  if (!now.wait()) Serial.println(now.errorText());
  delay(1000);
}
```

### Что умеет
- **Три действия:** `begin()`, `send(MAC, данные)`, на другой плате `readText()`.
- **Текст, байты и структуры:** `sendStruct(MAC, данные)` и `readStruct(данные)` с проверкой размера.
- **Рассылка всем:** `broadcast()` без MAC-адресов.
- **Подтверждение доставки:** `wait()` говорит, дошло ли сообщение.
- **Очередь приёма** (6 сообщений, меняется `ALASH_ESPNOW_QUEUE`): `available()`, `read()`, `readText()`; или `onReceive()` + `update()`.
- **Один код для ESP32 и ESP8266**, для старых и новых ядер ESP32 (2.x и 3.x).
- **Собеседники добавляются сами**, `addPeer()` нужен только для ключа шифрования.
- **Шифрование:** `setKey()` и ключ собеседника (16 символов).
- **Понятные подсказки** на трёх языках: `errorText()`, `diagnose(Serial)`.
- Дальность около 100 м на открытом месте. Все платы должны быть на одном канале.

### Чем отличается от других библиотек
Мы посмотрели примеры Espressif и Random Nerd Tutorials, ESP-NOW из ядра, esp_now_manager и EspNowSimple:
- В примерах приходится писать свои обработчики для разных ядер: у ESP32 2.x и 3.x они разные, у ESP8266 свои. У нас один код.
- Обработчик приёма работает в задаче Wi-Fi, и ошибка в нём ломает плату. У нас сообщения ложатся в очередь, их читают в `loop()`.
- Собеседников нужно регистрировать вручную, иначе `esp_now_send` молча падает. У нас это автоматически.
- Коды ошибок вида `0x300A` без пояснений. У нас подсказка на русском, казахском и английском.

### Примеры
`Sender_ESP`, `Receiver_ESP`, `Struct_ESP`, `Broadcast_ESP`, `Diagnose_ESP`

### Тесты
Имитация радио ESP-NOW (собеседники, кадры, подтверждения с задержкой, входящие сообщения) проверяет запуск и каналы, MAC-адреса, собеседников и пределы, отправку текста, байтов и структур, доставку и тайм-аут, рассылку, очередь приёма с переполнением и обходом по кругу, onReceive, ключи, повторный begin, все ошибки на трёх языках, diagnose и приём из другого потока (20 000 сообщений). Код собирается для ESP32 core 3.x, core 2.x, IDF 5.5 и ESP8266: `extras/test`. Реальное радио и расстояние тест не проверяет.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**ESP32 және ESP8266 тақталарын роутерсіз тікелей байланыстыруға** (ESP-NOW) арналған қарапайым кітапхана: мәтінді не құрылымды MAC-мекенжайға жібердіңіз, екінші тақтада кезектен оқыдыңыз. Серіктестер өздігінен қосылады, жеткізуді `wait()` растайды. Екі чипке де бір код. Тек кірістірілген ESP-NOW.

```cpp
#include <AlashESPNow.h>
AlashESPNow now;

void setup() {
  Serial.begin(115200);
  now.begin();
  Serial.println(now.myMac());                 // MAC этой платы: вписать на другой плате
}
void loop() {
  now.send("AA:BB:CC:DD:EE:FF", "привет");   // на другой плате: now.readText()
  if (!now.wait()) Serial.println(now.errorText());
  delay(1000);
}
```

### Мүмкіндіктері
- **Үш әрекет:** `begin()`, `send(MAC, деректер)`, екінші тақтада `readText()`.
- **Мәтін, байттар, құрылымдар:** `sendStruct()` және `readStruct()` өлшемді тексерумен.
- **Бәріне тарату:** `broadcast()` MAC-сыз.
- **Жеткізуді растау:** `wait()` хабар жеткенін айтады.
- **Қабылдау кезегі** (6 хабар, `ALASH_ESPNOW_QUEUE`): `available()`, `read()`, `readText()`; не `onReceive()` + `update()`.
- **ESP32 және ESP8266 үшін бір код**, ESP32 ядросының ескі және жаңа нұсқалары.
- **Серіктестер өздігінен қосылады**, `addPeer()` тек шифрлау кілті үшін.
- **Шифрлау:** `setKey()` және серіктес кілті (16 таңба).
- **Түсінікті кеңестер** үш тілде: `errorText()`, `diagnose(Serial)`.
- Ашық жерде қашықтығы шамамен 100 м. Барлық тақталар бір арнада болуы керек.

### Басқа кітапханалардан айырмашылығы
Espressif және Random Nerd Tutorials мысалдарын, ядродағы ESP-NOW-ды, esp_now_manager және EspNowSimple қарадық:
- Мысалдарда әр ядро үшін өз өңдеушіңді жазу керек; бізде бір код.
- Қабылдау өңдеушісі Wi-Fi тапсырмасында жұмыс істейді; бізде хабарлар кезекке түседі, `loop()` ішінде оқылады.
- Серіктестерді қолмен тіркеу керек; бізде автоматты.
- `0x300A` түріндегі қате кодтары; бізде үш тілде кеңес.

### Мысалдар
`Sender_ESP`, `Receiver_ESP`, `Struct_ESP`, `Broadcast_ESP`, `Diagnose_ESP`

### Тесттер
ESP-NOW радио имитациясы іске қосуды, MAC-мекенжайларды, серіктестерді, мәтін, байт және құрылымдарды жіберуді, жеткізуді және тайм-аутты, таратуды, толып кететін қабылдау кезегін, onReceive, кілттерді, барлық қателерді үш тілде, diagnose және басқа ағыннан қабылдауды тексереді. Код ESP32 core 3.x, core 2.x, IDF 5.5 және ESP8266 үшін жиналады: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for **direct communication between ESP32 and ESP8266 boards without a router** (ESP-NOW): send text or a struct to a MAC address and read it from the queue on the other board. Peers are added automatically and `wait()` confirms delivery. The same code works on both chips. Only the built-in ESP-NOW. Made for classes and Alash Electronics kits.

```cpp
#include <AlashESPNow.h>
AlashESPNow now;

void setup() {
  Serial.begin(115200);
  now.begin();
  Serial.println(now.myMac());                 // MAC этой платы: вписать на другой плате
}
void loop() {
  now.send("AA:BB:CC:DD:EE:FF", "привет");   // на другой плате: now.readText()
  if (!now.wait()) Serial.println(now.errorText());
  delay(1000);
}
```

### Features
- **Three steps:** `begin()`, `send(MAC, data)`, and `readText()` on the other board.
- **Text, bytes and structs:** `sendStruct(MAC, data)` and `readStruct(data)` with a size check.
- **Broadcast to all:** `broadcast()` with no MAC addresses.
- **Delivery check:** `wait()` tells whether the message arrived.
- **Receive queue** (6 messages, change `ALASH_ESPNOW_QUEUE`): `available()`, `read()`, `readText()`; or `onReceive()` + `update()`.
- **One code for ESP32 and ESP8266**, old and new ESP32 cores (2.x and 3.x).
- **Peers are added automatically**; `addPeer()` is needed only for an encryption key.
- **Encryption:** `setKey()` and a peer key (16 characters).
- **Clear tips** in three languages: `errorText()`, `diagnose(Serial)`.
- Range is about 100 m in the open. All boards must use the same channel.

### How it differs from other libraries
We looked at the Espressif and Random Nerd Tutorials examples, the ESP-NOW in the core, esp_now_manager and EspNowSimple:
- The examples need your own handlers for each core: ESP32 2.x and 3.x differ, ESP8266 has its own. Ours is one code.
- The receive handler runs in the Wi-Fi task, and a mistake in it crashes the board. Ours puts messages in a queue that you read in `loop()`.
- Peers must be registered by hand or `esp_now_send` fails quietly. Ours does it automatically.
- Error codes like `0x300A` with no explanation. Ours gives a tip in Russian, Kazakh or English.

### Examples
`Sender_ESP`, `Receiver_ESP`, `Struct_ESP`, `Broadcast_ESP`, `Diagnose_ESP`

### Tests
A simulated ESP-NOW radio (peers, frames, delayed acknowledgements, incoming messages) checks start-up and channels, MAC addresses, peers and limits, sending text, bytes and structs, delivery and timeout, broadcast, the receive queue with overflow and wrap-around, onReceive, keys, repeated begin, every error in three languages, diagnose and receiving from another thread (20,000 messages). The code is built for ESP32 core 3.x, core 2.x, IDF 5.5 and ESP8266: `extras/test`. The test does not cover the real radio or distance.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
