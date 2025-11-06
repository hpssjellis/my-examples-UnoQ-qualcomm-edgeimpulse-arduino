#include <Arduino_RouterBridge.h>
#define Serial Monitor

unsigned long myOldTime;
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
  Serial.println("Serial as the Monitor using the python bridge");
}

void loop() {  

 
  while (Serial.available()) {
    myInput = Serial.readStringUntil('\n');
    Serial.print(myInput);
  }
  
  int myButtonState = digitalRead(myButton);


  if (myButtonState == HIGH){     // A5 to 3.3V 
      Serial.print("LED4_B is LOW/On, A5 Pin is HIGH"); 
      digitalWrite(myOtherLED, LOW); 
  } else {
     digitalWrite(myOtherLED, HIGH); 
  }
  
  digitalWrite(myLED, LOW); 
  Serial.print("LED3_G is LOW/On, Send Characters or try A5 to 3.3V"); 
  delay(1000);
  digitalWrite(myLED, HIGH); 
  delay(3000);


  
}



