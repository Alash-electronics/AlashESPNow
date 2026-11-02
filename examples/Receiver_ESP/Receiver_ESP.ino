// [ru] AlashESPNow: приёмник. Запустите его первым: он напечатает свой MAC-адрес, его надо вписать в скетч передатчика (Sender_ESP).
// [kk] AlashESPNow: қабылдағыш. Оны бірінші іске қосыңыз: ол өз MAC-мекенжайын басып шығарады, оны таратқыш скетчіне (Sender_ESP) жазу керек.
// [en] AlashESPNow: the receiver. Start it first: it prints its MAC address, which must be written into the transmitter sketch (Sender_ESP).
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
