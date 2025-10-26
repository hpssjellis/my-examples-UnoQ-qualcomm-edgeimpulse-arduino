// You might need to install the ArduinoGraphics library via the Library Manager.
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>

// Create a descriptive LED Matrix object
ArduinoLEDMatrix myLedMatrix;

void setup() {
  // Initialize the LED matrix
  myLedMatrix.begin();
  
  // Set the default color for the text (0xFFFFFF for white/bright blue)
  // The UNO Q matrix is blue, so the color argument mostly controls brightness/shade.
  // The coordinates (0, 0) set the start location for the text print.
  myLedMatrix.beginText(0, 0, 0xFFFFFF);
  
  // The printText function is what places the full string into the buffer.
  myLedMatrix.printText("Hello World"); 
}

void loop() {
  // Use endText() with SCROLL_LEFT to activate the continuous scrolling animation.
  // The scroll speed is often controlled by how quickly loop() executes 
  // and the internal mechanism of the library. 
  // For a smooth scroll, you might adjust the delay below if needed, 
  // but the library often handles the frame rate automatically for smooth scrolling.
  myLedMatrix.endText(SCROLL_LEFT);
  
  // A small delay is sometimes needed to manage the scroll speed 
  // or prevent the microcontroller from being overwhelmed in a more complex sketch.
  delay(50); // Adjust this value to change the scroll speed
}
