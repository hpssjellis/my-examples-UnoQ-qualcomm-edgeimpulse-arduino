from unoQ import *
import time

# Initialize unoQ connection
uno = unoQ()

# Turn on LED1_B (blue LED on pin 9)
uno.LED1_B.on()

# Turn on LED2_G (green LED on pin 5)
uno.LED2_G.on()

# Write "hello" to the monitor
uno.monitor("hello")

# Write "hello" to the logs
uno.log("hello")

print("LEDs turned on and messages sent!")

# Non-blocking 1 second delay
time.sleep(1)

# Turn off the LEDs after delay
uno.LED1_B.off()
uno.LED2_G.off()

print("LEDs turned off after 1 second")
