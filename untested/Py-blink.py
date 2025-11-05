#!/usr/bin/env python3
# Arduino UNO Q - Control all RGB LED colors
import time

# GPIO paths for RGB LED on Arduino UNO Q
LED_R = "/sys/class/leds/led1_r/brightness"
LED_G = "/sys/class/leds/led1_g/brightness"
LED_B = "/sys/class/leds/led1_b/brightness"

def set_led(led_path, state):
    """Set LED state: 1 for on, 0 for off"""
    with open(led_path, 'w') as led:
        led.write(str(state))

def set_rgb(r, g, b):
    """Set RGB LED color (1=on, 0=off for each channel)"""
    set_led(LED_R, r)
    set_led(LED_G, g)
    set_led(LED_B, b)

print("RGB LED Color Demo on Arduino UNO Q...")
print("Press Ctrl+C to stop")

try:
    while True:
        print("Red")
        set_rgb(1, 0, 0)
        time.sleep(1)
        
        print("Green")
        set_rgb(0, 1, 0)
        time.sleep(1)
        
        print("Blue")
        set_rgb(0, 0, 1)
        time.sleep(1)
        
        print("Yellow")
        set_rgb(1, 1, 0)
        time.sleep(1)
        
        print("Cyan")
        set_rgb(0, 1, 1)
        time.sleep(1)
        
        print("Magenta")
        set_rgb(1, 0, 1)
        time.sleep(1)
        
        print("White")
        set_rgb(1, 1, 1)
        time.sleep(1)
        
        print("Off")
        set_rgb(0, 0, 0)
        time.sleep(1)

except KeyboardInterrupt:
    print("\nStopping...")
    set_rgb(0, 0, 0)  # Turn off all LEDs
    print("LEDs off")
