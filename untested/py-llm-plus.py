#!/usr/bin/env python3
"""
Arduino UNO Q - ML-Powered Random Sketch Generator
===================================================

This script uses a Large Language Model to automatically generate and deploy
random Arduino sketches to the STM32 side of the Arduino UNO Q every 2 minutes.

INSTALLATION INSTRUCTIONS:
==========================

1. INSTALL OLLAMA (Recommended for local LLM):
   On the Arduino UNO Q (Debian Linux), run:
   
   curl -fsSL https://ollama.com/install.sh | sh
   
   Official docs: https://ollama.com/download/linux

2. MOUNT USB DRIVE (if using external storage for models):
   
   # Check USB device name (usually /dev/sda1 or /dev/sdb1)
   lsblk
   
   # Create mount point
   sudo mkdir -p /mnt/usbdrive
   
   # Mount the drive
   sudo mount /dev/sda1 /mnt/usbdrive
   
   # Set Ollama model directory to USB drive
   export OLLAMA_MODELS=/mnt/usbdrive/ollama_models
   mkdir -p $OLLAMA_MODELS

3. DOWNLOAD A CODE-GENERATION MODEL:
   
   # Small model (~2GB, fits on internal storage):
   ollama pull tinyllama
   
   # Medium model (~4GB, recommended for USB drive):
   ollama pull codellama:7b-code
   
   # Larger, better model (~8GB, requires USB drive):
   ollama pull deepseek-coder:6.7b
   
   Available models: https://ollama.com/library

4. INSTALL PYTHON DEPENDENCIES:
   
   pip3 install requests
   
   # If using OpenAI API instead of local model:
   pip3 install openai

5. RUN THIS SCRIPT:
   
   python3 ml_sketch_generator.py

CONFIGURATION OPTIONS:
======================
- Change GENERATION_INTERVAL_SECONDS to adjust how often new sketches are generated
- Set USE_LOCAL_MODEL to False to use OpenAI API instead
- Modify MODEL_NAME to use different Ollama models
- Adjust SAFETY_KEYWORDS to filter out dangerous code patterns

"""

import json
import os
import subprocess
import sys
import time
import re
from pathlib import Path
from datetime import datetime

# ============================================================================
# CONFIGURATION
# ============================================================================

# Generation settings
GENERATION_INTERVAL_SECONDS = 120  # Generate new sketch every 2 minutes
OUTPUT_BASE_DIR = "generated_sketches"  # Base directory for all sketches

# LLM Configuration
USE_LOCAL_MODEL = True  # Set to False to use OpenAI API
MODEL_NAME = "codellama:7b-code"  # Ollama model name
OLLAMA_API_URL = "http://localhost:11434/api/generate"  # Ollama API endpoint

# OpenAI Configuration (if USE_LOCAL_MODEL = False)
OPENAI_API_KEY = os.environ.get("OPENAI_API_KEY", "your-api-key-here")
OPENAI_MODEL = "gpt-4"

# Safety configuration
SAFETY_KEYWORDS = [
    "EEPROM.write",  # Avoid excessive EEPROM writes
    "pinMode(0",     # Avoid messing with RX/TX pins
    "pinMode(1",
    "while(true)",   # Avoid infinite loops without delay
    "while(1)",
]

# Arduino constraints
MAX_SKETCH_LINES = 100  # Maximum lines of code
SAFE_PINS = [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13]  # Safe digital pins to use

# ============================================================================
# LLM INTERACTION FUNCTIONS
# ============================================================================

def generate_sketch_prompt():
    """Create a prompt for the LLM to generate an Arduino sketch."""
    
    sketch_types = [
        "LED blink pattern with varying speeds",
        "LED chaser pattern across multiple pins",
        "LED brightness fade using PWM",
        "Random LED pattern",
        "Binary counter display using LEDs",
        "Knight Rider style LED sweep",
        "Morse code SOS pattern",
        "Traffic light simulation",
        "LED breathing effect",
        "Simple LED game pattern"
    ]
    
    # Pick a random sketch type
    import random
    sketch_type = random.choice(sketch_types)
    
    prompt = f"""Generate a complete Arduino sketch that implements: {sketch_type}

Requirements:
- Must be a complete, valid Arduino sketch with setup() and loop() functions
- Only use digital pins: {', '.join(map(str, SAFE_PINS))}
- Include comments explaining what the code does
- Keep it under {MAX_SKETCH_LINES} lines
- Use only standard Arduino functions (digitalWrite, pinMode, delay, analogWrite, millis)
- No serial communication
- No EEPROM writes
- Include appropriate delays to make effects visible
- Must be safe and non-destructive

Generate ONLY the Arduino code, no explanations before or after the code."""
    
    return prompt

def query_ollama(prompt: str) -> str:
    """Query local Ollama model for code generation."""
    try:
        import requests
        
        payload = {
            "model": MODEL_NAME,
            "prompt": prompt,
            "stream": False,
            "temperature": 0.8,  # Some creativity
            "max_tokens": 1000
        }
        
        print(f"🤖 Querying local Ollama model: {MODEL_NAME}...")
        response = requests.post(OLLAMA_API_URL, json=payload, timeout=60)
        
        if response.status_code == 200:
            result = response.json()
            return result.get("response", "")
        else:
            print(f"❌ Ollama API error: {response.status_code}")
            return None
            
    except Exception as e:
        print(f"❌ Error querying Ollama: {e}")
        return None

def query_openai(prompt: str) -> str:
    """Query OpenAI API for code generation."""
    try:
        from openai import OpenAI
        
        client = OpenAI(api_key=OPENAI_API_KEY)
        
        print(f"🤖 Querying OpenAI model: {OPENAI_MODEL}...")
        response = client.chat.completions.create(
            model=OPENAI_MODEL,
            messages=[
                {"role": "system", "content": "You are an Arduino code generator. Generate only valid Arduino C++ code with no explanations."},
                {"role": "user", "content": prompt}
            ],
            temperature=0.8,
            max_tokens=1000
        )
        
        return response.choices[0].message.content
        
    except Exception as e:
        print(f"❌ Error querying OpenAI: {e}")
        return None

def generate_sketch_with_llm() -> str:
    """Generate an Arduino sketch using the configured LLM."""
    prompt = generate_sketch_prompt()
    
    if USE_LOCAL_MODEL:
        code = query_ollama(prompt)
    else:
        code = query_openai(prompt)
    
    if code:
        # Extract code from markdown code blocks if present
        code = extract_code_from_markdown(code)
        return code
    
    return None

def extract_code_from_markdown(text: str) -> str:
    """Extract Arduino code from markdown code blocks."""
    # Look for code blocks
    code_block_pattern = r"```(?:cpp|c|arduino)?\s*\n(.*?)\n```"
    matches = re.findall(code_block_pattern, text, re.DOTALL)
    
    if matches:
        return matches[0].strip()
    
    # If no code blocks, return the text as-is
    return text.strip()

# ============================================================================
# SAFETY AND VALIDATION
# ============================================================================

def validate_sketch_safety(code: str) -> tuple[bool, str]:
    """Check if generated sketch is safe to deploy."""
    
    # Check for dangerous keywords
    for keyword in SAFETY_KEYWORDS:
        if keyword in code:
            return False, f"Contains dangerous keyword: {keyword}"
    
    # Check if it has required functions
    if "void setup()" not in code and "void setup ()" not in code:
        return False, "Missing setup() function"
    
    if "void loop()" not in code and "void loop ()" not in code:
        return False, "Missing loop() function"
    
    # Check line count
    line_count = len(code.split('\n'))
    if line_count > MAX_SKETCH_LINES:
        return False, f"Too many lines: {line_count} (max: {MAX_SKETCH_LINES})"
    
    # Check for while loops without delays (potential hang)
    while_pattern = r"while\s*\([^)]+\)\s*{[^}]*}"
    while_matches = re.findall(while_pattern, code, re.DOTALL)
    for match in while_matches:
        if "delay" not in match.lower() and "millis" not in match.lower():
            return False, "Contains while loop without delay (potential hang)"
    
    return True, "Safe"

# ============================================================================
# SKETCH DEPLOYMENT
# ============================================================================

def create_sketch_directory(sketch_number: int) -> Path:
    """Create a directory for the generated sketch."""
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    dir_name = f"sketch_{sketch_number:04d}_{timestamp}"
    sketch_dir = Path(OUTPUT_BASE_DIR) / dir_name
    sketch_dir.mkdir(parents=True, exist_ok=True)
    return sketch_dir

def write_sketch_files(sketch_dir: Path, code: str, sketch_number: int):
    """Write the Arduino sketch and metadata to disk."""
    
    # Create sketch subdirectory
    sketch_subdir = sketch_dir / "sketch"
    sketch_subdir.mkdir(exist_ok=True)
    
    # Write Arduino sketch
    sketch_file = sketch_subdir / "sketch.ino"
    sketch_file.write_text(code)
    print(f"✅ Written sketch to: {sketch_file}")
    
    # Write metadata
    metadata = {
        "sketch_number": sketch_number,
        "generated_at": datetime.now().isoformat(),
        "model": MODEL_NAME if USE_LOCAL_MODEL else OPENAI_MODEL,
        "local_model": USE_LOCAL_MODEL,
        "line_count": len(code.split('\n'))
    }
    
    metadata_file = sketch_dir / "metadata.json"
    metadata_file.write_text(json.dumps(metadata, indent=2))
    
    # Write App Lab configuration files
    write_applab_config(sketch_dir, sketch_number)
    
    return sketch_file

def write_applab_config(sketch_dir: Path, sketch_number: int):
    """Write App Lab configuration files."""
    
    # brick_config.yaml
    config = f"""name: ml_sketch_{sketch_number:04d}
version: "1.0.0"
description: "AI-generated Arduino sketch #{sketch_number}"
author: "ML Generator"
"""
    (sketch_dir / "brick_config.yaml").write_text(config)
    
    # brick_compose.yaml
    compose = """version: '3.8'
services:
  # No additional services needed
"""
    (sketch_dir / "brick_compose.yaml").write_text(compose)
    
    # __init__.py
    (sketch_dir / "__init__.py").write_text("# ML-generated sketch\n")
    
    # README.md
    readme = f"""# ML-Generated Sketch #{sketch_number}

Auto-generated by ML sketch generator.

Generated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
"""
    (sketch_dir / "README.md").write_text(readme)

def deploy_sketch(sketch_dir: Path):
    """Attempt to deploy the sketch using App Lab CLI."""
    try:
        result = subprocess.run(
            ["applab", "deploy", str(sketch_dir.absolute())],
            capture_output=True,
            text=True,
            timeout=30,
            check=False
        )
        
        if result.returncode == 0:
            print("✅ Successfully deployed to STM32!")
            return True
        else:
            print(f"⚠️  Deployment failed: {result.stderr}")
            return False
            
    except subprocess.TimeoutExpired:
        print("⚠️  Deployment timed out")
        return False
    except Exception as e:
        print(f"⚠️  Could not deploy: {e}")
        return False

# ============================================================================
# MAIN LOOP
# ============================================================================

def check_dependencies():
    """Check if required dependencies are available."""
    print("🔍 Checking dependencies...")
    
    # Check Ollama
    if USE_LOCAL_MODEL:
        try:
            result = subprocess.run(
                ["ollama", "list"],
                capture_output=True,
                timeout=5,
                check=False
            )
            if result.returncode == 0:
                print("✅ Ollama is installed")
                if MODEL_NAME in result.stdout.decode():
                    print(f"✅ Model '{MODEL_NAME}' is available")
                else:
                    print(f"⚠️  Model '{MODEL_NAME}' not found. Download with:")
                    print(f"   ollama pull {MODEL_NAME}")
                    return False
            else:
                print("❌ Ollama not found. Install from: https://ollama.com/download")
                return False
        except:
            print("❌ Ollama not found. Install from: https://ollama.com/download")
            return False
    
    # Check App Lab CLI
    try:
        result = subprocess.run(
            ["applab", "--version"],
            capture_output=True,
            timeout=5,
            check=False
        )
        if result.returncode == 0:
            print("✅ App Lab CLI is available")
        else:
            print("⚠️  App Lab CLI not found (sketches will be saved but not deployed)")
    except:
        print("⚠️  App Lab CLI not found (sketches will be saved but not deployed)")
    
    return True

def main():
    """Main generation loop."""
    print("=" * 70)
    print("Arduino UNO Q - ML-Powered Random Sketch Generator")
    print("=" * 70)
    print()
    print(f"Configuration:")
    print(f"  - Generation interval: {GENERATION_INTERVAL_SECONDS} seconds")
    print(f"  - Output directory: {OUTPUT_BASE_DIR}/")
    print(f"  - Using local model: {USE_LOCAL_MODEL}")
    if USE_LOCAL_MODEL:
        print(f"  - Model: {MODEL_NAME}")
    else:
        print(f"  - Model: {OPENAI_MODEL}")
    print()
    
    # Check dependencies
    if not check_dependencies():
        print("\n❌ Missing required dependencies. Please see installation instructions at the top of this script.")
        sys.exit(1)
    
    print("\n🚀 Starting generation loop...")
    print(f"   New sketch will be generated every {GENERATION_INTERVAL_SECONDS} seconds")
    print(f"   Press Ctrl+C to stop\n")
    
    sketch_number = 1
    
    try:
        while True:
            print("\n" + "=" * 70)
            print(f"📝 Generating sketch #{sketch_number}...")
            print("=" * 70)
            
            # Generate sketch with LLM
            code = generate_sketch_with_llm()
            
            if not code:
                print("❌ Failed to generate sketch, will retry next cycle")
                time.sleep(GENERATION_INTERVAL_SECONDS)
                continue
            
            print(f"✅ Generated {len(code.split(chr(10)))} lines of code")
            
            # Validate safety
            is_safe, reason = validate_sketch_safety(code)
            
            if not is_safe:
                print(f"⚠️  Sketch failed safety check: {reason}")
                print(f"   Skipping this sketch, will try again next cycle")
                time.sleep(GENERATION_INTERVAL_SECONDS)
                continue
            
            print("✅ Sketch passed safety checks")
            
            # Create directory and write files
            sketch_dir = create_sketch_directory(sketch_number)
            write_sketch_files(sketch_dir, code, sketch_number)
            
            # Deploy to STM32
            print(f"\n🚀 Deploying sketch #{sketch_number}...")
            deploy_sketch(sketch_dir)
            
            print(f"\n✨ Sketch #{sketch_number} complete!")
            print(f"📂 Files saved to: {sketch_dir.absolute()}")
            
            sketch_number += 1
            
            # Wait for next cycle
            print(f"\n⏳ Waiting {GENERATION_INTERVAL_SECONDS} seconds until next sketch...")
            time.sleep(GENERATION_INTERVAL_SECONDS)
            
    except KeyboardInterrupt:
        print("\n\n👋 Stopped by user")
        print(f"Generated {sketch_number - 1} sketches total")
        print(f"All sketches saved in: {Path(OUTPUT_BASE_DIR).absolute()}")

if __name__ == "__main__":
    main()
