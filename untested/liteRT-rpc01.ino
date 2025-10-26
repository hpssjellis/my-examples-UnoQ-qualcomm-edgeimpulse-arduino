/*
 * Arduino UNO Q - Webcam Object Detection RPC Client
 * 
 * This sketch queries object detection results from a webcam running on the
 * Snapdragon/QRB2210 processor using TensorFlow Lite.
 * 
 * The STM32 can:
 * - Ask what objects are currently detected
 * - Check if specific objects are present
 * - Control LEDs based on detections
 * - Get detection confidence scores
 * 
 * INSTRUCTIONS:
 * 1. Upload this sketch to the STM32 side via Arduino IDE
 * 2. Copy the Python code at the bottom and save as webcam_detector.py
 * 3. Install dependencies on Snapdragon (see Python code for details)
 * 4. Run: python3 webcam_detector.py
 * 5. Watch the Serial Monitor for detection results!
 */

#include <Arduino_RouterBridge.h>

// LED pins for visual feedback
const int PERSON_LED = 2;    // Lights when person detected
const int OBJECT_LED = 3;    // Lights when any object detected
const int STATUS_LED = LED_BUILTIN;  // Blinks when querying

// Variables
String detected_objects;
bool person_detected;
int object_count;
float person_confidence;

void setup() {
  Bridge.begin();
  Monitor.begin();
  
  pinMode(PERSON_LED, OUTPUT);
  pinMode(OBJECT_LED, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  
  Monitor.println("╔═══════════════════════════════════════════╗");
  Monitor.println("║  STM32 Webcam Object Detection Client    ║");
  Monitor.println("╚═══════════════════════════════════════════╝");
  Monitor.println();
  Monitor.println("Starting object detection queries...\n");
  
  // Initial LED test
  digitalWrite(PERSON_LED, HIGH);
  digitalWrite(OBJECT_LED, HIGH);
  digitalWrite(STATUS_LED, HIGH);
  delay(500);
  digitalWrite(PERSON_LED, LOW);
  digitalWrite(OBJECT_LED, LOW);
  digitalWrite(STATUS_LED, LOW);
}

void loop() {
  unsigned long start_time = millis();
  
  // Status LED on during query
  digitalWrite(STATUS_LED, HIGH);
  
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Monitor.println("🎥 QUERYING WEBCAM...");
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  
  // Query 1: Get list of detected objects
  if (Bridge.call("get_detected_objects").result(detected_objects)) {
    Monitor.print("📦 Detected Objects: ");
    if (detected_objects.length() > 0) {
      Monitor.println(detected_objects);
    } else {
      Monitor.println("None");
    }
  } else {
    Monitor.println("❌ ERROR: Failed to get detected objects");
  }
  
  // Query 2: Get total object count
  if (Bridge.call("get_object_count").result(object_count)) {
    Monitor.print("🔢 Object Count: ");
    Monitor.println(object_count);
    
    // Control object LED
    if (object_count > 0) {
      digitalWrite(OBJECT_LED, HIGH);
    } else {
      digitalWrite(OBJECT_LED, LOW);
    }
  } else {
    Monitor.println("❌ ERROR: Failed to get object count");
  }
  
  // Query 3: Check if person is detected
  if (Bridge.call("is_person_detected").result(person_detected)) {
    Monitor.print("👤 Person Detected: ");
    Monitor.println(person_detected ? "YES" : "NO");
    
    // Control person LED
    digitalWrite(PERSON_LED, person_detected ? HIGH : LOW);
  } else {
    Monitor.println("❌ ERROR: Failed to check for person");
  }
  
  // Query 4: Get person detection confidence
  if (Bridge.call("get_person_confidence").result(person_confidence)) {
    if (person_confidence > 0.0) {
      Monitor.print("📊 Person Confidence: ");
      Monitor.print(person_confidence * 100.0);
      Monitor.println("%");
    }
  } else {
    Monitor.println("❌ ERROR: Failed to get confidence");
  }
  
  // Query 5: Check for specific object (example: bottle)
  bool bottle_found;
  if (Bridge.call("is_object_detected", String("bottle")).result(bottle_found)) {
    if (bottle_found) {
      Monitor.println("🍾 Bottle detected!");
    }
  }
  
  // Query 6: Check for cup
  bool cup_found;
  if (Bridge.call("is_object_detected", String("cup")).result(cup_found)) {
    if (cup_found) {
      Monitor.println("☕ Cup detected!");
    }
  }
  
  // Query 7: Check for cell phone
  bool phone_found;
  if (Bridge.call("is_object_detected", String("cell phone")).result(phone_found)) {
    if (phone_found) {
      Monitor.println("📱 Cell phone detected!");
    }
  }
  
  unsigned long elapsed_time = millis() - start_time;
  
  Monitor.println();
  Monitor.print("⏱️  Round-trip time: ");
  Monitor.print(elapsed_time);
  Monitor.println(" ms");
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  // Status LED off
  digitalWrite(STATUS_LED, LOW);
  
  // Query every 2 seconds for real-time feedback
  delay(2000);
}

/*
==============================================================================
PYTHON CODE FOR SNAPDRAGON SIDE - Copy everything below this line
Save as: webcam_detector.py
==============================================================================

#!/usr/bin/env python3
"""
Webcam Object Detection RPC Server for Arduino UNO Q
Uses TensorFlow Lite for real-time object detection
Run this on the Snapdragon/QRB2210 Linux side

Installation:
  pip3 install opencv-python
  pip3 install tflite-runtime
  pip3 install numpy
  pip3 install arduino-rpc
  
  # Download the TFLite model and labels:
  wget https://storage.googleapis.com/download.tensorflow.org/models/tflite/coco_ssd_mobilenet_v1_1.0_quant_2018_06_29.zip
  unzip coco_ssd_mobilenet_v1_1.0_quant_2018_06_29.zip

Usage:
  python3 webcam_detector.py
"""

import cv2
import numpy as np
from arduino_rpc import RpcServer
import threading
import time

try:
    from tflite_runtime.interpreter import Interpreter
except ImportError:
    print("Warning: tflite_runtime not found, using tensorflow.lite instead")
    import tensorflow as tf
    Interpreter = tf.lite.Interpreter

# Global variables for detection results
detected_objects = []
detection_lock = threading.Lock()
camera_active = False

# COCO dataset labels (subset of common objects)
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
        
        # Load TFLite model
        print(f"Loading model: {model_path}")
        self.interpreter = Interpreter(model_path=model_path)
        self.interpreter.allocate_tensors()
        
        # Get input and output details
        self.input_details = self.interpreter.get_input_details()
        self.output_details = self.interpreter.get_output_details()
        
        # Get input shape
        self.input_shape = self.input_details[0]['shape']
        self.height = self.input_shape[1]
        self.width = self.input_shape[2]
        
        print(f"Model loaded! Input size: {self.width}x{self.height}")
    
    def detect(self, frame):
        # Resize and preprocess
        img = cv2.resize(frame, (self.width, self.height))
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        
        # Add batch dimension and convert to uint8
        input_data = np.expand_dims(img, axis=0)
        
        # Run inference
        self.interpreter.set_tensor(self.input_details[0]['index'], input_data)
        self.interpreter.invoke()
        
        # Get results
        boxes = self.interpreter.get_tensor(self.output_details[0]['index'])[0]
        classes = self.interpreter.get_tensor(self.output_details[1]['index'])[0]
        scores = self.interpreter.get_tensor(self.output_details[2]['index'])[0]
        
        # Filter by threshold and build results
        results = []
        for i in range(len(scores)):
            if scores[i] > self.threshold:
                class_id = int(classes[i])
                if class_id < len(LABELS):
                    results.append({
                        'label': LABELS[class_id],
                        'confidence': float(scores[i]),
                        'box': boxes[i].tolist()
                    })
        
        return results

def camera_thread():
    """Background thread that continuously captures and processes frames"""
    global detected_objects, camera_active
    
    print("Initializing webcam...")
    cap = cv2.VideoCapture(0)
    
    if not cap.isOpened():
        print("ERROR: Cannot open webcam!")
        return
    
    print("Webcam opened successfully!")
    
    # Initialize detector
    try:
        detector = ObjectDetector('detect.tflite', threshold=0.5)
    except Exception as e:
        print(f"ERROR: Could not load model: {e}")
        print("Please download the model first (see installation instructions)")
        return
    
    camera_active = True
    print("Starting detection loop...\n")
    
    while camera_active:
        ret, frame = cap.read()
        if not ret:
            print("ERROR: Failed to grab frame")
            break
        
        # Detect objects
        results = detector.detect(frame)
        
        # Update global results
        with detection_lock:
            detected_objects = results
        
        # Small delay to prevent overwhelming CPU
        time.sleep(0.1)
    
    cap.release()
    print("Camera thread stopped")

# Initialize RPC server
server = RpcServer()

@server.register
def get_detected_objects():
    """Return comma-separated list of detected object labels"""
    with detection_lock:
        if not detected_objects:
            return ""
        labels = [obj['label'] for obj in detected_objects]
        return ", ".join(labels)

@server.register
def get_object_count():
    """Return number of objects currently detected"""
    with detection_lock:
        return len(detected_objects)

@server.register
def is_person_detected():
    """Return true if a person is detected"""
    with detection_lock:
        for obj in detected_objects:
            if obj['label'] == 'person':
                return True
        return False

@server.register
def get_person_confidence():
    """Return confidence score for person detection (0.0 if not detected)"""
    with detection_lock:
        for obj in detected_objects:
            if obj['label'] == 'person':
                return obj['confidence']
        return 0.0

@server.register
def is_object_detected(object_name):
    """Check if a specific object is detected"""
    with detection_lock:
        for obj in detected_objects:
            if obj['label'].lower() == object_name.lower():
                return True
        return False

@server.register
def get_all_detections():
    """Return detailed detection information as JSON string"""
    import json
    with detection_lock:
        return json.dumps(detected_objects)

if __name__ == "__main__":
    print("╔═══════════════════════════════════════════╗")
    print("║  Webcam Object Detection RPC Server      ║")
    print("╚═══════════════════════════════════════════╝")
    print()
    
    # Start camera thread
    cam_thread = threading.Thread(target=camera_thread, daemon=True)
    cam_thread.start()
    
    # Wait for camera to initialize
    time.sleep(2)
    
    if not camera_active:
        print("Failed to start camera. Exiting.")
        exit(1)
    
    print("RPC Server starting...")
    print("Registered methods:")
    print("  - get_detected_objects()")
    print("  - get_object_count()")
    print("  - is_person_detected()")
    print("  - get_person_confidence()")
    print("  - is_object_detected(object_name)")
    print("  - get_all_detections()")
    print("\nWaiting for RPC calls from STM32...\n")
    
    try:
        server.run()
    except KeyboardInterrupt:
        print("\nShutting down...")
        camera_active = False
        cam_thread.join(timeout=2)
        print("Goodbye!")

*/
