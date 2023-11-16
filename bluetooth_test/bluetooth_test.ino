#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// button + led variables
const int button = 12;
const int led = 33;
int currentState;
int lastState = LOW;

// bluetooth server variables
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// UUID 
#define SERVICE_UUID        "af97994f-4d78-457e-8e10-05dd0ce6f680"
#define CHARACTERISTIC_UUID "a333b197-f1a1-4d70-a452-757067b0bed6"

// server connection
class MyServerCallbacks: public BLEServerCallbacks{
  void onConnect(BLEServer* pServer){
    deviceConnected = true;
  };

  void onDisconnect(){
    deviceConnected = false;
  }
};

// setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);

  // Create BLE Device
  BLEDevice::init("ESP32 Thales");

  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer -> setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer -> createService(SERVICE_UUID);

  // Create BLE Characteristic
  pCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

  // Create BLE Descriptor
  pCharacteristic -> addDescriptor(new BLE2902());

  // Starting the BLE Service
  pService -> start();

  // Starting Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising -> addServiceUUID(SERVICE_UUID);
  pAdvertising -> setScanResponse(false);
  pAdvertising -> setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("Waiting...");

}

void loop() {
  // put your main code here, to run repeatedly:
  /*digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  delay(500);*/

  //currentState = digitalRead(button);
/*
  if (lastState == HIGH && currentState == LOW)
  {
    digitalWrite(led, HIGH);
    Serial.println("The button is pressed");
  }
  else if (lastState == LOW && currentState == HIGH)
  {
    Serial.println("The button is released");
    digitalWrite(led, HIGH);
  }*/
  /*
  if(currentState == LOW){
    digitalWrite(led, LOW);
    Serial.println("low");
  }
  else{
    digitalWrite(led, HIGH);
    Serial.println("high");
  }*/

  //lastState = currentState;

  if(deviceConnected){
    Serial.println("Device Connected");
    std::string message = "it connects yay";
    pCharacteristic -> setValue(message);
    pCharacteristic -> notify();
    delay(3);
  }

  // Disconnect
  if(!deviceConnected && oldDeviceConnected){
    delay(500);
    pServer -> startAdvertising();
    Serial.println("Advertising");
    oldDeviceConnected = deviceConnected;
  }

  // Connect
  if(deviceConnected && !oldDeviceConnected){
    oldDeviceConnected = deviceConnected;
  }

}
