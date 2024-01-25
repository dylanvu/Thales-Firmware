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
#define CHARACTERISTIC_UUID_TX "a333b197-f1a1-4d70-a452-757067b0bed6"
#define CHARACTERISTIC_UUID_RX "a5953780-748b-4857-96ce-cf31a643aeb7"

// server connection
class MyServerCallbacks: public BLEServerCallbacks{
  void onConnect(BLEServer* pServer){
    deviceConnected = true;
  };

  void onDisconnect(){
    deviceConnected = false;
  }
};

// receiving data from client
class MyCallbacks: public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic *pCharacteristic){
    std::string receivedValue = pCharacteristic -> getValue();

    if(receivedValue.length() > 0){
      Serial.println("Receiving data...");
      
      Serial.print("Received Value: ");
      for(int i = 0; i < receivedValue.length(); i++){
        Serial.print(receivedValue[i]);
      }

      Serial.println();

      if(receivedValue.find("1") != -1){
        Serial.println("Received command 1");
      }

      Serial.println("End of receiving data");
    }
  }
};

// setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);

  // Create BLE Device
  BLEDevice::init("ESP32 Thales");

  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer -> setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer -> createService(SERVICE_UUID);

  // Create BLE Characteristic for Sending Data
  //pCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  pCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

  // Create BLE Descriptor
  pCharacteristic -> addDescriptor(new BLE2902());

  // Create BLE Characteristic for Receiving Data
  BLECharacteristic *pCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  // Callbacks for Receiving Data
  pCharacteristic -> setCallbacks(new MyCallbacks());

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

  if(deviceConnected){
    
    Serial.println("Waiting for command...");
    //std::string message = "it connects yay";
    //pCharacteristic -> setValue(message);
    //pCharacteristic -> notify();
    //delay(1000);

    std::string fake_json_data_1 = "{\"heart_rate\": 80, \"temperature\": 50}";

    pCharacteristic -> setValue(fake_json_data_1);
    pCharacteristic -> notify();
    delay(1000);

    //std::string fake_json_data_2 = "{\"heart_rate\": 100, \"temperature\": 70}";
    int heart = 100;
    int temp = 70;
    std::string fake_json_data_2 = "{\"heart_rate\": " + std::to_string(heart) + ", \"temperature\": " + std::to_string(temp) + "}";

    pCharacteristic -> setValue(fake_json_data_2);
    pCharacteristic -> notify();
    delay(1000);

    /*
    int value = random(0, 50);
    char stringValue[8];
    dtostrf(value, 1, 2, stringValue);
    pCharacteristic->setValue(stringValue);
    pCharacteristic->notify();
    Serial.println("Sent value: " + String(stringValue));
    delay(1000);*/

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
