#include <Arduino_RouterBridge.h>

void setup() {
  // Initialize the Monitor
  Monitor.begin();
}

void loop() {
  // Transmit the string "Hello UNO Q" followed by a newline character
  Monitor.println("Hello UNO Q");
  delay(1000);
}
