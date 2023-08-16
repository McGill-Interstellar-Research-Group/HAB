#include <RTClib.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TCAADDR 0x70
#define seaLevelPressure_hPa 1013.
#define SD_SS 10
#define SD_ChipSelect 10
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 9

RTC_DS3231 rtc;
Adafruit_BMP085 bmp;
File myFile;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress TC0,TC1,TC2;


// the pin that is connected to SQW
#define CLOCK_INTERRUPT_PIN 2
#define timeDelay 5

bool get_data = false; 
String FileName = "test.txt";

void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void set_alarm(){
  tcaselect(4);
  if(!rtc.setAlarm1(
            rtc.now() + TimeSpan(timeDelay),
            DS3231_A1_Second // this mode triggers the alarm when the seconds match. See Doxygen for other options
    )) {
        Serial.println("Error, alarm wasn't set!");
    }else {
        Serial.println("Alarm will happen in 10 seconds!");
    }
}
void setup() {
    Serial.begin(9600);

    sensors.begin();
    if (!sensors.getAddress(TC0, 0)) {
    Serial.println("Unable to find address for Device 0"); 
    oops_blink();
    }
    if (!sensors.getAddress(TC1, 1)){
      Serial.println("Unable to find address for Device 1"); 
      oops_blink();
    }
    if (!sensors.getAddress(TC2, 2)){
      Serial.println("Unable to find address for Device 2"); 
      oops_blink();
    }

    Wire.begin();
    tcaselect(2);
    if(!bmp.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, bmp not detected ... Check your wiring!");
    oops_blink();
    while(1);
  } else{
    Serial.println("BMP Initialized");
  }
    tcaselect(4);
    // initializing the rtc
    if(!rtc.begin()) {
        Serial.println("Couldn't find RTC!");
        oops_blink();
        while (1);
    }
    // set time to zero
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


    //we don't need the 32K Pin, so disable it
    rtc.disable32K();

    // Making it so, that the alarm will trigger an interrupt
    pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), trigger, FALLING);

    // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
    // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
    rtc.clearAlarm(1);
    rtc.clearAlarm(2);

    // stop oscillating signals at SQW Pin
    // otherwise setAlarm1 will fail
    rtc.writeSqwPinMode(DS3231_OFF);

    // turn off alarm 2 (in case it isn't off already)
    // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
    rtc.disableAlarm(2);

    // schedule an alarm 10 seconds in the future
    set_alarm(); //ONE NANO DELAY 5, OTHER NANO DELAY 0

    SD_Initialize();

    pinMode(13, OUTPUT);
}

void loop() {
    // print current time
    tcaselect(4);
    char date[10] = "hh:mm:ss";
    rtc.now().toString(date);
    Serial.print(date);

    // the value at SQW-Pin (because of pullup 1 means no alarm)
    Serial.print("] SQW: ");
    Serial.print(digitalRead(CLOCK_INTERRUPT_PIN));

    // whether a alarm fired
    Serial.print(" Fired: ");
    Serial.println(rtc.alarmFired(1));
    delay(1000);

    if (get_data){
      get_data = false;
      rtc.clearAlarm(1);
      set_alarm();

      char date[10] = "hh:mm:ss";
      rtc.now().toString(date);
      Serial.println(date);
      
      String data_other = get_baro();
      sensors.requestTemperatures();
      String tc_data = get_Temperature(TC0) + "\t" + get_Temperature(TC1)+ "\t" + get_Temperature(TC2);
      Serial.println(data_other);
      Serial.println(tc_data);

      SD_Write(date);
      SD_Write(data_other);
      SD_Write(tc_data);
      delay(1000);
      digitalWrite(13, LOW);
      
    }
}

void trigger() {
    get_data = true;
}

String get_baro()
{
  tcaselect(2);
  String Pa = String( (float) bmp.readPressure());
  tcaselect(4);
  String temp = String(rtc.getTemperature());
  String data = Pa + "Pa\t" + temp;
  return data;
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
    digitalWrite(13, HIGH);
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening txt"));
    digitalWrite(13, LOW);
  }
}

void SD_Initialize(){  
  pinMode(SD_SS,OUTPUT);

  if (!SD.begin(SD_ChipSelect)) {
    Serial.println(F("SD INIT FAILED"));
    oops_blink();
    while (1);
  }

}


String get_Temperature(DeviceAddress deviceAddress){
  
  String T = String((float)sensors.getTempC(deviceAddress));
  return T;
}

void oops_blink(){
  digitalWrite(13, HIGH);
  delay(2000);
  digitalWrite(13, LOW);
}
