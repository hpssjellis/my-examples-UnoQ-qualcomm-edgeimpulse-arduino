import time
import os
from datetime import datetime

# --- Configuration: Linux File Paths for Onboard LEDs ---
myLed1BluePath = "/sys/class/leds/blue:user/brightness"
myLed2GreenPath = "/sys/class/leds/green:wlan/brightness"

# --- Function to Control LED Brightness ---
def mySetLedBrightness(myLedFilePath, myValue):
    """Writes a brightness value (0-255) to the specified LED file path."""
    try:
        with open(myLedFilePath, "w") as myFile:
            myFile.write(str(myValue))
    except Exception as myError:
        print(f"Error controlling LED at {myLedFilePath}: {myError}. Check permissions.")

# --- LED State Class ---
class LEDController:
    def __init__(self, ledPath, period):
        self.ledPath = ledPath
        self.period = period  # Time in seconds for one complete cycle (on + off)
        self.lastToggle = time.time()
        self.state = False  # False = OFF, True = ON
    
    def update(self, currentTime):
        """Non-blocking update - checks if it's time to toggle the LED"""
        if currentTime - self.lastToggle >= self.period / 2:
            self.state = not self.state
            mySetLedBrightness(self.ledPath, 255 if self.state else 0)
            self.lastToggle = currentTime
            return True  # LED was toggled
        return False  # No toggle this cycle

def main():
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}] hello")
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}] Starting non-blocking LED flash sequence...")
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}] Press Ctrl+C to stop")
    
    # Create LED controllers with different periods
    blueLED = LEDController(myLed1BluePath, period=1.0)   # 1 second cycle (0.5s on, 0.5s off)
    greenLED = LEDController(myLed2GreenPath, period=2.0)  # 2 second cycle (1s on, 1s off)
    
    try:
        while True:
            currentTime = time.time()
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
            
            # Update each LED independently
            if blueLED.update(currentTime):
                print(f"[{timestamp}] Blue LED: {'ON' if blueLED.state else 'OFF'}")
            
            if greenLED.update(currentTime):
                print(f"[{timestamp}] Green LED: {'ON' if greenLED.state else 'OFF'}")
            
            # Small sleep to prevent CPU spinning
            time.sleep(0.01)
    
    except KeyboardInterrupt:
        print(f"\n[{datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}] Stopping LED flash sequence...")
        # Turn off both LEDs on exit
        mySetLedBrightness(myLed1BluePath, 0)
        mySetLedBrightness(myLed2GreenPath, 0)
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}] LEDs turned off. Goodbye!")

if __name__ == "__main__":
    main()
