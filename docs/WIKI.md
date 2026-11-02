# AlashESPNow: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **связи плат ESP32 и ESP8266 напрямую, без роутера** (ESP-NOW): отправили текст или структуру на MAC-адрес, на другой плате прочитали из очереди. Собеседники добавляются сами, доставку подтверждает `wait()`. Один и тот же код для обоих чипов. Только встроенный ESP-NOW. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashESPNow** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin(channel)` | Запустить ESP-NOW (канал 1-13, 0 = текущий) |
| `myMac()` | MAC этой платы |
| `send(MAC, данные)` | Отправить текст или байты |
| `sendStruct(MAC, данные)` | Отправить структуру |
| `broadcast(данные)` | Разослать всем платам на канале |
| `wait(мс)` | Дождаться подтверждения доставки |
| `available() / read() / readText()` | Принятые сообщения |
| `readStruct(данные)` | Принять структуру (размер проверяется) |
| `onReceive(f) + update()` | Обработчик приёма в loop() |
| `addPeer(MAC, ключ) / removePeer(MAC)` | Собеседник и шифрование |
| `setKey(ключ)` | Общий ключ шифрования (16 символов) |
| `diagnose(Serial)` | Самопроверка |

### Если что-то не работает
**Сообщения не доходят**

Запустите Diagnose_ESP. Проверьте: MAC получателя вписан верно (его печатает `myMac()` на приёмнике), у приёмника вызван `begin()`, оба на одном канале, платы рядом.

**Плата подключена к роутеру, а сообщения теряются**

ESP-NOW работает на канале роутера. Платы без роутера тоже окажутся на своём канале, поэтому удобнее всем платам задать `begin(1)` и не подключаться к Wi-Fi, либо подключить все к одному роутеру.

**Ошибка «размер не равен размеру структуры»**

Структура на передатчике и приёмнике должна быть одинаковой: те же поля в том же порядке и тех же типов.

**Как узнать MAC платы**

Запустите Receiver_ESP или `Serial.println(now.myMac())` после `begin()`: адрес вида 24:6F:28:AA:BB:01.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Sender_ESP`, `Receiver_ESP`, `Struct_ESP`, `Broadcast_ESP`, `Diagnose_ESP`

---

## Қазақша

**ESP32 және ESP8266 тақталарын роутерсіз тікелей байланыстыруға** (ESP-NOW) арналған қарапайым кітапхана: мәтінді не құрылымды MAC-мекенжайға жібердіңіз, екінші тақтада кезектен оқыдыңыз. Серіктестер өздігінен қосылады, жеткізуді `wait()` растайды. Екі чипке де бір код. Тек кірістірілген ESP-NOW.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashESPNow** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin(channel)` | ESP-NOW іске қосу (арна 1-13, 0 = ағымдағы) |
| `myMac()` | Осы тақтаның MAC-ы |
| `send(MAC, данные)` | Мәтін не байт жіберу |
| `sendStruct(MAC, данные)` | Құрылым жіберу |
| `broadcast(данные)` | Каналдағы барлық тақтаға тарату |
| `wait(мс)` | Жеткізуді растауды күту |
| `available() / read() / readText()` | Қабылданған хабарлар |
| `readStruct(данные)` | Құрылымды қабылдау (өлшемі тексеріледі) |
| `onReceive(f) + update()` | loop() ішіндегі қабылдау өңдеушісі |
| `addPeer(MAC, ключ) / removePeer(MAC)` | Серіктес және шифрлау |
| `setKey(ключ)` | Ортақ шифрлау кілті (16 таңба) |
| `diagnose(Serial)` | Өзін тексеру |

### Бірдеңе жұмыс істемесе
**Хабарлар жетпейді**

Diagnose_ESP іске қосыңыз. Алушы MAC дұрыс па (`myMac()` алушыда басып шығарады), алушыда `begin()` бар ма, екеуі бір арнада ма, тақталар жақын ба.

**Тақта роутерге қосылған, хабарлар жоғалады**

ESP-NOW роутер арнасында жұмыс істейді. Барлық тақтаға `begin(1)` беріңіз және Wi-Fi-ға қоспаңыз, не бәрін бір роутерге қосыңыз.

**«Өлшемі құрылым өлшеміне тең емес» қатесі**

Құрылым екі тақтада бірдей болуы керек: өрістер, тәртібі және түрлері.

**Тақтаның MAC-ын қалай білуге болады**

Receiver_ESP іске қосыңыз немесе `begin()` кейін `Serial.println(now.myMac())`.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Sender_ESP`, `Receiver_ESP`, `Struct_ESP`, `Broadcast_ESP`, `Diagnose_ESP`

---

## English

A simple library for **direct communication between ESP32 and ESP8266 boards without a router** (ESP-NOW): send text or a struct to a MAC address and read it from the queue on the other board. Peers are added automatically and `wait()` confirms delivery. The same code works on both chips. Only the built-in ESP-NOW. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashESPNow** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `begin(channel)` | Start ESP-NOW (channel 1-13, 0 = current) |
| `myMac()` | MAC of this board |
| `send(MAC, данные)` | Send text or bytes |
| `sendStruct(MAC, данные)` | Send a struct |
| `broadcast(данные)` | Send to every board on the channel |
| `wait(мс)` | Wait for the delivery confirmation |
| `available() / read() / readText()` | Received messages |
| `readStruct(данные)` | Receive a struct (size is checked) |
| `onReceive(f) + update()` | Receive handler run from loop() |
| `addPeer(MAC, ключ) / removePeer(MAC)` | Peer and encryption |
| `setKey(ключ)` | Shared encryption key (16 characters) |
| `diagnose(Serial)` | Self-check |

### If something does not work
**Messages do not arrive**

Run Diagnose_ESP. Check: the receiver MAC is right (`myMac()` prints it on the receiver), the receiver called `begin()`, both use the same channel, the boards are close.

**The board is on a router and messages are lost**

ESP-NOW uses the router's channel. Give every board `begin(1)` and do not join Wi-Fi, or connect all of them to the same router.

**Error: size does not match the struct**

The struct must be identical on both boards: the same fields in the same order with the same types.

**How to find the board MAC**

Run Receiver_ESP or `Serial.println(now.myMac())` after `begin()`: an address like 24:6F:28:AA:BB:01.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Sender_ESP`, `Receiver_ESP`, `Struct_ESP`, `Broadcast_ESP`, `Diagnose_ESP`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashESPNow
