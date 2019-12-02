#include "./resources/sensor.h"
#include "./resources/eeprom.h"
#include <Wire.h>


//LoRa  ------------------------------
#include <LoRa.h>
#include <SPI.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2
// ------------------------------------

sensor myData; 
eeprom myEEPROM; //create instance of eeprom

void onLoRaRx(int packetSize){
  Serial.println("LoRa packet received.");
  while (LoRa.available()) {
    Serial.printf("LoRa Packet: %s\n", LoRa.readString().c_str());
    //String LoRaData = LoRa.readString();
    //Serial.println(LoRaData); 
  }
}

void setup() {
  //attachInterrupt(dio0,onLoRaRx,CHANGE);
  Wire.begin();
  Serial.begin(9600);
  delay(10);

  if(myEEPROM.init()){
    Serial.println("EEPROM Initialized!");
  }

  //Initialize sensors - resets and calibrates sensors
  Serial.print("Initializing Sensors");
  if (myData.init()){
    Serial.print(".");
    delay(1000); // small delay between function calls
  }
  Serial.println("");
  Serial.println("Sensor Init Complete!");


  //Lets see what I2C devices are out there... after we have already initialized our sensors
  myData.i2c_scan();

  //LoRa Init ------------
  //setup LoRa transceiver module
  //915E6 for North America
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
  LoRa.receive();
  LoRa.onReceive(onLoRaRx);
  Serial.println("LoRa Initializing OK!");
  Serial.println("Starting Buoy Program");

}

int temp_address = 0; 

void loop() {
  Serial.printf("DO: %.2f mg/L\n", myData.dissolved_oxygen());
  delay(7*1000); // 7 second delay between DO checks
  
  // myData.read(); //first we read from the bar02 sensor
  // Serial.print("Pressure: ");
  // Serial.print(myData.pressure());
  // Serial.println(" mbar");

  // Serial.print("Temperature: ");
  // Serial.print(myData.temperature());
  // Serial.println(" C");
  // delay(1000);


  // //Write the Data to the EEPROM
  // myEEPROM.write_temp(temp_address, myData.temperature());
  // Serial.print(myData.temperature());
  // Serial.print(" is stored at address ");
  // Serial.println(temp_address);
  // temp_address = temp_address + 2;  // increment the addresss 

  // //Testing reading EEPROM -> temperature 
  // if (temp_address == 20) {
  //   myEEPROM.read_temp(temp_address);
  //   temp_address = 0;
  // }


  //Serial.println();  
  //delay(1000);
}