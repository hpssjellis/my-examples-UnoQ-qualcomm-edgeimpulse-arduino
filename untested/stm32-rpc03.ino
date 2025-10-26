/*
 * Arduino UNO Q - STM32 RPC Client Sketch
 * 
 * This sketch queries system information from the Snapdragon/QRB2210 processor
 * via RPC calls using the Arduino_RouterBridge library.
 * 
 * NO DELAYS - All operations are instantaneous for accurate timing
 * 
 * INSTRUCTIONS:
 * 1. Upload this sketch to the STM32 side via Arduino IDE
 * 2. Copy the Python code at the bottom of this file (after the C++ code)
 * 3. Save it as system_info_rpc.py on the Snapdragon/Linux side
 * 4. Install dependencies: pip3 install psutil
 * 5. Run: python3 system_info_rpc.py
 */

#include <Arduino_RouterBridge.h>

// Variables to store system info
String timestamp;
float uptime;
float cpu_temp;
float cpu_usage;
float mem_usage;
String ip_address;
int query_count;

void setup() {
  Bridge.begin();
  Monitor.begin();
  
  Monitor.println("STM32 RPC Client Started");
  Monitor.println("Querying Snapdragon system info...\n");
}

void loop() {
  bool success = true;
  unsigned long start_time = millis();
  
  Monitor.println("=== Querying Snapdragon System Info ===");
  
  // Query 1: Get current timestamp
  if (Bridge.call("get_timestamp").result(timestamp)) {
    Monitor.print("Timestamp: ");
    Monitor.println(timestamp);
  } else {
    Monitor.println("ERROR: Failed to get timestamp");
    success = false;
  }
  
  // Query 2: Get system uptime in seconds
  if (Bridge.call("get_uptime").result(uptime)) {
    Monitor.print("Uptime: ");
    Monitor.print(uptime);
    Monitor.println(" seconds");
  } else {
    Monitor.println("ERROR: Failed to get uptime");
    success = false;
  }
  
  // Query 3: Get CPU temperature
  if (Bridge.call("get_cpu_temp").result(cpu_temp)) {
    Monitor.print("CPU Temperature: ");
    Monitor.print(cpu_temp);
    Monitor.println(" °C");
  } else {
    Monitor.println("ERROR: Failed to get CPU temperature");
    success = false;
  }
  
  // Query 4: Get CPU usage percentage
  if (Bridge.call("get_cpu_usage").result(cpu_usage)) {
    Monitor.print("CPU Usage: ");
    Monitor.print(cpu_usage);
    Monitor.println(" %");
  } else {
    Monitor.println("ERROR: Failed to get CPU usage");
    success = false;
  }
  
  // Query 5: Get memory usage percentage
  if (Bridge.call("get_memory_usage").result(mem_usage)) {
    Monitor.print("Memory Usage: ");
    Monitor.print(mem_usage);
    Monitor.println(" %");
  } else {
    Monitor.println("ERROR: Failed to get memory usage");
    success = false;
  }
  
  // Query 6: Get IP address
  if (Bridge.call("get_ip_address").result(ip_address)) {
    Monitor.print("IP Address: ");
    Monitor.println(ip_address);
  } else {
    Monitor.println("ERROR: Failed to get IP address");
    success = false;
  }
  
  // Query 7: Get total query count
  if (Bridge.call("get_query_count").result(query_count)) {
    Monitor.print("Total Queries Handled: ");
    Monitor.println(query_count);
  } else {
    Monitor.println("ERROR: Failed to get query count");
    success = false;
  }
  
  if (success) {
    Monitor.println("\n✓ All queries successful!");
  } else {
    Monitor.println("\n✗ Some queries failed - check Snapdragon service");
  }
  
  unsigned long elapsed_time = millis() - start_time;
  Monitor.print("Round-trip time: ");
  Monitor.print(elapsed_time);
  Monitor.println(" ms");
  
  Monitor.println("=====================================\n");
  
  // Query every 5 seconds
  delay(5000);
}

/*
==============================================================================
PYTHON CODE FOR SNAPDRAGON SIDE - Copy everything below this line
Save as: system_info_rpc.py
==============================================================================

#!/usr/bin/env python3
"""
RPC Server for Arduino UNO Q - Provides system information to STM32
Run this on the Snapdragon/QRB2210 Linux side

Installation:
  pip3 install psutil
  pip3 install arduino-rpc

Usage:
  python3 system_info_rpc.py
"""

import time
import psutil
import socket
from datetime import datetime
from arduino_rpc import RpcServer

# Initialize RPC server
server = RpcServer()

# Track number of queries
query_count = 0

@server.register
def get_timestamp():
    """Return current timestamp as string"""
    global query_count
    query_count += 1
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

@server.register
def get_uptime():
    """Return system uptime in seconds"""
    global query_count
    query_count += 1
    return time.time() - psutil.boot_time()

@server.register
def get_cpu_temp():
    """Return CPU temperature in Celsius"""
    global query_count
    query_count += 1
    try:
        temps = psutil.sensors_temperatures()
        if 'cpu_thermal' in temps:
            return temps['cpu_thermal'][0].current
        elif 'coretemp' in temps:
            return temps['coretemp'][0].current
        else:
            # Return first available temperature
            for name, entries in temps.items():
                if entries:
                    return entries[0].current
    except:
        pass
    return 0.0  # Return 0 if temperature not available

@server.register
def get_cpu_usage():
    """Return CPU usage percentage - instant read, no delay"""
    global query_count
    query_count += 1
    return psutil.cpu_percent(interval=None)

@server.register
def get_memory_usage():
    """Return memory usage percentage"""
    global query_count
    query_count += 1
    return psutil.virtual_memory().percent

@server.register
def get_ip_address():
    """Return IP address of first network interface"""
    global query_count
    query_count += 1
    try:
        # Get all network interfaces
        addrs = psutil.net_if_addrs()
        for interface, addr_list in addrs.items():
            if interface.startswith('lo'):  # Skip loopback
                continue
            for addr in addr_list:
                if addr.family == socket.AF_INET:
                    return addr.address
    except:
        pass
    return "No IP"

@server.register
def get_query_count():
    """Return total number of queries handled"""
    return query_count

if __name__ == "__main__":
    print("Starting RPC server for system info...")
    print("Registered methods:")
    print("  - get_timestamp")
    print("  - get_uptime")
    print("  - get_cpu_temp")
    print("  - get_cpu_usage")
    print("  - get_memory_usage")
    print("  - get_ip_address")
    print("  - get_query_count")
    print("\nWaiting for RPC calls from STM32...\n")
    server.run()

*/
