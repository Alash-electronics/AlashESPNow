// [ru] AlashESPNow: рассылка всем платам на канале без MAC-адресов. Загрузите этот скетч на несколько плат:
//      каждая раз в 3 секунды говорит "я здесь" и печатает, кого услышала. Подтверждений на рассылку нет.
// [kk] AlashESPNow: MAC-мекенжайларсыз арнадағы барлық тақтаға тарату. Бұл скетчті бірнеше тақтаға жүктеңіз:
//      әрқайсысы 3 секунд сайын «мен осындамын» дейді және кімді естігенін басып шығарады. Таратуға растау жоқ.
// [en] AlashESPNow: broadcast to all boards on the channel without MAC addresses. Upload this sketch to several boards:
//      each says "I am here" every 3 seconds and prints whom it heard. A broadcast has no acknowledgement.
#include <AlashESPNow.h>

AlashESPNow now;
uint32_t lastSend = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  if (!now.begin(1)) {                       // у всех плат один и тот же канал
    Serial.println(now.errorText());
    return;
  }
  Serial.print("Я: ");
  Serial.println(now.myMac());
}

void loop() {
  if (millis() - lastSend > 3000) {
    lastSend = millis();
    now.broadcast("я здесь");
  }
  while (now.available()) {
    String text = now.readText();
    Serial.print("Услышал от ");
    Serial.print(now.lastMacString());
    Serial.print(": ");
    Serial.println(text);
  }
}
