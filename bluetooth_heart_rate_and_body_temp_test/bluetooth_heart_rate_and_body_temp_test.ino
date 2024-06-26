// ble libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// heart rate sensor libraries
#include <DFRobot_BloodOxygen_S.h>
#include <DFRobot_RTU.h>
#include <Wire.h>
#define I2C_SDA 18
#define I2C_SCL 19

// contact body temperature sensor libraries
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>

/*!
  * @file  gainHeartbeatSPO2.ino
  * @n experiment phenomena: get the heart rate and blood oxygenation, during the update the data obtained does not change
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @licence     The MIT License (MIT)
  * @author      PengKaixing(kaixing.peng@dfrobot.com)
  * @version     V1.0
  * @date        2021-06-21
  * @get         from https://www.dfrobot.com
  * @url         https://github.com/DFRobot/DFRobot_BloodOxygen_S
*/

/**
 * @file basic_test.ino
 * @author Bryan Siepert for Adafruit Industries
 * @brief Shows how to specify a
 * @date 2020-11-10
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "DFRobot_BloodOxygen_S.h"

#define I2C_COMMUNICATION  //use I2C for communication, but use the serial port for communication if the line of codes were masked

#ifdef  I2C_COMMUNICATION
#define I2C_ADDRESS    0x57
  DFRobot_BloodOxygen_S_I2C MAX30102(&Wire ,I2C_ADDRESS);
#else
/* ---------------------------------------------------------------------------------------------------------------
 *    board   |             MCU                | Leonardo/Mega2560/M0 |    UNO    | ESP8266 | ESP32 |  microbit  |
 *     VCC    |            3.3V/5V             |        VCC           |    VCC    |   VCC   |  VCC  |     X      |
 *     GND    |              GND               |        GND           |    GND    |   GND   |  GND  |     X      |
 *     RX     |              TX                |     Serial1 TX1      |     5     |   5/D6  |  D2   |     X      |
 *     TX     |              RX                |     Serial1 RX1      |     4     |   4/D7  |  D3   |     X      |
 * ---------------------------------------------------------------------------------------------------------------*/
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
SoftwareSerial mySerial(4, 5);
DFRobot_BloodOxygen_S_SoftWareUart MAX30102(&mySerial, 115200);
#else
DFRobot_BloodOxygen_S_HardWareUart MAX30102(&Serial1, 115200); 
#endif
#endif

// bluetooth server variables
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// contact body temperature variables
Adafruit_TMP117  temperature_sensor;

// touchpoint variables
const int touchpoint = 42;

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

      // to turn on and off touchpoint
      if(receivedValue.find("1") != -1)
      {
        //Serial.println("Received command 1");
        delay(100);
        digitalWrite(touchpoint, LOW);
        delay(100);
        digitalWrite(touchpoint, HIGH);
        delay(100);
        digitalWrite(touchpoint, LOW);
        delay(100);
        digitalWrite(touchpoint, HIGH);
        Serial.println("Received command 1");
      }
      else if(receivedValue.find("0") != -1)
      {
        digitalWrite(touchpoint, HIGH);
        Serial.println("Received command 0");
      }

      Serial.println("End of receiving data");
    }
  }
};

void setup()
{
  Serial.begin(115200);
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

  // Touchpoint Initializing
  pinMode(touchpoint, OUTPUT);
  digitalWrite(touchpoint, LOW);
  delay(100);
  digitalWrite(touchpoint, HIGH);
  delay(100);
  digitalWrite(touchpoint, LOW);
  delay(100);
  digitalWrite(touchpoint, HIGH);
  delay(100);

  // Heart Rate Sensor Initializing
  Serial.println("READY");
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();
  while (false == MAX30102.begin())
  {
    Serial.println("Heart Rate Sensor Init Fail!");
    delay(1000);
  }
  Serial.println("Heart Rate Sensor Init Success!");
  Serial.println("Start measuring...");
  MAX30102.sensorStartCollect();

   // Contact Body Temperature Sensor Initializing
  while (!Serial)
  {
    delay(100);
  }

  if (!temperature_sensor.begin()) 
  {
    Serial.println("Contact Body Temperature Sensor Init Fail!");
    delay(1000);
  }
  Serial.println("Contact Body Temperature Sensor Init Success!");

}

void loop()
{
  if(deviceConnected){
    
    Serial.println("Waiting for command...");
    /*std::string message = "it connects yay";
    pCharacteristic -> setValue(message);
    pCharacteristic -> notify();
    delay(1000);*/

    // Heart Rate Sensor
    MAX30102.getHeartbeatSPO2();

    // Contact Body Temperature Sensor
    sensors_event_t temp; // create an empty event to be filled
    temperature_sensor.getEvent(&temp); //fill the empty event object with the current measurements

    std::string json_data = "{\"heart_rate\": " + std::to_string(MAX30102._sHeartbeatSPO2.Heartbeat) + ".00" + ", \"temperature\": " + std::to_string(temp.temperature) + "}";

    
    pCharacteristic -> setValue(json_data);
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

  /*
  int count = 0;
  while(Serial.available() == 0){
    // gather heart rate data
    MAX30102.getHeartbeatSPO2();
    
    // increment counter (in seconds)
    count++;

    //Serial.print("Heart rate is : ");
    Serial.print(MAX30102._sHeartbeatSPO2.Heartbeat);
    Serial.print("*");
    Serial.println(count);
    //Serial.println(" Times/min");

    delay(1000);

    // Contact Body Temperature Sensor
    sensors_event_t temp; // create an empty event to be filled
    temperature_sensor.getEvent(&temp); //fill the empty event object with the current measurements
    Serial.print("Temperature  "); 
    Serial.print(temp.temperature);
    Serial.println(" degrees C");
    Serial.println("");

    delay(1000);
  }*/


/*  String command = Serial.readString();
  command.trim();

  if(command == "hello"){
    Serial.println(command);
    Serial.write("Device is on!\r\n");
  }
  else{
    Serial.println(command);
    Serial.println("it doesn't work");
  }*/
  
}
