#include <Arduino_LED_Matrix.h>

// --- UNO Q Pin Configuration ---

// Digital Input Pin: Pin 3 (a Digital I/O pin)
// Pin 3 is used as a digital input (HIGH/LOW detection).
const int myDigitalInputPin = 3;

// LED Output Pin: Set to LED4_B as requested.
// IMPORTANT: LED4_B is assumed to use Active-LOW logic (LOW = ON, HIGH = OFF).
const int myLedPin = LED4_B;

// NOTE: The voltage threshold is no longer needed since we are using digitalRead.
// const int myHighVoltageThreshold = 512;

// Variable to store the digital reading (HIGH or LOW).
int myDigitalValue = 0;

// LED Matrix object
Arduino_LED_Matrix matrix;


// --- LED MATRIX PATTERNS (8 rows x 13 columns = 104 elements) ---

// Pattern to display "ON" when input is HIGH (LED is ON)
// Redesigned 'N' to have a diagonal for better clarity.
uint8_t myMatrixOn[104] = {
  // O (Cols 2-4)     N (Cols 6-9)
  0,0,0,0,0,0,0,0,0,0,0,0,0, // R0
  0,0,0,0,0,0,0,0,0,0,0,0,0, // R1
  0,0,1,1,1,0,1,0,0,1,0,0,0, // R2: O top bar. N: vertical bar + top-right pixel.
  0,0,1,0,1,0,1,1,0,1,0,0,0, // R3: O sides. N: diagonal line starting.
  0,0,1,0,1,0,1,0,1,1,0,0,0, // R4: O sides. N: middle diagonal pixel.
  0,0,1,0,1,0,1,0,0,1,0,0,0, // R5: O sides. N: vertical bars.
  0,0,1,1,1,0,1,0,0,1,0,0,0, // R6: O bottom bar. N: vertical bars.
  0,0,0,0,0,0,0,0,0,0,0,0,0  // R7
};

// Pattern to display "OFF" when input is LOW (LED is OFF)
uint8_t myMatrixOff[104] = {
  // O        F        F
  0,0,0,0,0,0,0,0,0,0,0,0,0, // R0
  0,0,0,0,0,0,0,0,0,0,0,0,0, // R1
  0,1,1,1,0,1,1,1,0,1,1,1,0, // R2 (Top bar)
  0,1,0,1,0,1,0,0,0,1,0,0,0, // R3
  0,1,0,1,0,1,1,1,0,1,1,1,0, // R4 (Middle bar)
  0,1,0,1,0,1,0,0,0,1,0,0,0, // R5
  0,1,1,1,0,1,0,0,0,1,0,0,0, // R6 (Bottom bar, F's base)
  0,0,0,0,0,0,0,0,0,0,0,0,0  // R7
};


void setup() {
  // Initialize the serial communication for debugging purposes.
  Serial.begin(9600);
  Serial.println("UNO Q LED Control Initialized for Pin 3 Digital Input and LED4_B (Active-LOW logic).");

  // Initialize the LED Matrix
  matrix.begin();
  // Set the brightness (optional, 1 is default)
  matrix.setGrayscaleBits(1);
  
  // Configure the LED pin as an output.
  pinMode(myLedPin, OUTPUT);
  
  // Configure the input pin as an input.
  pinMode(myDigitalInputPin, INPUT);
}

void loop() {
  // 1. Read the digital state (HIGH/LOW) from the Pin 3.
  myDigitalValue = digitalRead(myDigitalInputPin);

  // 2. Check if the input value is HIGH (connected to 3.3V).
  if (myDigitalValue == HIGH) {
    // Input is HIGH (Pin 3 is 3.3V).
    
    // Turn LED4_B ON using LOW signal (Active-LOW).
    digitalWrite(myLedPin, LOW);
    
    // Display "ON" on the LED Matrix.
    matrix.draw(myMatrixOn);
    
    Serial.print("Pin 3 Read: HIGH (");
    Serial.print(myDigitalValue);
    Serial.println(") -> LED ON (Matrix: ON)");
  } else {
    // Input is LOW (Pin 3 is GND or floating).
    
    // Turn LED4_B OFF using HIGH signal (Active-LOW).
    digitalWrite(myLedPin, HIGH);
    
    // Display "OFF" on the LED Matrix.
    matrix.draw(myMatrixOff);
    
    Serial.print("Pin 3 Read: LOW (");
    Serial.print(myDigitalValue);
    Serial.println(") -> LED OFF (Matrix: OFF)");
  }

  // Small delay to prevent the loop from running too quickly.
  delay(100);
}
