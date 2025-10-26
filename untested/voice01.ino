/*
 * Arduino UNO Q - Voice Controlled Robotics
 * 
 * This demonstrates something the Portenta H7 CANNOT do well:
 * Real-time speech recognition controlling motors/servos/LEDs
 * 
 * ADVANTAGES OVER PORTENTA H7:
 * - Full speech recognition (Vosk/Whisper on Snapdragon)
 * - No cloud needed - all processing local
 * - Natural language commands: "turn left", "stop", "move forward"
 * - STM32 handles real-time motor control
 * - Snapdragon does heavy ML processing
 * - Can add wake word detection ("Hey Robot")
 * 
 * HARDWARE SETUP:
 * - USB microphone or I2S mic connected to Snapdragon
 * - Servo on D2
 * - DC motor driver on D3, D4 (PWM and direction)
 * - LED indicators on D5, D6, D7
 * 
 * VOICE COMMANDS:
 * - "forward" / "go" - Move forward
 * - "back" / "backward" / "reverse" - Move backward
 * - "left" / "turn left" - Turn left
 * - "right" / "turn right" - Turn right
 * - "stop" - Stop all motors
 * - "lights on" - Turn on LEDs
 * - "lights off" - Turn off LEDs
 * - "servo left" / "servo right" / "servo center" - Control servo
 * 
 * INSTRUCTIONS:
 * 1. Upload this sketch to STM32
 * 2. Copy Python code and save as voice_control.py on Snapdragon
 * 3. Install dependencies (see Python section)
 * 4. Connect USB microphone to Snapdragon
 * 5. Run: python3 voice_control.py
 * 6. Start talking to control your robot!
 */

#include <Arduino_RouterBridge.h>
#include <Servo.h>

// Pin definitions
const int SERVO_PIN = 2;
const int MOTOR_PWM_PIN = 3;
const int MOTOR_DIR_PIN = 4;
const int LED_RED = 5;
const int LED_GREEN = 6;
const int LED_BLUE = 7;

// Objects
Servo myServo;

// State variables
int motorSpeed = 0;      // -255 to 255 (negative = reverse)
int servoPosition = 90;  // 0 to 180 degrees
bool lightsOn = false;
String lastCommand = "none";
unsigned long lastCommandTime = 0;

void setup() {
  Bridge.begin();
  Monitor.begin();
  
  // Setup pins
  pinMode(MOTOR_PWM_PIN, OUTPUT);
  pinMode(MOTOR_DIR_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  myServo.attach(SERVO_PIN);
  myServo.write(servoPosition);
  
  Monitor.println("╔════════════════════════════════════════════╗");
  Monitor.println("║  Voice Controlled Robotics System         ║");
  Monitor.println("║  Arduino UNO Q - STM32 + Snapdragon       ║");
  Monitor.println("╚════════════════════════════════════════════╝");
  Monitor.println();
  Monitor.println("🎤 Waiting for voice commands from Snapdragon...");
  Monitor.println();
  
  // Register RPC command handlers
  Bridge.provide_safe("execute_command", executeCommand);
  
  // Initial state
  stopMotor();
  setLights(false);
}

void loop() {
  // Auto-stop motor if no command received in 2 seconds (safety)
  if (motorSpeed != 0 && millis() - lastCommandTime > 2000) {
    Monitor.println("⚠️  Auto-stop: No command timeout");
    stopMotor();
  }
  
  delay(50);
}

// RPC function called by Snapdragon with voice command
String executeCommand(String command) {
  command.toLowerCase();
  command.trim();
  
  lastCommand = command;
  lastCommandTime = millis();
  
  Monitor.print("🎤 Voice Command: \"");
  Monitor.print(command);
  Monitor.println("\"");
  
  // Motor commands
  if (command == "forward" || command == "go" || command == "move forward") {
    moveForward();
    return "Moving forward";
  }
  else if (command == "back" || command == "backward" || command == "reverse") {
    moveBackward();
    return "Moving backward";
  }
  else if (command == "left" || command == "turn left") {
    turnLeft();
    return "Turning left";
  }
  else if (command == "right" || command == "turn right") {
    turnRight();
    return "Turning right";
  }
  else if (command == "stop" || command == "halt") {
    stopMotor();
    return "Stopped";
  }
  
  // Servo commands
  else if (command == "servo left") {
    servoPosition = 0;
    myServo.write(servoPosition);
    Monitor.println("  ↳ Servo: LEFT (0°)");
    return "Servo moved left";
  }
  else if (command == "servo right") {
    servoPosition = 180;
    myServo.write(servoPosition);
    Monitor.println("  ↳ Servo: RIGHT (180°)");
    return "Servo moved right";
  }
  else if (command == "servo center" || command == "servo middle") {
    servoPosition = 90;
    myServo.write(servoPosition);
    Monitor.println("  ↳ Servo: CENTER (90°)");
    return "Servo centered";
  }
  
  // Light commands
  else if (command == "lights on" || command == "light on") {
    setLights(true);
    return "Lights on";
  }
  else if (command == "lights off" || command == "light off") {
    setLights(false);
    return "Lights off";
  }
  
  // Unknown command
  else {
    Monitor.println("  ↳ Unknown command");
    return "Unknown command";
  }
}

void moveForward() {
  motorSpeed = 200;
  digitalWrite(MOTOR_DIR_PIN, HIGH);
  analogWrite(MOTOR_PWM_PIN, motorSpeed);
  Monitor.println("  ↳ Motor: FORWARD");
}

void moveBackward() {
  motorSpeed = -200;
  digitalWrite(MOTOR_DIR_PIN, LOW);
  analogWrite(MOTOR_PWM_PIN, abs(motorSpeed));
  Monitor.println("  ↳ Motor: BACKWARD");
}

void turnLeft() {
  motorSpeed = 150;
  digitalWrite(MOTOR_DIR_PIN, HIGH);
  analogWrite(MOTOR_PWM_PIN, motorSpeed);
  servoPosition = 45;
  myServo.write(servoPosition);
  Monitor.println("  ↳ Motor: FORWARD-LEFT");
}

void turnRight() {
  motorSpeed = 150;
  digitalWrite(MOTOR_DIR_PIN, HIGH);
  analogWrite(MOTOR_PWM_PIN, motorSpeed);
  servoPosition = 135;
  myServo.write(servoPosition);
  Monitor.println("  ↳ Motor: FORWARD-RIGHT");
}

void stopMotor() {
  motorSpeed = 0;
  analogWrite(MOTOR_PWM_PIN, 0);
  Monitor.println("  ↳ Motor: STOPPED");
}

void setLights(bool on) {
  lightsOn = on;
  digitalWrite(LED_RED, on);
  digitalWrite(LED_GREEN, on);
  digitalWrite(LED_BLUE, on);
  Monitor.print("  ↳ Lights: ");
  Monitor.println(on ? "ON" : "OFF");
}

/*
==============================================================================
PYTHON CODE FOR SNAPDRAGON SIDE - Copy everything below this line
Save as: voice_control.py
==============================================================================

#!/usr/bin/env python3
"""
Voice Control System for Arduino UNO Q
Uses Vosk for offline speech recognition on Snapdragon
Sends recognized commands to STM32 via RPC

This is something the Portenta H7 CANNOT do effectively!

Installation:
  # Audio processing
  pip3 install pyaudio
  pip3 install vosk
  pip3 install arduino-rpc
  
  # Download Vosk model (small English model ~40MB)
  wget https://alphacephei.com/vosk/models/vosk-model-small-en-us-0.15.zip
  unzip vosk-model-small-en-us-0.15.zip
  mv vosk-model-small-en-us-0.15 model

Alternative Installation (if above doesn't work):
  # Use pocketsphinx (lighter weight, less accurate)
  pip3 install SpeechRecognition
  pip3 install pocketsphinx

Usage:
  python3 voice_control.py
  
Then just speak commands:
  "forward", "stop", "turn left", "lights on", etc.
"""

import json
import pyaudio
from vosk import Model, KaldiRecognizer
from arduino_rpc import RpcClient
import sys
import threading
import queue

# Configuration
SAMPLE_RATE = 16000
MODEL_PATH = "model"  # Path to Vosk model directory

# Command queue
command_queue = queue.Queue()

class VoiceRecognizer:
    def __init__(self, model_path):
        print("Loading speech recognition model...")
        try:
            self.model = Model(model_path)
            self.recognizer = KaldiRecognizer(self.model, SAMPLE_RATE)
            self.recognizer.SetWords(True)
            print("✓ Model loaded successfully")
        except Exception as e:
            print(f"ERROR loading model: {e}")
            print("Please download the Vosk model first (see installation instructions)")
            sys.exit(1)
        
        # Initialize audio
        print("Initializing microphone...")
        self.audio = pyaudio.PyAudio()
        try:
            self.stream = self.audio.open(
                format=pyaudio.paInt16,
                channels=1,
                rate=SAMPLE_RATE,
                input=True,
                frames_per_buffer=4096
            )
            print("✓ Microphone initialized")
        except Exception as e:
            print(f"ERROR: Could not access microphone: {e}")
            sys.exit(1)
    
    def listen(self):
        """Continuously listen and recognize speech"""
        print("\n🎤 Listening for commands... (Speak now!)")
        print("Commands: forward, back, left, right, stop, lights on/off, servo left/right/center")
        print("-" * 60)
        
        while True:
            data = self.stream.read(4096, exception_on_overflow=False)
            
            if self.recognizer.AcceptWaveform(data):
                result = json.loads(self.recognizer.Result())
                text = result.get('text', '')
                
                if text:
                    print(f"\n🗣️  Heard: \"{text}\"")
                    command_queue.put(text)
    
    def close(self):
        self.stream.stop_stream()
        self.stream.close()
        self.audio.terminate()

def rpc_commander():
    """Thread that sends commands to STM32 via RPC"""
    print("Connecting to STM32...")
    
    try:
        client = RpcClient()
        print("✓ Connected to STM32\n")
    except Exception as e:
        print(f"ERROR: Could not connect to STM32: {e}")
        sys.exit(1)
    
    while True:
        try:
            # Get command from queue (blocking)
            command = command_queue.get()
            
            # Send to STM32
            print(f"📤 Sending command: \"{command}\"")
            response = client.call('execute_command', command)
            print(f"📥 STM32 response: {response}")
            print("-" * 60)
            
        except Exception as e:
            print(f"ERROR sending command: {e}")

def main():
    print("╔════════════════════════════════════════════╗")
    print("║  Voice Control System - Snapdragon Side   ║")
    print("║  Offline Speech Recognition + RPC         ║")
    print("╚════════════════════════════════════════════╝")
    print()
    
    # Start RPC commander thread
    rpc_thread = threading.Thread(target=rpc_commander, daemon=True)
    rpc_thread.start()
    
    # Start voice recognition (main thread)
    recognizer = VoiceRecognizer(MODEL_PATH)
    
    try:
        recognizer.listen()
    except KeyboardInterrupt:
        print("\n\nShutting down...")
        recognizer.close()
        print("Goodbye!")

if __name__ == "__main__":
    main()

# ALTERNATIVE: Using SpeechRecognition library (easier but requires internet)
"""
import speech_recognition as sr
from arduino_rpc import RpcClient

client = RpcClient()
recognizer = sr.Recognizer()
mic = sr.Microphone()

print("🎤 Listening...")

while True:
    try:
        with mic as source:
            recognizer.adjust_for_ambient_noise(source, duration=0.5)
            audio = recognizer.listen(source, timeout=5)
        
        text = recognizer.recognize_sphinx(audio)  # Offline
        # text = recognizer.recognize_google(audio)  # Online (more accurate)
        
        if text:
            print(f"Heard: {text}")
            response = client.call('execute_command', text)
            print(f"Response: {response}")
    
    except sr.WaitTimeoutError:
        pass
    except Exception as e:
        print(f"Error: {e}")
"""

*/
