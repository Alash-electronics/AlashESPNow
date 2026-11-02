// AlashESPNow: приёмник. Запустите его первым: он напечатает свой MAC-адрес, его надо вписать в скетч передатчика (Sender_ESP).
#include <AlashESPNow.h>

AlashESPNow now;

void setup() {
  Serial.begin(115200);
  delay(500);
  now.setLanguage(AlashESPNow::LANG_RU);     // LANG_KK: қазақша, LANG_EN: English
  if (!now.begin()) {
    Serial.println(now.errorText());
    return;
  }
  Serial.print("MAC этой платы: ");
  Serial.println(now.myMac());               // вписать в Sender_ESP
}

void loop() {
  while (now.available()) {
    String text = now.readText();
    Serial.print(now.lastMacString());
    Serial.print(" -> ");
    Serial.println(text);
  }
}
