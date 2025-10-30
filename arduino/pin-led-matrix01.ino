#include <Arduino_LED_Matrix.h>

// --- UNO Q Pin Configuration ---

// Analog Input Pin: A5
// NOTE: You mentioned the signal appeared to be on A4, but we are keeping A5
// for now as per the initial request. You can change 'A5' to 'A4' below if needed.
// All I/O pins, including A5, are 3.3V logic tolerant.
const int myAnalogInputPin = A5;

// LED Output Pin: Set to LED4_B as requested.
// IMPORTANT: LED4_B is assumed to use Active-LOW logic (LOW = ON, HIGH = OFF).
const int myLedPin = LED4_B;

// Threshold for 3.3V detection (out of 1024/4096 ADC reading).
// This value helps reliably distinguish between a floating/GND connection and 3.3V.
const int myHighVoltageThreshold = 512;

// Variable to store the reading from the analog pin.
int myAnalogValue = 0;

// LED Matrix object
Arduino_LED_Matrix matrix;


// --- LED MATRIX PATTERNS (8 rows x 13 columns = 104 elements) ---

// Pattern to display "ON" when A5 is HIGH (LED is ON)
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

// Pattern to display "OFF" when A5 is LOW (LED is OFF)
// Kept "OFF" array the same.
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
  Serial.println("UNO Q LED Control Initialized for LED4_B (Active-LOW logic) and Matrix.");

  // Initialize the LED Matrix
  matrix.begin();
  // Set the brightness (optional, 1 is default)
  matrix.setGrayscaleBits(1);
  // Configure the LED pin as an output.
  pinMode(myLedPin, OUTPUT);
}

void loop() {
  // 1. Read the analog value from the A5 pin.
  myAnalogValue = analogRead(myAnalogInputPin);

  // 2. Check if the analog value is above the defined threshold (A5 is connected to 3.3V).
  if (myAnalogValue > myHighVoltageThreshold) {
    // Input is HIGH (A5 is 3.3V).
    
    // Turn LED4_B ON using LOW signal (Active-LOW).
    digitalWrite(myLedPin, LOW);
    
    // Display "ON" on the LED Matrix.
    matrix.draw(myMatrixOn);
    
    Serial.print("A5 Read: ");
    Serial.print(myAnalogValue);
    Serial.println(" -> Input HIGH, LED ON (Matrix: ON)");
  } else {
    // Input is LOW/Floating.
    
    // Turn LED4_B OFF using HIGH signal (Active-LOW).
    digitalWrite(myLedPin, HIGH);
    
    // Display "OFF" on the LED Matrix.
    matrix.draw(myMatrixOff);
    
    Serial.print("A5 Read: ");
    Serial.print(myAnalogValue);
    Serial.println(" -> Input LOW, LED OFF (Matrix: OFF)");
  }

  // Small delay to prevent the loop from running too quickly.
  delay(100);
}
