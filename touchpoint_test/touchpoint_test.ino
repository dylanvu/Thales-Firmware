const int pin = 45;

void setup() {
   pinMode(pin, OUTPUT);
}

void loop() {
  digitalWrite(pin, LOW);
  delay(500);
  digitalWrite(pin, HIGH);
  delay(500);
  digitalWrite(pin, LOW);
  delay(500);
  digitalWrite(pin, HIGH);
}
