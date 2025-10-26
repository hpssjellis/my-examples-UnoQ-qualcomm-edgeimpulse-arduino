/*
 * Arduino UNO Q - STM32 RPC Server Sketch
 * 
 * This sketch provides information to the Snapdragon/QRB2210 processor
 * via RPC calls using the Arduino_RouterBridge library.
 * The Snapdragon initiates requests and measures round-trip timing.
 * 
 * NO DELAYS - All operations are instantaneous for accurate timing
 * 
 * INSTRUCTIONS:
 * 1. Upload this sketch to the STM32 side via Arduino IDE
 * 2. Copy the Python code at the bottom of this file (after the C++ code)
 * 3. Save it as stm32_query.py on the Snapdragon/Linux side
 * 4. Install dependencies: pip3 install arduino-rpc
 * 5. Run: python3 stm32_query.py
 */

#include <Arduino_RouterBridge.h>

// Pin definitions
const int LED_PIN = LED_BUILTIN;
const int ANALOG_PIN = A0;

// Variables to track state
int call_count = 0;

void setup() {
  Bridge.begin();
  Monitor.begin();
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(ANALOG_PIN, INPUT);
  
  Monitor.println("STM32 RPC Server Started");
  Monitor.println("Waiting for requests from Snapdragon...\n");
  
  // Register RPC methods that Snapdragon can call
  if (!Bridge.provide_safe("get_analog_value", get_analog_value)) {
    Monitor.println("Error providing method: get_analog_value");
  }
  
  if (!Bridge.provide_safe("get_uptime", get_uptime)) {
    Monitor.println("Error providing method: get_uptime");
  }
  
  if (!Bridge.provide_safe("get_free_memory", get_free_memory)) {
    Monitor.println("Error providing method: get_free_memory");
  }
  
  if (!Bridge.provide_safe("get_led_state", get_led_state)) {
    Monitor.println("Error providing method: get_led_state");
  }
  
  if (!Bridge.provide_safe("set_led_on", set_led_on)) {
    Monitor.println("Error providing method: set_led_on");
  }
  
  if (!Bridge.provide_safe("set_led_off", set_led_off)) {
    Monitor.println("Error providing method: set_led_off");
  }
  
  if (!Bridge.provide_safe("get_call_count", get_call_count)) {
    Monitor.println("Error providing method: get_call_count");
  }
  
  Monitor.println("All RPC methods registered successfully!");
}

void loop() {
  // Just let the Bridge handle incoming requests - no delays
  delay(1);
}

// RPC Methods callable from Snapdragon

int get_analog_value() {
  call_count++;
  int value = analogRead(ANALOG_PIN);
  Monitor.print("RPC call: get_analog_value() -> ");
  Monitor.println(value);
  return value;
}

unsigned long get_uptime() {
  call_count++;
  unsigned long uptime_ms = millis();
  Monitor.print("RPC call: get_uptime() -> ");
  Monitor.print(uptime_ms);
  Monitor.println(" ms");
  return uptime_ms;
}

int get_free_memory() {
  call_count++;
  // Approximate free memory calculation for ARM Cortex
  extern char _end;
  extern char *__brkval;
  int free_memory;
  
  if ((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&_end);
  } else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
  }
  
  Monitor.print("RPC call: get_free_memory() -> ");
  Monitor.print(free_memory);
  Monitor.println(" bytes");
  return free_memory;
}

bool get_led_state() {
  call_count++;
  bool state = digitalRead(LED_PIN);
  Monitor.print("RPC call: get_led_state() -> ");
  Monitor.println(state ? "ON" : "OFF");
  return state;
}

bool set_led_on() {
  call_count++;
  digitalWrite(LED_PIN, HIGH);
  Monitor.println("RPC call: set_led_on() -> LED ON");
  return true;
}

bool set_led_off() {
  call_count++;
  digitalWrite(LED_PIN, LOW);
  Monitor.println("RPC call: set_led_off() -> LED OFF");
  return true;
}

int get_call_count() {
  Monitor.print("RPC call: get_call_count() -> ");
  Monitor.println(call_count);
  return call_count;
}

/*
==============================================================================
PYTHON CODE FOR SNAPDRAGON SIDE - Copy everything below this line
Save as: stm32_query.py
==============================================================================

#!/usr/bin/env python3
"""
RPC Client for Arduino UNO Q - Queries STM32 information from Snapdragon
Run this on the Snapdragon/QRB2210 Linux side

Installation:
  pip3 install arduino-rpc

Usage:
  python3 stm32_query.py
"""

import time
from arduino_rpc import RpcClient

def main():
    print("Starting RPC client to query STM32...")
    print("Connecting to STM32...\n")
    
    # Initialize RPC client
    client = RpcClient()
    
    try:
        while True:
            print("=" * 50)
            print("Querying STM32 System Info")
            print("=" * 50)
            
            # Start timing
            start_time = time.time()
            
            results = {}
            success = True
            
            # Query 1: Get analog value
            try:
                results['analog'] = client.call('get_analog_value')
                print(f"Analog Value (A0): {results['analog']}")
            except Exception as e:
                print(f"ERROR: Failed to get analog value - {e}")
                success = False
            
            # Query 2: Get STM32 uptime
            try:
                results['uptime'] = client.call('get_uptime')
                uptime_sec = results['uptime'] / 1000.0
                print(f"STM32 Uptime: {uptime_sec:.2f} seconds")
            except Exception as e:
                print(f"ERROR: Failed to get uptime - {e}")
                success = False
            
            # Query 3: Get free memory
            try:
                results['memory'] = client.call('get_free_memory')
                print(f"Free Memory: {results['memory']} bytes")
            except Exception as e:
                print(f"ERROR: Failed to get free memory - {e}")
                success = False
            
            # Query 4: Get LED state (before turning on)
            try:
                results['led_state_before'] = client.call('get_led_state')
                print(f"LED State (before): {'ON' if results['led_state_before'] else 'OFF'}")
            except Exception as e:
                print(f"ERROR: Failed to get LED state - {e}")
                success = False
            
            # Query 5: Turn LED ON
            try:
                client.call('set_led_on')
                print("LED: Turned ON")
            except Exception as e:
                print(f"ERROR: Failed to turn LED on - {e}")
                success = False
            
            # Query 6: Get LED state (after turning on)
            try:
                results['led_state_on'] = client.call('get_led_state')
                print(f"LED State (after ON): {'ON' if results['led_state_on'] else 'OFF'}")
            except Exception as e:
                print(f"ERROR: Failed to get LED state - {e}")
                success = False
            
            # Query 7: Turn LED OFF
            try:
                client.call('set_led_off')
                print("LED: Turned OFF")
            except Exception as e:
                print(f"ERROR: Failed to turn LED off - {e}")
                success = False
            
            # Query 8: Get LED state (after turning off)
            try:
                results['led_state_off'] = client.call('get_led_state')
                print(f"LED State (after OFF): {'ON' if results['led_state_off'] else 'OFF'}")
            except Exception as e:
                print(f"ERROR: Failed to get LED state - {e}")
                success = False
            
            # Query 9: Get total call count
            try:
                results['call_count'] = client.call('get_call_count')
                print(f"Total RPC Calls: {results['call_count']}")
            except Exception as e:
                print(f"ERROR: Failed to get call count - {e}")
                success = False
            
            # Calculate elapsed time
            elapsed_time = (time.time() - start_time) * 1000  # Convert to ms
            
            print()
            if success:
                print("✓ All queries successful!")
            else:
                print("✗ Some queries failed - check STM32 service")
            
            print(f"Round-trip time: {elapsed_time:.2f} ms")
            print("=" * 50)
            print()
            
            # Wait 5 seconds before next query cycle
            time.sleep(5)
            
    except KeyboardInterrupt:
        print("\n\nShutting down RPC client...")
        print("Goodbye!")
    except Exception as e:
        print(f"\n\nFATAL ERROR: {e}")
        print("Make sure the STM32 sketch is running and providing RPC services.")

if __name__ == "__main__":
    main()

*/
