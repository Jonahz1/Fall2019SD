#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(915E6))
  {
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

void loop()
{

  if (Serial.available())
  {
    code = Serial.read();
    myNum = Serial.parseInt();

    // ping code block
    Serial.print("Sending Packet - ID: ");
    Serial.print(code);
    Serial.println(myNum);

    LoRa.beginPacket();
    LoRa.print(code);
    LoRa.print(myNum);
    LoRa.endPacket();
  }

  int packetSize = LoRa.parsePacket();

  if (packetSize)
  {
    String LoRaData;
    // received a packet
    //Serial.print("Received packet '");
    // read packet
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
      Serial.println(LoRaData);
    }
    // char *data = (char *)LoRaData.c_str();

    // for (int i = 0; i < 32; i++)
    // {
    //   Serial.printf("%X ", LoRaData[i]);
    // }
    // Serial.println();
  }
}