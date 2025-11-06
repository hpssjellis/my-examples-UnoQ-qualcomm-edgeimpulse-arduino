#include <Arduino_RouterBridge.h>
#define Serial Monitor

unsigned long t;
String input;

void setup() {
  Serial.begin(9600);
  pinMode(LED3_G, OUTPUT);
  t = millis();
  Serial.println("Serial as the monitor using the python bridge");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      Serial.print(input);
      input = "";
    } else input += c;
  }
  if (millis() - t >= 1000) {
    digitalWrite(LED3_G, !digitalRead(LED3_G));
    t = millis();
    if (digitalRead(LED3_G) == LOW){
      Serial.print("LED3_G is on and LOW, Type and send something");
    }
  }
}
