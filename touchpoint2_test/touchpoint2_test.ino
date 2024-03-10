// touchpoint variables
const int touchpoint1 = 45;
const int touchpoint2 = 46;

void sendPulseTouchpoint1() {
  Serial.println("Pulsing Touchpoint 1");

  // first touchpoint
  digitalWrite(touchpoint1, HIGH);
  delay(25);
  digitalWrite(touchpoint1, LOW);
  delay(25);
}

void sendPulseTouchpoint2() {
  Serial.println("Pulsing Touchpoint 2");

  // second touchpoint
  digitalWrite(touchpoint2, HIGH);
  delay(25);
  digitalWrite(touchpoint2, LOW);
  delay(25);
}

void turnOnTouchpoint() {
  sendPulseTouchpoint1();
  delay(1000); // 1 second delay
  sendPulseTouchpoint2();
}

void turnOffTouchpoint() {
  sendPulseTouchpoint1();
  sendPulseTouchpoint1();
  sendPulseTouchpoint1();

  delay(1000); // 1 second delay

  sendPulseTouchpoint2();
  sendPulseTouchpoint2();
  sendPulseTouchpoint2();
}

void setup() {
  Serial.begin(115200);
  // Touchpoint Initializing
  pinMode(touchpoint1, OUTPUT);
  pinMode(touchpoint2, OUTPUT);

}

void loop() {
  if (Serial.available() > 0) {
    char receivedValue = Serial.read();

    if (receivedValue == '0') 
    {
      turnOnTouchpoint();
    } 
    else if (receivedValue == '1') 
    {
      turnOffTouchpoint();
    } 
    else 
    {
      Serial.println("Invalid command received.");
    }
  }

  delay(500); 
}
