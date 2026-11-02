// AlashESPNow: рассылка всем платам на канале без MAC-адресов. Загрузите этот скетч на несколько плат:
// каждая раз в 3 секунды говорит "я здесь" и печатает, кого услышала. Подтверждений на рассылку нет.
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
