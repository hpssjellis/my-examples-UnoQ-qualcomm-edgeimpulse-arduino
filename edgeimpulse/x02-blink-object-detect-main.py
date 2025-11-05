import time
from datetime import datetime, timezone
from arduino.app_utils import App
from arduino.app_bricks.web_ui import WebUI
from arduino.app_bricks.video_objectdetection import VideoObjectDetection

# Update this path after checking /sys/class/leds on your board
myLedGreenPath = "/sys/class/leds/green:wlan/brightness"

def mySetLedBrightness(path, value):
    try:
        with open(path, "w") as f:
            f.write(str(value))
    except Exception as e:
        print("Error writing LED brightness:", e)

# Blink in a separate thread or before the main App run
# (depending on how App.run() handles blocking)
import threading
def blink_loop():
    mySetLedBrightness(myLedGreenPath, 255)
    time.sleep(0.5)
    mySetLedBrightness(myLedGreenPath, 0)
    time.sleep(0.5)

threading.Thread(target=blink_loop, daemon=True).start()

# Setup UI & detection
ui = WebUI()
detection_stream = VideoObjectDetection(confidence=0.5, debounce_sec=0.0)

ui.on_message("override_th", lambda sid, threshold: detection_stream.override_threshold(threshold))

def send_detections_to_ui(detections: dict):
    for key, value in detections.items():
        entry = {
            "content": key,
            "confidence": value.get("confidence"),
            "timestamp": datetime.now(timezone.utc).isoformat()
        }
        ui.send_message("detection", message=entry)
        
        # ✅ Fixed if statement
        if key == "person":
            blink_loop()

detection_stream.on_detect_all(send_detections_to_ui)

App.run()
