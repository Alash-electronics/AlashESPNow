// [ru] AlashESPNow: связь не работает? Скетч печатает MAC платы, канал, список собеседников, счётчики отправок и подсказки, что проверить.
// [kk] AlashESPNow: байланыс жоқ па? Скетч тақта MAC-ын, арнаны, әңгімелесушілер тізімін, жіберу есептегіштерін және нені тексеру керегін басып шығарады.
// [en] AlashESPNow: no link? The sketch prints the board MAC, the channel, the list of peers, the send counters and tips on what to check.
#include <AlashESPNow.h>

const char *RECEIVER = "AA:BB:CC:DD:EE:FF";   // MAC второй платы

AlashESPNow now;

void setup() {
  Serial.begin(115200);
  delay(500);
  now.setLanguage(AlashESPNow::LANG_RU);     // LANG_KK: қазақша, LANG_EN: English
  now.begin();
  now.addPeer(RECEIVER);
  for (int i = 0; i < 3; i++) {              // пробные отправки, чтобы в отчёте были счётчики
    now.send(RECEIVER, "проверка");
    now.wait();
    delay(200);
  }
  now.diagnose(Serial);
}

void loop() {
}
