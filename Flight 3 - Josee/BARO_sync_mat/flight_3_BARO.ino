/*
 * MCGILL UNIVERSITY - INTERSTELLAR LABS
 * PROGRAM FOR STAGE 'BARO'
 * DAQ FOR BAROMETER, ACCELERO, GYRO, 2 TC
 */

//DELAYS NOT RIGHT

#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP085.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

//SD Definitions
#define SD_SS         10
#define SD_ChipSelect 10
//TC Definitions
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9
//BMP Definitions
#define seaLevelPressure_hPa 1013.

#define BUZZER    4
#define MOSFET    7 

//VARIABLES
int timeDelay = 2000; //ms
unsigned long timer = millis();
String FileName = "data.txt";
int terminationTime = 2*60*60*1000; //2 hours********************************************

//OBJECTS
File myFile;
Adafruit_BMP085 bmp;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress TC0,TC1;


void setup() {
  Serial.begin(9600);

  pinMode(SD_SS,OUTPUT);
  pinMode(BUZZER, OUTPUT);

 
  SD_Initialize();  //INITIALIZE SD CARD
  BMP_Initialize(); //INITIALIZE BMP
  TC_Initialize(); //INITIALIZE TC'S

  String full_STR = "Time \t Pressure \t TC4 \t TC5";
  SD_Write(full_STR);
  
}

void loop() {
  //Update time
  timer = millis();
  Beep();
  //Pull temperature from TC's
  sensors.requestTemperatures();

  //Acquire Data
  String Pa = String( (float) bmp.readPressure());
  String T0 = String( (float) get_Temperature(TC0));
  String T1 = String( (float) get_Temperature(TC1));
  

 
  String FullSTR = String(timer/1000) 
    + "\t" + Pa + "\t" + T0 + "\t"  + T1;

  Serial.println(FullSTR);
  SD_Write(FullSTR);
  delay(timeDelay);

  if(timer > terminationTime){
    ActivateMosfet(timeDelay);  
  }
}


// ------------ [ METHODS ] ------------

void TC_Initialize(){
  sensors.begin();
    if (!sensors.getAddress(TC0, 0)) Serial.println(F("TC: Unable to find address for Device 0")); 
    if (!sensors.getAddress(TC1, 1)) Serial.println(F("TC: Unable to find address for Device 1")); 
}

void BMP_Initialize(){
  if (!bmp.begin()) {
    Serial.println(F("BMP INIT FAILED"));
    while (1) {}
   }
}

void SD_Initialize(){  
  pinMode(SD_SS,OUTPUT);

  if (!SD.begin(SD_ChipSelect)) {
    Serial.println(F("SD INIT FAILED"));
    while (1);
  }

  SD_Write("McGill University - INTERSTELLAR HAB");
  SD_Write("IN-STELLAR MARK 7 - STAGE BARO");
}


void SD_Write(String arg){

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(FileName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print(F("Writing to test.txt..."));
    myFile.println(arg);
    // close the file:
    myFile.close();
    Serial.println(F("done."));
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening test.txt"));
  }
}

float get_Temperature(DeviceAddress deviceAddress){
  return sensors.getTempC(deviceAddress);
}

void ActivateMosfet(int timeLength){
  digitalWrite(MOSFET, HIGH);
  delay(timeLength);
  digitalWrite(MOSFET, LOW);
}

void Beep(){
  digitalWrite(BUZZER, HIGH);
  delay(250);
  digitalWrite(BUZZER, LOW);
}
