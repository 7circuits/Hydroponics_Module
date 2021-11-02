//Aquaponics Sensors Modul 2

//Programmed for Arduino Pro Mini 3.3V ATMega828
//Einstellung auf 5V 16MHz
//Reset, CMD, Reset, Data working
//27,038 Bytes 88% max 30,720
//1,507 Bytes 73% max 2,048 Bytes

//DHT22 Temperature and Humidity Sensor, DS18B20 Water-Temperature-Sensor, Water Level Sensor
//Micro-SD-Card-Reader, Chargerboard with Akku and Solarpanel
//HC-05 Bluetooth Modul, Reset, CMD, Reset, Data working
//Reading Sensor-Data and Serial-Print, Bluetooth-Serial-Print
//Time Module, Write Time on Card working
//Bluetooth data send only when read 'r' from device

//Time
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

// SD card datalogger, MOSI-pin11, MISO-pin12, CLK-pin13, CS-pin 10
#include <SPI.h>
#include <SD.h>
const int chipSelect = 10;

//DHT22 Temp Humidity Sensor
#include "DHT.h"
#define DHTPIN 6 // Pin 6 !!!!!
#define DHTTYPE DHT22   // DHT22 / DHT11
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);
// default for a 16mhz AVR is a value of 6, Arduino Due 84mhz a value of 30 works. Example to initialize DHT sensor for Arduino Due:
//DHT dht(DHTPIN, DHTTYPE, 30);

//DS18B20 on +3.3VDC Water Level Sensor Standard XH2.54 interface, I<20mA, 10-30 Celsius, 10-90%, Analog Output
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 5 //Pin 5 !!!!!
OneWire oneWire(ONE_WIRE_BUS); // Init oneWire instance
DallasTemperature sensors(&oneWire);// Dallas Temperature Library

//Water Level Sensor
int waterlevelPin = A0;    // Pin A0
int waterlevelValue = 0;

//Serial Port
#include <SoftwareSerial.h>
char myChar ;
SoftwareSerial BTSerial(3, 2); // RX, TX !!!!!

void setup(){
  //Bluetooth
  pinMode(4, OUTPUT);  // CMD Pin
  digitalWrite(4, LOW); // CMD HIGH for AT-Mode, LOW for Data Mode
  pinMode(9, OUTPUT); // RESET Pin
  digitalWrite(9, HIGH); // No Reset
  delay(100);
  digitalWrite(9, LOW); //Reset Set
  delay(100);
  digitalWrite(9, HIGH); //Reset inactive
  delay(1000);
  Serial.begin(9600);
  //Serial.println("Enter AT commands:");
  //BTSerial.begin(38400);  //HC-05 default speed in AT command more
  //delay(100);
  //BTSerial.println("AT+PSWD?");
  //delay(100);
  //BTSerial.println("AT+NAME?");
  //delay(100);
  BTSerial.begin(9600);
  //digitalWrite(4, LOW); // CMD LOW DATA-Mode
  //digitalWrite(9, HIGH);
  //delay(100);
  //digitalWrite(9, LOW);
  //delay(100);
  //digitalWrite(9, HIGH);
  delay(1000);
  sensors.begin();/* Inizialisieren der Dallas Temperature library */
  //SD-Card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    //return;
  }
  Serial.println("Micro-SD-Card initialized.");
  
}

void loop() {
  readSensors();
  delay(3000); 
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void readSensors (){
  tmElements_t tm;
  String dataString = "";
  
  if (RTC.read(tm)) {
  // Readtemperature or humidity takes about 250 milliseconds up to 2 seconds
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
   // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  float hi = dht.computeHeatIndex(f, h);
  float hi2 = (hi-32)*0.55;
  //Water Temperature Sensor
  sensors.requestTemperatures(); // Wasser Temperatur abfragen
  //Water Level Sensor
  waterlevelValue = analogRead(waterlevelPin); 
  
  print2digits(tm.Hour);
  Serial.write(':');
  print2digits(tm.Minute);
  Serial.write(':');
  print2digits(tm.Second);
  Serial.print(", ");
  Serial.print(tm.Day);
  Serial.write('.');
  Serial.print(tm.Month);
  Serial.write('.');
  Serial.println(tmYearToCalendar(tm.Year)); 
  
  if ((tm.Hour) >= 0 && (tm.Hour) < 10){
  dataString += String("0");
  }
  dataString += String(tm.Hour);
  dataString += String(":");
  if ((tm.Minute) >= 0 && (tm.Minute) < 10){
    dataString += String("0");
  }
  dataString += String(tm.Minute);
  dataString += String(":");
  if ((tm.Second) >= 0 && (tm.Second) < 10){
    dataString += String("0");
  }
  dataString += String(tm.Second);
  dataString += String(", ");
  dataString += String(tm.Day);
  dataString += String(".");
  dataString += String(tm.Month);
  dataString += String(".");
  dataString += String(tmYearToCalendar(tm.Year));

  myChar = BTSerial.read();
  if (myChar == 'r'){
  BTSerial.print("T ");
  BTSerial.print(t);
  BTSerial.print("*C, H ");
  BTSerial.print(h);
  BTSerial.print("%, WT ");
  BTSerial.print(sensors.getTempCByIndex(0) );
  BTSerial.print("*C, WL ");
  BTSerial.print(waterlevelValue);
  BTSerial.println("L");
  }
  dataString += String(t);
  dataString += String(",");
  dataString += String(h);
  dataString += String(",");
  dataString += String(sensors.getTempCByIndex(0) );
  dataString += String(",");
  dataString += String(waterlevelValue);
          
  Serial.print("T "); 
  Serial.print(t);
  Serial.print("*C, H "); 
  Serial.print(h);
  Serial.print("%, WT ");
  Serial.print(sensors.getTempCByIndex(0) );
  Serial.print("*C, WL ");
  Serial.print(waterlevelValue);
  Serial.println("L");

  //Write Data to Card
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
                dataFile.println(dataString);
                dataFile.close();
                }
  else {
  Serial.println(" error opening datalog.txt");
  }
  }
 else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }
 delay(1000);  
}
