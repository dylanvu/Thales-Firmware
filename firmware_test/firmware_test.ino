// BLE libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// contactless body temp sensor library
#include <Wire.h>

// contact body temp sensor libraries
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>

// heart rate sensor libraries
#include <DFRobot_BloodOxygen_S.h>
#include <DFRobot_RTU.h>

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

// contact body sensor variable
Adafruit_TMP117  tmp117;

// contactless body sensor varibles
#define D6T_ADDR 0x0A 
#define D6T_CMD 0x4C  
#define N_ROW 4
#define N_PIXEL (4 * 4)
#define N_READ ((N_PIXEL + 1) * 2 + 1)

uint8_t rbuf[N_READ];
double ptat;
double pix_data[N_PIXEL];

// temperature calculations
uint8_t calc_crc(uint8_t data) {
    int index;
    uint8_t temp;
    for (index = 0; index < 8; index++) {
        temp = data;
        data <<= 1;
        if (temp & 0x80) {data ^= 0x07;}
    }
    return data;
}

// temperature calculating data sequence
bool D6T_checkPEC(uint8_t buf[], int n) {
    int i;
    uint8_t crc = calc_crc((D6T_ADDR << 1) | 1);  // I2C Read address (8bit)
    for (i = 0; i < n; i++) {
        crc = calc_crc(buf[i] ^ crc);
    }
    bool ret = crc != buf[n];
    if (ret) {
        Serial.print("PEC check failed:");
        Serial.print(crc, HEX);
        Serial.print("(cal) vs ");
        Serial.print(buf[n], HEX);
        Serial.println("(get)");
    }
    return ret;
}

// converting data from byte stream
int16_t conv8us_s16_le(uint8_t* buf, int n) {
    uint16_t ret;
    ret = (uint16_t)buf[n];
    ret += ((uint16_t)buf[n + 1]) << 8;
    return (int16_t)ret;   // and convert negative.
}

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

  // i2c set up
  Wire.begin();
  delay(100);

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


  // contact body sensor setup
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("Contact Body Sensor Test!");

  // initializing
  if (!tmp117.begin()) {
    Serial.println("Failed to find TMP117 chip");
    while (1) { delay(10); }
  }
  Serial.println("TMP117 Found!");


  // heart rate sensor
  Serial.begin(115200);
  while (false == MAX30102.begin())
  {
    Serial.println("init fail!");
    delay(1000);
  }
  Serial.println("init success!");
  Serial.println("start measuring...");
  MAX30102.sensorStartCollect();


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
    delay(20);
  }

  // Disconnect
  if(!deviceConnected && oldDeviceConnected){
    delay(500);
    pServer -> startAdvertising();
    Serial.println("Advertising");
    oldDeviceConnected = deviceConnected;
  }

  // Connect to contact body sensor
  if(deviceConnected && !oldDeviceConnected){
    oldDeviceConnected = deviceConnected;
  }

  sensors_event_t temp;
  tmp117.getEvent(&temp);
  std::string temp_message = "Temperature: " + temp.temperature + " degrees C";
  Serial.println(temp_message);

  delay(1000); // 1 sec

  // heart rate sensor (still need to define i2c communication stuff)
  MAX30102.getHeartbeatSPO2();
  /*
  Serial.print("SPO2 is : ");
  Serial.print(MAX30102._sHeartbeatSPO2.SPO2);
  Serial.println("%");*/ // not sure what this is for
  str::string heart_rate_message = "Heart Rate: " + MAX30102._sHeartbeatSPO2.Heartbeat + " times/min";
  Serial.println(heart_rate_message);

  delay(1000); // 1 sec

  // contactless body temperature sensor
  int i = 0;
	int16_t itemp = 0;

  // reading data
  memset(rbuf, 0, N_READ);
  Wire.beginTransmission(D6T_ADDR); 
  Wire.write(D6T_CMD);               
  Wire.endTransmission();
	delay(1);
  Wire.requestFrom(D6T_ADDR, N_READ);
  while (Wire.available()) {
    rbuf[i++] = Wire.read();
  }
  D6T_checkPEC(rbuf, N_READ - 1);

  // converting data
  //ptat = (double)conv8us_s16_le(rbuf, 0) / 10.0; maybe don't need this
	for (i = 0; i < N_PIXEL; i++) {
		itemp = conv8us_s16_le(rbuf, 2 + 2*i);
		pix_data[i] = (double)itemp / 10.0;
  }

  for (i = 0; i < N_PIXEL; i++) {
	  Serial.print(pix_data[i], 1);
		Serial.print(", ");
	}	
  Serial.println(" degrees C");
	
  delay(1000);

}
