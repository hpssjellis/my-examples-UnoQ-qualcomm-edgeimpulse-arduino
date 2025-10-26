/*
 * Arduino UNO Q - Low Power Sleep with Periodic Vision Detection
 * 
 * This sketch puts both STM32 and Snapdragon into low power mode,
 * then wakes up periodically to perform object detection.
 * 
 * SLEEP/WAKE CYCLE:
 * - Sleep for 15 minutes (configurable)
 * - Wake up Snapdragon
 * - Run detection for 30 seconds
 * - Log results
 * - Go back to sleep
 * 
 * Perfect for:
 * - Battery-powered wildlife cameras
 * - Security monitoring
 * - Time-lapse object detection
 * - Low-power IoT applications
 * 
 * POWER SAVINGS:
 * With 15min sleep / 30sec active:
 * - Active time: ~3% of total
 * - Sleep time: ~97% of total
 * - Estimated battery life increase: 20-30x
 * 
 * INSTRUCTIONS:
 * 1. Upload this sketch to the STM32 side
 * 2. Copy the Python code and save as sleep_detector.py on Snapdragon
 * 3. Install dependencies (see Python code)
 * 4. The Python script will auto-start on boot (systemd service recommended)
 * 5. STM32 will control the sleep/wake cycle
 */

#include <Arduino_RouterBridge.h>
#include <STM32LowPower.h>

// Configuration
const unsigned long SLEEP_DURATION_MS = 15 * 60 * 1000;  // 15 minutes
const unsigned long DETECTION_DURATION_MS = 30 * 1000;    // 30 seconds
const unsigned long SNAPDRAGON_BOOT_TIME_MS = 10 * 1000;  // 10 sec for Linux boot

// Pins
const int SNAPDRAGON_POWER_PIN = 4;  // Controls Snapdragon power
const int STATUS_LED = LED_BUILTIN;
const int DETECTION_LED = 2;

// Detection results storage
struct DetectionLog {
  unsigned long timestamp;
  int object_count;
  bool person_detected;
  float person_confidence;
  char objects[128];  // Comma-separated object list
};

DetectionLog last_detection;
int wake_cycle = 0;

void setup() {
  Bridge.begin();
  Monitor.begin();
  
  pinMode(SNAPDRAGON_POWER_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(DETECTION_LED, OUTPUT);
  
  // Configure low power
  LowPower.begin();
  
  Monitor.println("╔═════════════════════════════════════════════╗");
  Monitor.println("║  Low Power Vision Detection System         ║");
  Monitor.println("║  STM32 + Snapdragon QRB2210                 ║");
  Monitor.println("╚═════════════════════════════════════════════╝");
  Monitor.println();
  Monitor.print("Sleep Duration: ");
  Monitor.print(SLEEP_DURATION_MS / 60000);
  Monitor.println(" minutes");
  Monitor.print("Detection Duration: ");
  Monitor.print(DETECTION_DURATION_MS / 1000);
  Monitor.println(" seconds");
  Monitor.println();
  
  // Initial state - Snapdragon off
  powerDownSnapdragon();
  
  delay(2000);
  Monitor.println("System initialized. Starting sleep/wake cycle...\n");
}

void loop() {
  wake_cycle++;
  
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Monitor.print("Wake Cycle #");
  Monitor.println(wake_cycle);
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  
  // WAKE PHASE
  Monitor.println("⚡ WAKE PHASE");
  digitalWrite(STATUS_LED, HIGH);
  
  // Power up Snapdragon
  Monitor.println("🔌 Powering up Snapdragon...");
  powerUpSnapdragon();
  
  // Wait for Linux to boot
  Monitor.print("⏳ Waiting for Linux boot (");
  Monitor.print(SNAPDRAGON_BOOT_TIME_MS / 1000);
  Monitor.println(" seconds)...");
  delay(SNAPDRAGON_BOOT_TIME_MS);
  
  // Signal detection service to start
  Monitor.println("📸 Starting detection service...");
  bool detection_started = false;
  if (Bridge.call("start_detection").result(detection_started)) {
    if (detection_started) {
      Monitor.println("✓ Detection service started");
    } else {
      Monitor.println("✗ Detection service failed to start");
    }
  } else {
    Monitor.println("✗ Cannot communicate with Snapdragon");
  }
  
  // RUN DETECTION PHASE
  Monitor.println();
  Monitor.print("🎥 DETECTION PHASE (");
  Monitor.print(DETECTION_DURATION_MS / 1000);
  Monitor.println(" seconds)");
  digitalWrite(DETECTION_LED, HIGH);
  
  unsigned long detection_start = millis();
  unsigned long last_query = 0;
  int query_count = 0;
  
  // Query detection results every 2 seconds during active period
  while (millis() - detection_start < DETECTION_DURATION_MS) {
    if (millis() - last_query >= 2000) {
      last_query = millis();
      query_count++;
      
      Monitor.print("Query ");
      Monitor.print(query_count);
      Monitor.print(": ");
      
      // Get current detection results
      String objects;
      int count;
      bool person;
      float confidence;
      
      if (Bridge.call("get_detected_objects").result(objects) &&
          Bridge.call("get_object_count").result(count) &&
          Bridge.call("is_person_detected").result(person) &&
          Bridge.call("get_person_confidence").result(confidence)) {
        
        if (count > 0) {
          Monitor.print(count);
          Monitor.print(" objects - ");
          Monitor.println(objects);
          
          if (person) {
            Monitor.print("  👤 Person (");
            Monitor.print(confidence * 100);
            Monitor.println("%)");
          }
        } else {
          Monitor.println("No objects detected");
        }
        
        // Store latest detection
        last_detection.timestamp = millis();
        last_detection.object_count = count;
        last_detection.person_detected = person;
        last_detection.person_confidence = confidence;
        objects.toCharArray(last_detection.objects, 128);
        
      } else {
        Monitor.println("Query failed");
      }
    }
    
    delay(100);  // Small delay to prevent overwhelming CPU
  }
  
  digitalWrite(DETECTION_LED, LOW);
  
  // SUMMARY
  Monitor.println();
  Monitor.println("📊 DETECTION SUMMARY");
  Monitor.print("Total Queries: ");
  Monitor.println(query_count);
  Monitor.print("Last Object Count: ");
  Monitor.println(last_detection.object_count);
  if (last_detection.object_count > 0) {
    Monitor.print("Objects: ");
    Monitor.println(last_detection.objects);
    if (last_detection.person_detected) {
      Monitor.print("Person Confidence: ");
      Monitor.print(last_detection.person_confidence * 100);
      Monitor.println("%");
    }
  }
  
  // Stop detection service
  Monitor.println();
  Monitor.println("🛑 Stopping detection service...");
  Bridge.call("stop_detection");
  
  delay(1000);
  
  // Power down Snapdragon
  Monitor.println("💤 Powering down Snapdragon...");
  powerDownSnapdragon();
  
  digitalWrite(STATUS_LED, LOW);
  
  // SLEEP PHASE
  Monitor.println();
  Monitor.println("😴 SLEEP PHASE");
  Monitor.print("Sleeping for ");
  Monitor.print(SLEEP_DURATION_MS / 60000);
  Monitor.println(" minutes...");
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  delay(100);  // Let serial finish
  
  // Enter low power sleep
  LowPower.deepSleep(SLEEP_DURATION_MS);
  
  // When we wake up, loop() runs again
}

void powerUpSnapdragon() {
  digitalWrite(SNAPDRAGON_POWER_PIN, HIGH);
  // Additional power-up sequencing can go here
}

void powerDownSnapdragon() {
  // Send shutdown command first (graceful)
  Bridge.call("prepare_shutdown");
  delay(2000);  // Give time for graceful shutdown
  
  // Cut power
  digitalWrite(SNAPDRAGON_POWER_PIN, LOW);
}

/*
==============================================================================
PYTHON CODE FOR SNAPDRAGON SIDE - Copy everything below this line
Save as: sleep_detector.py
==============================================================================

#!/usr/bin/env python3
"""
Low Power Object Detection Service for Arduino UNO Q
Designed to be woken up periodically, run detection, then sleep

Installation:
  pip3 install opencv-python
  pip3 install tflite-runtime
  pip3 install numpy
  pip3 install arduino-rpc
  
  # Download TFLite model:
  wget https://storage.googleapis.com/download.tensorflow.org/models/tflite/coco_ssd_mobilenet_v1_1.0_quant_2018_06_29.zip
  unzip coco_ssd_mobilenet_v1_1.0_quant_2018_06_29.zip

Setup as systemd service (auto-start on boot):
  sudo cp sleep_detector.py /usr/local/bin/
  sudo chmod +x /usr/local/bin/sleep_detector.py
  
  Create /etc/systemd/system/sleep-detector.service:
    [Unit]
    Description=Sleep Detector RPC Service
    After=network.target
    
    [Service]
    ExecStart=/usr/bin/python3 /usr/local/bin/sleep_detector.py
    Restart=always
    User=root
    
    [Install]
    WantedBy=multi-user.target
  
  sudo systemctl enable sleep-detector
  sudo systemctl start sleep-detector

Usage:
  python3 sleep_detector.py
"""

import cv2
import numpy as np
from arduino_rpc import RpcServer
import threading
import time
import os
import signal
import sys

try:
    from tflite_runtime.interpreter import Interpreter
except ImportError:
    import tensorflow as tf
    Interpreter = tf.lite.Interpreter

# Global variables
detected_objects = []
detection_lock = threading.Lock()
detection_active = False
camera = None
detector = None
camera_thread = None

# COCO labels (subset)
LABELS = [
    'person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus', 'train',
    'truck', 'boat', 'traffic light', 'fire hydrant', 'stop sign',
    'parking meter', 'bench', 'bird', 'cat', 'dog', 'horse', 'sheep', 'cow',
    'elephant', 'bear', 'zebra', 'giraffe', 'backpack', 'umbrella', 'handbag',
    'tie', 'suitcase', 'frisbee', 'skis', 'snowboard', 'sports ball', 'kite',
    'baseball bat', 'baseball glove', 'skateboard', 'surfboard', 'tennis racket',
    'bottle', 'wine glass', 'cup', 'fork', 'knife', 'spoon', 'bowl', 'banana',
    'apple', 'sandwich', 'orange', 'broccoli', 'carrot', 'hot dog', 'pizza',
    'donut', 'cake', 'chair', 'couch', 'potted plant', 'bed', 'dining table',
    'toilet', 'tv', 'laptop', 'mouse', 'remote', 'keyboard', 'cell phone',
    'microwave', 'oven', 'toaster', 'sink', 'refrigerator', 'book', 'clock',
    'vase', 'scissors', 'teddy bear', 'hair drier', 'toothbrush'
]

class ObjectDetector:
    def __init__(self, model_path='detect.tflite', threshold=0.5):
        self.threshold = threshold
        self.interpreter = Interpreter(model_path=model_path)
        self.interpreter.allocate_tensors()
        
        self.input_details = self.interpreter.get_input_details()
        self.output_details = self.interpreter.get_output_details()
        
        self.input_shape = self.input_details[0]['shape']
        self.height = self.input_shape[1]
        self.width = self.input_shape[2]
    
    def detect(self, frame):
        img = cv2.resize(frame, (self.width, self.height))
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        input_data = np.expand_dims(img, axis=0)
        
        self.interpreter.set_tensor(self.input_details[0]['index'], input_data)
        self.interpreter.invoke()
        
        boxes = self.interpreter.get_tensor(self.output_details[0]['index'])[0]
        classes = self.interpreter.get_tensor(self.output_details[1]['index'])[0]
        scores = self.interpreter.get_tensor(self.output_details[2]['index'])[0]
        
        results = []
        for i in range(len(scores)):
            if scores[i] > self.threshold:
                class_id = int(classes[i])
                if class_id < len(LABELS):
                    results.append({
                        'label': LABELS[class_id],
                        'confidence': float(scores[i])
                    })
        
        return results

def detection_thread_func():
    """Background thread for continuous detection"""
    global detected_objects, detection_active, camera, detector
    
    print("[Detection Thread] Starting...")
    
    while detection_active:
        if camera is None or detector is None:
            time.sleep(0.1)
            continue
        
        ret, frame = camera.read()
        if not ret:
            print("[Detection Thread] Failed to grab frame")
            time.sleep(0.1)
            continue
        
        # Detect objects
        results = detector.detect(frame)
        
        # Update global results
        with detection_lock:
            detected_objects = results
        
        # Small delay
        time.sleep(0.1)
    
    print("[Detection Thread] Stopped")

# Initialize RPC server
server = RpcServer()

@server.register
def start_detection():
    """Initialize camera and detector, start detection thread"""
    global detection_active, camera, detector, camera_thread, detected_objects
    
    print("[RPC] start_detection() called")
    
    if detection_active:
        print("[RPC] Detection already active")
        return True
    
    try:
        # Initialize camera
        print("[RPC] Opening camera...")
        camera = cv2.VideoCapture(0)
        if not camera.isOpened():
            print("[RPC] ERROR: Cannot open camera")
            return False
        
        # Initialize detector
        print("[RPC] Loading model...")
        detector = ObjectDetector('detect.tflite', threshold=0.5)
        
        # Clear previous detections
        with detection_lock:
            detected_objects = []
        
        # Start detection thread
        detection_active = True
        camera_thread = threading.Thread(target=detection_thread_func, daemon=True)
        camera_thread.start()
        
        print("[RPC] Detection started successfully")
        return True
        
    except Exception as e:
        print(f"[RPC] ERROR starting detection: {e}")
        return False

@server.register
def stop_detection():
    """Stop detection and release camera"""
    global detection_active, camera, detector, camera_thread
    
    print("[RPC] stop_detection() called")
    
    if not detection_active:
        print("[RPC] Detection not active")
        return True
    
    # Stop thread
    detection_active = False
    
    if camera_thread:
        camera_thread.join(timeout=2)
    
    # Release camera
    if camera:
        camera.release()
        camera = None
    
    detector = None
    
    print("[RPC] Detection stopped")
    return True

@server.register
def get_detected_objects():
    """Return comma-separated list of detected objects"""
    with detection_lock:
        if not detected_objects:
            return ""
        labels = [obj['label'] for obj in detected_objects]
        return ", ".join(labels)

@server.register
def get_object_count():
    """Return number of detected objects"""
    with detection_lock:
        return len(detected_objects)

@server.register
def is_person_detected():
    """Check if person is detected"""
    with detection_lock:
        for obj in detected_objects:
            if obj['label'] == 'person':
                return True
        return False

@server.register
def get_person_confidence():
    """Get person detection confidence"""
    with detection_lock:
        for obj in detected_objects:
            if obj['label'] == 'person':
                return obj['confidence']
        return 0.0

@server.register
def prepare_shutdown():
    """Prepare for graceful shutdown"""
    print("[RPC] prepare_shutdown() called")
    stop_detection()
    print("[RPC] Ready for shutdown")
    return True

def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    print("\n[Main] Shutdown signal received")
    stop_detection()
    sys.exit(0)

if __name__ == "__main__":
    print("╔═════════════════════════════════════════════╗")
    print("║  Low Power Detection RPC Service           ║")
    print("╚═════════════════════════════════════════════╝")
    print()
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    print("RPC Server starting...")
    print("Registered methods:")
    print("  - start_detection()")
    print("  - stop_detection()")
    print("  - get_detected_objects()")
    print("  - get_object_count()")
    print("  - is_person_detected()")
    print("  - get_person_confidence()")
    print("  - prepare_shutdown()")
    print("\nWaiting for commands from STM32...\n")
    
    try:
        server.run()
    except KeyboardInterrupt:
        print("\n[Main] Interrupted")
        stop_detection()

*/
