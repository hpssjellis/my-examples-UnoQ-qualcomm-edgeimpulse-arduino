#include <Arduino_RouterBridge.h>
#define Serial Monitor

unsigned long myOldTime;
int myDelay    = 1000;    // wait 100 ms or 1 second
int myButton   = A5;      // pin A5
int myLED      = LED3_G; 
int myOtherLED = LED4_B;
String myInput;

void setup() {
  // Monitor.begin() ignores the baud rate
  Serial.begin(9600);  // Serial.begin(115200); 
  pinMode(myLED, OUTPUT);
  pinMode(myOtherLED, OUTPUT);
  pinMode(myButton, INPUT_PULLDOWN);
  
  delay(3000);
  myOldTime = millis();
  Serial.println("Serial as the Monitor using the python bridge");
}

void loop() {  

 
  while (Serial.available()) {
    myInput = Serial.readStringUntil('\n');
    Serial.print(myInput);
  }
  
  int myButtonState = digitalRead(myButton);

  
  if (millis() - myOldTime >= myDelay) {
    digitalWrite(myLED, !digitalRead(myLED));
    myOldTime = millis();
    
    if (digitalRead(myLED) == LOW){
      Serial.print("LED3_G is LOW/On, Send string or A5 to 3.3V");
    }

    if (myButtonState == HIGH){     // A5 to 3.3V 
      Serial.print("LED4_B is LOW/On, A5 Pin is HIGH"); 
    } 
  }
  

  if (myButtonState == HIGH){          // A5 to 3.3V 
     digitalWrite(myOtherLED, LOW);    // on, yes it is confusing
  }  else {
     digitalWrite(myOtherLED, HIGH); 
  }

  
}



