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
int next_addr;   // next location to write to on EEPROM - must be global for 1 time init

void onLoRaRx(int packetSize){
  Serial.println("LoRa packet received.");
  while (LoRa.available()) {
    Serial.printf("LoRa Packet: %s\n", LoRa.readString().c_str());
    //String LoRaData = LoRa.readString();
    //Serial.println(LoRaData); 
  }
}

void setup() {

  Wire.begin();
  Serial.begin(9600);
  delay(10);
  Serial.println();

  if(myEEPROM.init()){
    Serial.println("EEPROM Initialized!");
    //next_addr = myEEPROM.read_next_address();
    next_addr = 32;

    myEEPROM.read_config_raw();

        // TESTING THE EEPROM
        // byte temp_arr[] = {'4','8','2','6','0','a','b','\0'}; // 32 bytes of char format
        // //myEEPROM.page_write(0,temp_arr,32);
        // delay(500);
        // byte temp_arr2[32];
        // myEEPROM.page_read(0,temp_arr2,32);
        // for(int j = 0; j<7; j++){
        //   temp_arr2[j] += 1;
        // }
        //printf("Testing EEPROM page operations...\n%s\n",temp_arr2);

        // TEST INDIVIDUAL GETTERS and SETTERS
        // id
        //myEEPROM.write_id(73);
        Serial.printf("EEPROM ID test (73 expected): %d\n",(int)myEEPROM.read_id());
        // mode
        //myEEPROM.write_mode(254);
        Serial.printf("EEPROM mode test (254 expected): %d\n",(int)myEEPROM.read_mode());
        // addr
        myEEPROM.write_next_address(32);
        Serial.printf("EEPROM next_addr test (32 expected): %d\n",(int)myEEPROM.read_next_address());
        // interval
        //myEEPROM.write_interval(498);
        Serial.printf("EEPROM interval test (498 expected): %d\n",(int)myEEPROM.read_interval());
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

  //printf("Sizeof int: %d\nSizeof long: %d\nSizeof unsigned long int: %d\n",sizeof(int),sizeof(long),sizeof(unsigned long int));
}

//int temp_address = 0; 

void loop() {
  float temp_ambient     = 0.0;
  float pressure_ambient = 0.0;
  float wind_speed       = 0.0;
  float lux              = 0.0;
  float temp_water       = 0.0;
  float pressure_water   = 0.0;
  float dissolved_oxygen = 0.0;
  int timestamp          = 0;
  byte current_data_package[33]; // one extra character, which will NULL terminate when this array is used as a string

  // sleep when not doing things
  delay(7*1000); // 7 second delay between sensor checks
  // read ALL the SENSORS
  temp_ambient     = 23.45;
  pressure_ambient = 1011.66;
  wind_speed       = 4.5;
  lux              = 0.0;
  temp_water       = 19.3456433;
  pressure_water   = 990.0;
  dissolved_oxygen = myData.dissolved_oxygen();
  timestamp        = millis();

    // DEBUG
    myEEPROM.read_config_raw();
    Serial.printf("%f %f %f %f %f %f %f %d\n", 
                  temp_ambient, pressure_ambient, wind_speed, lux, temp_water, pressure_water, dissolved_oxygen, timestamp);

  // construct packet
  memcpy(current_data_package+0,&temp_ambient,sizeof(float));
  memcpy(current_data_package+4,&pressure_ambient,sizeof(float));
  memcpy(current_data_package+8,&wind_speed,sizeof(float));
  memcpy(current_data_package+12,&lux,sizeof(float));
  memcpy(current_data_package+16,&temp_water,sizeof(float));
  memcpy(current_data_package+20,&pressure_water,sizeof(float));
  memcpy(current_data_package+24,&dissolved_oxygen,sizeof(float));
  memcpy(current_data_package+28,&timestamp,sizeof(int));
  // send packet

      // packet to serial
      Serial.println((char*)&current_data_package);
      // packet to EEPROM

      // packet to LoRa
      LoRa.beginPacket();
      LoRa.print((char*)&current_data_package);
      LoRa.endPacket();

  next_addr += 32;
  while(next_addr%32!=0){
    Serial.println("Bad address for frame! Skipping to next 32 byte chunk.");
    next_addr = next_addr + (32-(next_addr % 32));
  }
  if(next_addr >=0 && next_addr <32){
    Serial.println("DATA LIMIT REACHED!!! CANNOT STORE DATA!!!");
    next_addr -= 32; // get stuck!!!
  }
  

  //Serial.printf("DO: %.2f mg/L\n", myData.dissolved_oxygen());
  //delay(7*1000); // 7 second delay between DO checks
}