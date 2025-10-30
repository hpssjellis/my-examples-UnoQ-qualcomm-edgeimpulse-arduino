import time
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

def timestamp():
    """Returns current timestamp string"""
    return datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]

def main():
    print(f"[{timestamp()}] hello")
    print(f"[{timestamp()}] Starting LED flash sequence...")
    print(f"[{timestamp()}] Press Ctrl+C to stop")
    
    blueState = False
    greenState = False
    blueLastToggle = time.time()
    greenLastToggle = time.time()
    
    try:
        while True:
            now = time.time()
            
            # Toggle blue LED every 0.5 seconds (1 second period)
            if now - blueLastToggle >= 0.5:
                blueState = not blueState
                mySetLedBrightness(myLed1BluePath, 255 if blueState else 0)
                print(f"[{timestamp()}] Blue LED: {'ON' if blueState else 'OFF'}")
                blueLastToggle = now
            
            # Toggle green LED every 1 second (2 second period)
            if now - greenLastToggle >= 1.0:
                greenState = not greenState
                mySetLedBrightness(myLed2GreenPath, 255 if greenState else 0)
                print(f"[{timestamp()}] Green LED: {'ON' if greenState else 'OFF'}")
                greenLastToggle = now
            
            time.sleep(0.01)  # Small sleep to prevent CPU spinning
    
    except KeyboardInterrupt:
        print(f"\n[{timestamp()}] Stopping LED flash sequence...")
        mySetLedBrightness(myLed1BluePath, 0)
        mySetLedBrightness(myLed2GreenPath, 0)
        print(f"[{timestamp()}] LEDs turned off. Goodbye!")

if __name__ == "__main__":
    main()
