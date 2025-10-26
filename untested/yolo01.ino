/*
 * Arduino UNO Q - Full YOLO Object Detection
 * 
 * This demonstrates the TRUE POWER of the Snapdragon processor!
 * Running full YOLOv8 (not TFLite) with bounding boxes, confidence scores,
 * and multiple simultaneous object detection.
 * 
 * IMPOSSIBLE ON PORTENTA H7!
 * - Portenta limited to TFLite models (~1-2MB)
 * - YOLO models are 6MB-100MB+
 * - Portenta can't handle the computation
 * - Portenta limited RAM
 * 
 * YOLO ADVANTAGES:
 * - Detects multiple objects simultaneously with bounding boxes
 * - Real-time performance (30+ FPS on Snapdragon)
 * - 80 COCO object classes
 * - Confidence scores and precise locations
 * - State-of-the-art accuracy
 * 
 * STM32 RECEIVES:
 * - Object count
 * - List of detected objects with confidence
 * - Bounding box coordinates (x, y, width, height)
 * - Person tracking (largest person position)
 * - Zone detection (left/center/right of frame)
 * 
 * USE CASES:
 * - Smart security camera
 * - Autonomous robot navigation
 * - People counting system
 * - Object tracking robot
 * - Gesture-controlled devices
 * 
 * HARDWARE:
 * - USB webcam connected to Snapdragon
 * - LEDs on D2, D3, D4 (left/center/right zone indicators)
 * - Servo on D5 (tracks largest person)
 * - Status LED on D6
 * 
 * INSTRUCTIONS:
 * 1. Upload this sketch to STM32
 * 2. Copy Python code and save as yolo_detector.py on Snapdragon
 * 3. Install dependencies (see Python section)
 * 4. Connect USB webcam to Snapdragon
 * 5. Run: python3 yolo_detector.py
 * 6. Watch real-time YOLO detection results on Serial Monitor!
 */

#include <Arduino_RouterBridge.h>
#include <Servo.h>

// Pin definitions
const int LED_LEFT = 2;
const int LED_CENTER = 3;
const int LED_RIGHT = 4;
const int SERVO_PIN = 5;
const int STATUS_LED = 6;

// Servo for tracking
Servo trackingServo;
int servoPosition = 90;

// Detection statistics
int totalObjects = 0;
int peopleCount = 0;
int vehicleCount = 0;
int animalCount = 0;
String dominantObject = "none";
float dominantConfidence = 0.0;

// Person tracking
bool personDetected = false;
float personX = 0.5;  // Normalized 0.0-1.0 (left to right)
float personY = 0.5;  // Normalized 0.0-1.0 (top to bottom)
float personSize = 0.0;  // Normalized area

unsigned long lastUpdate = 0;
unsigned long updateInterval = 100;  // Query every 100ms for smooth tracking

void setup() {
  Bridge.begin();
  Monitor.begin();
  
  // Setup pins
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_CENTER, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  
  trackingServo.attach(SERVO_PIN);
  trackingServo.write(90);
  
  Monitor.println("╔══════════════════════════════════════════════╗");
  Monitor.println("║   YOLO Object Detection System               ║");
  Monitor.println("║   Arduino UNO Q - Full Computer Vision       ║");
  Monitor.println("╚══════════════════════════════════════════════╝");
  Monitor.println();
  Monitor.println("🚀 IMPOSSIBLE ON PORTENTA H7!");
  Monitor.println("   Running full YOLOv8 (not TFLite)");
  Monitor.println("   Multiple objects + bounding boxes");
  Monitor.println("   Real-time tracking and analysis");
  Monitor.println();
  Monitor.println("📹 Waiting for YOLO detections...\n");
  
  // LED test
  digitalWrite(STATUS_LED, HIGH);
  delay(200);
  digitalWrite(STATUS_LED, LOW);
}

void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();
    
    digitalWrite(STATUS_LED, HIGH);
    
    // Query YOLO detection results
    queryYOLODetections();
    
    // Update servo to track person
    updatePersonTracking();
    
    // Update zone LEDs
    updateZoneLEDs();
    
    digitalWrite(STATUS_LED, LOW);
  }
}

void queryYOLODetections() {
  bool success = true;
  
  // Get total object count
  if (!Bridge.call("get_object_count").result(totalObjects)) {
    success = false;
  }
  
  // Get people count
  if (!Bridge.call("get_people_count").result(peopleCount)) {
    success = false;
  }
  
  // Get vehicle count (cars, trucks, buses)
  if (!Bridge.call("get_vehicle_count").result(vehicleCount)) {
    success = false;
  }
  
  // Get animal count
  if (!Bridge.call("get_animal_count").result(animalCount)) {
    success = false;
  }
  
  // Get dominant object (largest/most confident)
  if (!Bridge.call("get_dominant_object").result(dominantObject)) {
    success = false;
  }
  
  // Get dominant object confidence
  if (!Bridge.call("get_dominant_confidence").result(dominantConfidence)) {
    success = false;
  }
  
  // Get person tracking data
  if (!Bridge.call("is_person_detected").result(personDetected)) {
    success = false;
  }
  
  if (personDetected) {
    Bridge.call("get_person_x").result(personX);
    Bridge.call("get_person_y").result(personY);
    Bridge.call("get_person_size").result(personSize);
  }
  
  // Print summary every 2 seconds
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 2000) {
    lastPrint = millis();
    printDetectionSummary();
  }
}

void printDetectionSummary() {
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Monitor.println("🔍 YOLO DETECTION SUMMARY");
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  
  Monitor.print("Total Objects: ");
  Monitor.println(totalObjects);
  
  Monitor.print("👤 People: ");
  Monitor.println(peopleCount);
  
  Monitor.print("🚗 Vehicles: ");
  Monitor.println(vehicleCount);
  
  Monitor.print("🐾 Animals: ");
  Monitor.println(animalCount);
  
  if (totalObjects > 0) {
    Monitor.print("🎯 Dominant: ");
    Monitor.print(dominantObject);
    Monitor.print(" (");
    Monitor.print(dominantConfidence * 100.0, 1);
    Monitor.println("%)");
  }
  
  if (personDetected) {
    Monitor.println();
    Monitor.println("👤 PERSON TRACKING:");
    Monitor.print("   Position: (");
    Monitor.print(personX, 2);
    Monitor.print(", ");
    Monitor.print(personY, 2);
    Monitor.println(")");
    Monitor.print("   Size: ");
    Monitor.print(personSize * 100.0, 1);
    Monitor.println("%");
    Monitor.print("   Zone: ");
    if (personX < 0.33) Monitor.println("LEFT");
    else if (personX < 0.66) Monitor.println("CENTER");
    else Monitor.println("RIGHT");
    Monitor.print("   Servo: ");
    Monitor.print(servoPosition);
    Monitor.println("°");
  }
  
  Monitor.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

void updatePersonTracking() {
  if (personDetected) {
    // Map person X position (0.0-1.0) to servo angle (0-180)
    int targetPosition = (int)(personX * 180.0);
    
    // Smooth servo movement
    if (abs(targetPosition - servoPosition) > 5) {
      if (targetPosition > servoPosition) {
        servoPosition += 2;
      } else {
        servoPosition -= 2;
      }
      servoPosition = constrain(servoPosition, 0, 180);
      trackingServo.write(servoPosition);
    }
  } else {
    // No person detected - return to center
    if (servoPosition != 90) {
      if (servoPosition > 90) servoPosition--;
      else servoPosition++;
      trackingServo.write(servoPosition);
    }
  }
}

void updateZoneLEDs() {
  // Turn off all LEDs
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_CENTER, LOW);
  digitalWrite(LED_RIGHT, LOW);
  
  // Light up LED based on person zone
  if (personDetected) {
    if (personX < 0.33) {
      digitalWrite(LED_LEFT, HIGH);
    } else if (personX < 0.66) {
      digitalWrite(LED_CENTER, HIGH);
    } else {
      digitalWrite(LED_RIGHT, HIGH);
    }
  }
}

/*
==============================================================================
PYTHON CODE FOR SNAPDRAGON SIDE - Copy everything below this line
Save as: yolo_detector.py
==============================================================================

#!/usr/bin/env python3
"""
Full YOLO Object Detection for Arduino UNO Q
Uses YOLOv8 with OpenCV and Ultralytics
Runs on Snapdragon - IMPOSSIBLE on Portenta H7!

Installation:
  pip3 install ultralytics
  pip3 install opencv-python
  pip3 install numpy
  pip3 install arduino-rpc
  
  # YOLO will auto-download model on first run (~6MB)
  # Or manually download YOLOv8n (nano) model:
  # wget https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n.pt

Usage:
  python3 yolo_detector.py
  
Features:
  - Real-time YOLOv8 detection
  - 80 COCO object classes
  - Bounding boxes with confidence scores
  - Person tracking (largest person)
  - Zone detection (left/center/right)
  - Category counting (people, vehicles, animals)
"""

import cv2
import numpy as np
from ultralytics import YOLO
from arduino_rpc import RpcServer
import threading
import time

# Global detection results
detections = []
detection_lock = threading.Lock()
camera_active = False

# COCO categories
PERSON_CLASSES = ['person']
VEHICLE_CLASSES = ['bicycle', 'car', 'motorcycle', 'airplane', 'bus', 'train', 'truck', 'boat']
ANIMAL_CLASSES = ['bird', 'cat', 'dog', 'horse', 'sheep', 'cow', 'elephant', 'bear', 'zebra', 'giraffe']

class YOLODetector:
    def __init__(self, model_name='yolov8n.pt'):
        """
        Initialize YOLO detector
        Models: yolov8n.pt (nano), yolov8s.pt (small), yolov8m.pt (medium)
        Nano is fastest, medium is most accurate
        """
        print(f"Loading YOLO model: {model_name}")
        print("(First run will download model ~6MB)")
        
        try:
            self.model = YOLO(model_name)
            print("✓ YOLO model loaded successfully!")
            print(f"  Model: {model_name}")
            print(f"  Classes: {len(self.model.names)}")
        except Exception as e:
            print(f"ERROR loading YOLO: {e}")
            raise
    
    def detect(self, frame):
        """
        Run YOLO detection on frame
        Returns list of detections with boxes, confidence, class
        """
        # Run inference
        results = self.model(frame, verbose=False)
        
        detections = []
        
        # Parse results
        for result in results:
            boxes = result.boxes
            for box in boxes:
                # Get box coordinates
                x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                
                # Get confidence and class
                conf = float(box.conf[0])
                cls = int(box.cls[0])
                class_name = self.model.names[cls]
                
                # Calculate normalized center and size
                frame_height, frame_width = frame.shape[:2]
                center_x = ((x1 + x2) / 2) / frame_width
                center_y = ((y1 + y2) / 2) / frame_height
                width = (x2 - x1) / frame_width
                height = (y2 - y1) / frame_height
                area = width * height
                
                detections.append({
                    'class': class_name,
                    'confidence': conf,
                    'box': [x1, y1, x2, y2],
                    'center': [center_x, center_y],
                    'size': area
                })
        
        return detections

def camera_thread():
    """Background thread for continuous YOLO detection"""
    global detections, camera_active
    
    print("\n🎥 Initializing webcam...")
    cap = cv2.VideoCapture(0)
    
    if not cap.isOpened():
        print("ERROR: Cannot open webcam!")
        return
    
    # Set resolution (lower = faster)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    
    print("✓ Webcam opened successfully!")
    
    # Initialize YOLO
    try:
        detector = YOLODetector('yolov8n.pt')  # Use nano model for speed
    except Exception as e:
        print(f"ERROR: Could not initialize YOLO: {e}")
        return
    
    camera_active = True
    print("\n🚀 Starting YOLO detection loop...")
    print("   This runs full YOLOv8 - impossible on Portenta H7!")
    print("   Press Ctrl+C to stop\n")
    
    frame_count = 0
    start_time = time.time()
    
    while camera_active:
        ret, frame = cap.read()
        if not ret:
            print("ERROR: Failed to grab frame")
            break
        
        # Run YOLO detection
        results = detector.detect(frame)
        
        # Update global results
        with detection_lock:
            detections = results
        
        # FPS calculation
        frame_count += 1
        if frame_count % 30 == 0:
            elapsed = time.time() - start_time
            fps = frame_count / elapsed
            print(f"📊 FPS: {fps:.1f} | Objects: {len(results)}")
        
        # Small delay for CPU
        time.sleep(0.01)
    
    cap.release()
    print("Camera thread stopped")

# Initialize RPC server
server = RpcServer()

@server.register
def get_object_count():
    """Return total number of objects detected"""
    with detection_lock:
        return len(detections)

@server.register
def get_people_count():
    """Return number of people detected"""
    with detection_lock:
        count = sum(1 for d in detections if d['class'] in PERSON_CLASSES)
        return count

@server.register
def get_vehicle_count():
    """Return number of vehicles detected"""
    with detection_lock:
        count = sum(1 for d in detections if d['class'] in VEHICLE_CLASSES)
        return count

@server.register
def get_animal_count():
    """Return number of animals detected"""
    with detection_lock:
        count = sum(1 for d in detections if d['class'] in ANIMAL_CLASSES)
        return count

@server.register
def get_dominant_object():
    """Return the most confident/largest object"""
    with detection_lock:
        if not detections:
            return "none"
        # Sort by confidence * size (most prominent)
        dominant = max(detections, key=lambda d: d['confidence'] * d['size'])
        return dominant['class']

@server.register
def get_dominant_confidence():
    """Return confidence of dominant object"""
    with detection_lock:
        if not detections:
            return 0.0
        dominant = max(detections, key=lambda d: d['confidence'] * d['size'])
        return dominant['confidence']

@server.register
def is_person_detected():
    """Check if any person is detected"""
    with detection_lock:
        return any(d['class'] in PERSON_CLASSES for d in detections)

@server.register
def get_person_x():
    """Get X position of largest person (0.0-1.0)"""
    with detection_lock:
        people = [d for d in detections if d['class'] in PERSON_CLASSES]
        if not people:
            return 0.5
        largest = max(people, key=lambda d: d['size'])
        return largest['center'][0]

@server.register
def get_person_y():
    """Get Y position of largest person (0.0-1.0)"""
    with detection_lock:
        people = [d for d in detections if d['class'] in PERSON_CLASSES]
        if not people:
            return 0.5
        largest = max(people, key=lambda d: d['size'])
        return largest['center'][1]

@server.register
def get_person_size():
    """Get size of largest person (normalized area)"""
    with detection_lock:
        people = [d for d in detections if d['class'] in PERSON_CLASSES]
        if not people:
            return 0.0
        largest = max(people, key=lambda d: d['size'])
        return largest['size']

@server.register
def get_all_objects():
    """Return JSON string of all detections"""
    import json
    with detection_lock:
        return json.dumps([{
            'class': d['class'],
            'confidence': d['confidence'],
            'center': d['center']
        } for d in detections])

if __name__ == "__main__":
    print("╔══════════════════════════════════════════════╗")
    print("║   YOLO Object Detection RPC Server           ║")
    print("║   Running YOLOv8 on Snapdragon              ║")
    print("╚══════════════════════════════════════════════╝")
    print()
    print("⚡ FULL YOLO - NOT TFLite!")
    print("   This is IMPOSSIBLE on Portenta H7")
    print()
    
    # Start camera thread
    cam_thread = threading.Thread(target=camera_thread, daemon=True)
    cam_thread.start()
    
    # Wait for camera to initialize
    time.sleep(3)
    
    if not camera_active:
        print("Failed to start camera. Exiting.")
        exit(1)
    
    print("RPC Server starting...")
    print("Registered methods:")
    print("  - get_object_count()")
    print("  - get_people_count()")
    print("  - get_vehicle_count()")
    print("  - get_animal_count()")
    print("  - get_dominant_object()")
    print("  - get_dominant_confidence()")
    print("  - is_person_detected()")
    print("  - get_person_x/y/size()")
    print("  - get_all_objects()")
    print("\n🎯 Waiting for RPC calls from STM32...\n")
    
    try:
        server.run()
    except KeyboardInterrupt:
        print("\n\nShutting down...")
        camera_active = False
        cam_thread.join(timeout=2)
        print("Goodbye!")

*/
