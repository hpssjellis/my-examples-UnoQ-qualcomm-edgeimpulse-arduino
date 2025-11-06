// --- UNO Q Pin Configuration ---

// Analog Input Pin: A5
// NOTE on 3.3V Tolerance: The Arduino Uno Q's STM32U585 Microcontroller (MCU)
// runs at 3.3V. All I/O pins, including A5, are 3.3V logic tolerant.
// Connecting A5 to 3.3V is perfectly safe and within the operating specifications.
const int myAnalogInputPin = A5;

// LED Output Pin: Set to LED4_B as requested.
// IMPORTANT: LED4_B is assumed to use Active-LOW logic (LOW = ON, HIGH = OFF).
const int myLedPin = LED4_B;

// Threshold for 3.3V detection (out of 1024, or 4096 depending on ADC resolution).
// We set a threshold of 512 to reliably detect a HIGH signal.
const int myHighVoltageThreshold = 512;

// Variable to store the reading from the analog pin.
int myAnalogValue = 0;


void setup() {
  // Initialize the serial communication for debugging purposes.
  Serial.begin(9600);
  Serial.println("UNO Q LED Control Initialized for LED4_B (Active-LOW logic).");

  // Configure the LED pin as an output.
  pinMode(myLedPin, OUTPUT);

  // Configure the analog input pin (A5) for input.
  // No specific configuration is usually needed for analog pins, but
  // setting pinMode is good practice if used as digital. We rely on analogRead.
}

void loop() {
  // 1. Read the analog value from the A5 pin.
  myAnalogValue = analogRead(myAnalogInputPin);

  // 2. Check if the analog value is above the defined threshold (A5 is connected to 3.3V).
  if (myAnalogValue > myHighVoltageThreshold) {
    // Input is HIGH (A5 is 3.3V). Turn LED ON using LOW signal (Active-LOW).
    digitalWrite(myLedPin, LOW);
    Serial.print("A5 Read: ");
    Serial.print(myAnalogValue);
    Serial.println(" -> Input HIGH, LED ON (LOW)");
  } else {
    // Input is LOW/Floating. Turn LED OFF using HIGH signal (Active-LOW).
    digitalWrite(myLedPin, HIGH);
    Serial.print("A5 Read: ");
    Serial.print(myAnalogValue);
    Serial.println(" -> Input LOW, LED OFF (HIGH)");
  }

  // Small delay to prevent the loop from running too quickly.
  delay(100);
}
