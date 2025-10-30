import time
import os

# --- Configuration: Linux File Paths for Onboard LEDs ---
# These are the file paths used by the Linux operating system on the UNO Q
# to control the brightness of the built-in user LEDs.
myLed1BluePath = "/sys/class/leds/blue:user/brightness"
myLed2GreenPath = "/sys/class/leds/green:wlan/brightness"

# --- Function to Control LED Brightness ---
# 0 is OFF, 255 is ON (maximum brightness)
def mySetLedBrightness(myLedFilePath, myValue):
    """Writes a brightness value (0-255) to the specified LED file path."""
    try:
        # 'w' opens the file for writing
        with open(myLedFilePath, "w") as myFile:
            myFile.write(str(myValue))
    except Exception as myError:
        # You may need to run your script with sufficient permissions (e.g., as root)
        print(f"Error controlling LED at {myLedFilePath}: {myError}. Check permissions.")

def main():
    print("hello")
    print("Starting LED flash sequence...")
    
    # Flash the LEDs 5 times
    for i in range(5):
        # Turn on LED1_B (blue) and LED2_G (green)
        mySetLedBrightness(myLed1BluePath, 255)
        mySetLedBrightness(myLed2GreenPath, 255)
        print(f"LEDs ON - Flash {i+1}")
        
        # Keep LEDs on for 1 second
        time.sleep(1)
        
        # Turn off the LEDs
        mySetLedBrightness(myLed1BluePath, 0)
        mySetLedBrightness(myLed2GreenPath, 0)
        print(f"LEDs OFF - Flash {i+1}")
        
        # Keep LEDs off for 1 second
        time.sleep(1)
    
    print("LED flash sequence complete!")

if __name__ == "__main__":
    main()
