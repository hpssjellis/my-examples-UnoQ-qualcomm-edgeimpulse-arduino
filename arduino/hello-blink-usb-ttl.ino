// -------------------------------------------------------------------
// UNO Q Serial1 Communication (D0/D1) - Uses 3.3V Logic
// WARNING: The external TTL adapter MUST be set to 3.3V mode.
// WIRING: UNO Q D1 (TX) -> Adapter RX | UNO Q D0 (RX) <- Adapter TX | GND -> GND
// -------------------------------------------------------------------

/**
 * Arduino Sketch: Zephyr-Style Serial Echo and Non-Blocking Blink
 * * This sketch demonstrates two key features:
 * 1. Non-blocking LED blinking on the built-in LED (LED_BUILTIN).
 * 2. Reading characters from the Serial Monitor and echoing them back.
 * * The non-blocking delay ensures that the serial communication can run
 * simultaneously with the LED blink without interruption.
 */

// Pin definition for the built-in LED on most Arduino boards
const int ledPin = LED4_B;   // 4 LED blink Blue

// Timing variables for the non-blocking delay
unsigned long myPreviousMillis = 0; 
const long myInterval = 1000; // Blink interval in milliseconds (1 second)

// The setup function runs once when the board boots up or resets.
void setup() {
  // Initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);

  // Initialize serial communication at a standard baud rate
  Serial.begin(9600);
  Serial.println("--- Arduino Zephyr-Style Sketch Initialized ---");
  Serial.println("Type a message in the Serial Monitor and press Enter.");
  Serial.println("The built-in LED will blink every 1 second.");
}

// The loop function runs repeatedly forever.
void loop() {
  
  // ----------------------------------------------------
  // 1. Non-Blocking LED Blink Logic
  // ----------------------------------------------------
  
  // Get the current time
  unsigned long myCurrentMillis = millis();

  // Check if the interval has passed since the last state change
  if (myCurrentMillis - myPreviousMillis >= myInterval) {
    // Save the current time as the last time we blinked the LED
    myPreviousMillis = myCurrentMillis;

    // Read the current state of the LED (HIGH or LOW)
    int myLedState = digitalRead(ledPin);

    // If the LED is off, turn it on, and vice-versa
    if (myLedState == LOW) {
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }
  }

  // ----------------------------------------------------
  // 2. Serial Read/Write Echo Logic
  // ----------------------------------------------------

  // Check if any data is available to be read from the serial buffer
  while (Serial.available() > 0) {
    // Read the incoming byte (a single character)
    char myIncomingByte = Serial.read();

    // Print the received byte back to the Serial Monitor (echo)
    Serial.print("You sent: ");
    Serial.print(myIncomingByte);
    Serial.print(" (ASCII: ");
    Serial.print((int)myIncomingByte);
    Serial.println(")");
  }
}
