#include <DFRobot_BloodOxygen_S.h>
#include <DFRobot_RTU.h>
#include <Wire.h>
#define I2C_SDA 18
#define I2C_SCL 19

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

void setup()
{
  Serial.begin(115200);
  Serial.println("READY");
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();
  while (false == MAX30102.begin())
  {
    Serial.println("init fail!");
    delay(1000);
  }
  Serial.println("init success!");
  Serial.println("Start measuring...");
  MAX30102.sensorStartCollect();
}

void loop()
{
  
  Serial.println("Waiting for command...");
  while(Serial.available() == 0){
    // gather heart rate data
    MAX30102.getHeartbeatSPO2();
    
    Serial.print("Heart rate is : ");
    Serial.print(MAX30102._sHeartbeatSPO2.Heartbeat);
    Serial.println(" Times/min");

    delay(1000);
    //Serial.println("wait");
    //delay(1000);
  }

  String command = Serial.readString();
  command.trim();

  if(command == "hello"){
    Serial.println(command);
    Serial.write("Device is on!\r\n");
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


  //MAX30102.getHeartbeatSPO2();
  /*Serial.print("SPO2 is : ");
  Serial.print(MAX30102._sHeartbeatSPO2.SPO2);
  Serial.println("%");*/
  //Serial.print("Heart rate is : ");
  //Serial.print(MAX30102._sHeartbeatSPO2.Heartbeat);
  //Serial.println(" Times/min");
  // Serial.print("Temperature value of the board is : ");
  //Serial.print(MAX30102.getTemperature_C());
  // Serial.println(" ℃");
  //The sensor updates the data every 4 seconds
  //delay(4000);
  //Serial.println("stop measuring...");
  //MAX30102.sensorEndCollect();
  
}
