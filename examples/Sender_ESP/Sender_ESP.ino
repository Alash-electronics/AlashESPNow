// [ru] AlashESPNow: передатчик. Отправляет текст раз в секунду на плату-приёмник (Receiver_ESP) и сообщает, дошло ли.
// [kk] AlashESPNow: таратқыш. Қабылдағыш тақтаға (Receiver_ESP) секунд сайын мәтін жібереді және жеткенін хабарлайды.
// [en] AlashESPNow: the transmitter. Sends text once a second to the receiver board (Receiver_ESP) and reports whether it arrived.
#include <AlashESPNow.h>

const char *RECEIVER = "AA:BB:CC:DD:EE:FF";   // MAC приёмника: его печатает Receiver_ESP при запуске

AlashESPNow now;
int counter = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  now.setLanguage(AlashESPNow::LANG_RU);     // LANG_KK: қазақша, LANG_EN: English
  if (!now.begin()) {
    Serial.println(now.errorText());
    return;
  }
  now.addPeer(RECEIVER);
  if (now.error() != AlashESPNow::ERR_OK) Serial.println(now.errorText());   // например, MAC записан неверно
}

void loop() {
  String text = "Привет #" + String(counter++);
  if (now.send(RECEIVER, text.c_str()) && now.wait()) {
    Serial.println("Доставлено: " + text);
  } else {
    Serial.println(now.errorText());         // нет подтверждения: неверный MAC, приёмник выключен, другой канал
  }
  delay(1000);
}
