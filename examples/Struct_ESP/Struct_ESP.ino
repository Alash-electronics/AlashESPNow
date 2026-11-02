// [ru] AlashESPNow: передача набора чисел одной структурой (например, показания датчиков). Один и тот же скетч для обеих плат:
//      плата с кнопкой BOOT (GPIO 0) удерживается при включении = передатчик, иначе приёмник. Структура должна быть одинаковой на обеих платах.
// [kk] AlashESPNow: сандар жиынын бір құрылыммен жіберу (мысалы, датчик көрсеткіштері). Екі тақтаға да бір скетч:
//      қосқанда BOOT батырмасы (GPIO 0) ұсталған тақта = таратқыш, әйтпесе қабылдағыш. Құрылым екі тақтада бірдей болуы керек.
// [en] AlashESPNow: sending a set of numbers as one struct (for example sensor readings). The same sketch for both boards:
//      the board with the BOOT button (GPIO 0) held at power-on = the transmitter, otherwise the receiver. The struct must be identical on both boards.
#include <AlashESPNow.h>

struct Reading {
  int16_t temperature10;    // температура в десятых долях градуса
  uint8_t humidity;
  uint32_t uptimeSec;
};

const char *RECEIVER = "AA:BB:CC:DD:EE:FF";   // MAC приёмника
const int MODE_PIN = 0;

AlashESPNow now;
bool sender;

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(MODE_PIN, INPUT_PULLUP);
  sender = (digitalRead(MODE_PIN) == LOW);
  if (!now.begin()) {
    Serial.println(now.errorText());
    return;
  }
  Serial.print(sender ? "Передатчик, " : "Приёмник, ");
  Serial.print("MAC: ");
  Serial.println(now.myMac());
}

void loop() {
  if (sender) {
    Reading r = { 235, 48, millis() / 1000 };      // вместо чисел подставьте показания датчиков
    if (now.sendStruct(RECEIVER, r) && now.wait()) Serial.println("Отправлено");
    else Serial.println(now.errorText());
    delay(2000);
  } else {
    Reading r;
    if (now.readStruct(r)) {
      Serial.print("Температура: ");
      Serial.print(r.temperature10 / 10.0, 1);
      Serial.print(", влажность: ");
      Serial.print(r.humidity);
      Serial.print("%, работает секунд: ");
      Serial.println(r.uptimeSec);
    } else if (now.error() == AlashESPNow::ERR_SIZE) {
      Serial.println(now.errorText());
    }
  }
}
