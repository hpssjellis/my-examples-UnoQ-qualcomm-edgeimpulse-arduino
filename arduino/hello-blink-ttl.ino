#include <Arduino.h> // Only needed by https://platformio.org/

void setup() {
  Serial.begin(9600);   // not 115200
  pinMode(LED4_B, OUTPUT);  
}

void loop() {
  Serial.println("Serial print works on the UnoQ USB-TTL TX RX 3.3V. Here is A0 reading: " + String(analogRead(A0)) );
  digitalWrite(LED4_B, LOW);   // internal LED LOW = on for onboard LED
  delay(1000);                      // wait for a second
  digitalWrite(LED4_B, HIGH);  
  delay(3000);               
}
