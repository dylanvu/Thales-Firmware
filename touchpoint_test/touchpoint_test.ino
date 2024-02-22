const int pin = 45;

void sendPulse() {
  Serial.println("Pulsing");
  digitalWrite(pin, HIGH);
  delay(25);
  digitalWrite(pin, LOW);
  delay(25);
}

void turnOnTouchpoint() {
  sendPulse();
}

void turnOffTouchpoint() {
  sendPulse();
  sendPulse();
  sendPulse();
}

void setup()
{
  pinMode(pin, OUTPUT);
  // test out the button presses
  // sendPulse();
  // delay(2000);
  // sendPulse();
  // sendPulse();
}

void loop()
{
  turnOnTouchpoint();
  delay(2000);
  turnOffTouchpoint();
  delay(2000);
}