#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

//SD Definitions
#define SD_SS         10
#define SD_ChipSelect 10

//TC Definitions
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

//CMB DEFINITIONS
#define BUZZER    4

//VARIABLES
int timeDelay = 2000+15; //ms
unsigned long timer = millis();
String FileName = "data.txt";


//OBJECTS
File myFile;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress TC0,TC1,TC2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(SD_SS,OUTPUT);
  pinMode(BUZZER, OUTPUT);

 
  SD_Initialize();  //INITIALIZE SD CARD
  gyro_Initialize(); //INITIALIZE GYRO
  TC_Initialize(); //INITIALIZE TC'S

  String full_STR = "Time \t TCi \t TC1 \t TC2 \t TC3 \t Pitch \t Yaw \t Roll \t xAxis \t yAxis \t zAxis";
  SD_Write(full_STR);

}

void loop() {
  //Update time
  timer = millis();
  Beep();
  //Pull temperature from TC's
  sensors.requestTemperatures();
  
  //get temp
  String Ti = String ( (float) mpu.readTemperature());
  String T0 = String( (float) get_Temperature(TC0));
  String T1 = String( (float) get_Temperature(TC1));
  String T2 = String( (float) get_Temperature(TC2));

  //get gyro 
  Vector normAccel = mpu.readNormalizeAccel();

  int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  int roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;
  int yaw = (atan2(normAccel.ZAxis, sqrt(normAccel.XAxis*normAccel.XAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  
  
  //write data
  String full_STR = String(timer/1000) + "\t" 
    + Ti + "\t" + T0 + "\t" + T1 + "\t" + T2 + "\t" 
    + String(pitch) + "\t" + String(roll) + "\t" + String(yaw)+ "\t" 
    + String(normAccel.XAxis) + "\t" + String(normAccel.YAxis) + "\t" + String(normAccel.ZAxis);
  SD_Write(full_STR);
  Serial.println(full_STR);
  delay(timeDelay);
}

void TC_Initialize(){
  sensors.begin();
    if (!sensors.getAddress(TC0, 0)) Serial.println(F("TC: Unable to find address for Device 0")); 
    if (!sensors.getAddress(TC1, 1)) Serial.println(F("TC: Unable to find address for Device 1")); 
    if (!sensors.getAddress(TC2, 2)) Serial.println(F("TC: Unable to find address for Device 2")); 

}

void gyro_Initialize(){
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
  }
}

void SD_Initialize(){  
  pinMode(SD_SS,OUTPUT);

  if (!SD.begin(SD_ChipSelect)) {
    Serial.println(F("SD INIT FAILED"));
    while (1);
  }

  SD_Write("McGill University - INTERSTELLAR HAB");
  SD_Write("IN-STELLAR MARK 8 - STAGE THERMO");
  SD_Write(__DATE__);
}

void SD_Write(String arg){

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(FileName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print(F("Writing to txt..."));
    myFile.println(arg);
    // close the file:
    myFile.close();
    Serial.println(F("done."));
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening txt"));
  }
}

float get_Temperature(DeviceAddress deviceAddress){
  return sensors.getTempC(deviceAddress);
}

void Beep(){
  digitalWrite(BUZZER, HIGH);
  delay(250);
  digitalWrite(BUZZER, LOW);
}
