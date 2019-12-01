#include "./resources/sensor.h"
#include "./resources/eeprom.h"
#include <Wire.h>

sensor myData; 
eeprom myEEPROM; //create instance of eeprom

void setup() {
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

  myData.i2c_scan();
}

int temp_address = 0; 

void loop() {
  Serial.print("DO: ");
  Serial.print(myData.dissolved_oxygen());
  Serial.println(" mg/L");

  myData.read(); //first we read from the bar02 sensor
  Serial.print("Pressure: ");
  Serial.print(myData.pressure());
  Serial.println(" mbar");

  Serial.print("Temperature: ");
  Serial.print(myData.temperature());
  Serial.println(" C");
  delay(1000);


  //Write the Data to the EEPROM
  myEEPROM.write_temp(temp_address, myData.temperature());
  Serial.print(myData.temperature());
  Serial.print(" is stored at address ");
  Serial.println(temp_address);
  temp_address = temp_address + 2;  // increment the addresss 

  //Testing reading EEPROM -> temperature 
  if (temp_address == 20) {
    myEEPROM.read_temp(temp_address);
    temp_address = 0;
  }


  Serial.println();  
}