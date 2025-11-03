// -------------------------------------------------------------------
// UNO Q Serial1 Communication (D0/D1) - Uses 3.3V Logic
// WARNING: The external TTL adapter MUST be set to 3.3V mode.
// WIRING: UNO Q D1 (TX) -> Adapter RX | UNO Q D0 (RX) <- Adapter TX | GND -> GND
// -------------------------------------------------------------------

void setup() {
  // Starts the hardware UART connected to D0/D1 at 115200 baud.
  Serial1.begin(115200); 
}

void loop() {
  // Read and echo back any received data.
  if (Serial1.available()) {
    char myIncomingChar = Serial1.read();
    Serial1.print("Received: ");
    Serial1.println(myIncomingChar);
  }
}
