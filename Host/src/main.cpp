#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

void setup() {
  Serial.begin(9600);
  while (!Serial);

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }

  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println();
  Serial.println("LoRa Initializing OK!");
  Serial.println("Starting Host Program");
}
char code;
int myNum; 
void loop() {

  if(Serial.available()){
    code = Serial.read(); 
    myNum = Serial.parseInt();
  }

  switch(code) {
  case 'p':
    // ping code block
    Serial.print("Sending Ping - ID: ");
    Serial.println(myNum); 
    break;
  case 'g':
    Serial.println("sending Get Data ");
    break;
  default:
    int i = 0;
  }

  code = '0'; 
}