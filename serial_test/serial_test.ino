// led variables
//const int led = 33;
const int led = 2;


// setup
void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(led, OUTPUT);
}

void loop() {
  Serial.println("Waiting for command...");
  int count = 0;
  while(Serial.available() == 0){
    count++;
    Serial.println((String)"wait" + count);
    delay(1000);
  }

  String command = Serial.readString();
  command.trim();

  if(command == "hello"){
    Serial.println(command);
    Serial.println("it works");
    Serial.write("hey im testing write\r\n");
    /*digitalWrite(led, HIGH);
    delay(1000);
    Serial.println("high");
    digitalWrite(led, LOW);
    delay(1000);
    Serial.println("low");*/
    
    

  }
  else{
    Serial.println(command);
    Serial.println("it doesn't work");
  }

}
