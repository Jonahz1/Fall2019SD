#include <Wire.h>
#include "eeprom.h"

#define EEPROM_ADDR 0x50

eeprom::eeprom(){}

bool eeprom::init()
{
    return true; 
}


byte eeprom::read(unsigned int eeaddress) {
    byte rdata = 0xFF;
    
    Wire.beginTransmission(EEPROM_ADDR);
    //address is sent in two bytes
    Wire.write((int)(eeaddress >> 8));   // MSB 
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    
    Wire.requestFrom(EEPROM_ADDR,1);
    
    if (Wire.available()) rdata = Wire.read();
 
    return rdata;
}

void eeprom::write(unsigned int eeaddress, byte data)
{
    //Since there are 32,000 addresses in our EEPROM, we have to send the address in two bytes; MSB & LSB
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(data);
    Wire.endTransmission();

    delay(5);

    //Serial.println("EEPROM Write Complete");
}

void eeprom::write_temp(int eeaddress, float data)
{
    int a = int(data);
    
    int b = int((data - a) * 100);
    
    write(eeaddress, byte(a));
    write(eeaddress+1, byte(b));
}

float eeprom::read_temp(int eeaddress){
    eeaddress = 0; 
    while (eeaddress < 20){
        Serial.print(read(eeaddress));
        Serial.print(".");
        Serial.print(read(eeaddress+1));
        Serial.print(" read from address ");
        Serial.println(eeaddress);

        eeaddress = eeaddress + 2; 
    }
    return 0.0;
}