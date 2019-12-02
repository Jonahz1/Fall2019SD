#include <Wire.h>
#include "eeprom.h"

#define EEPROM_ADDR 0x50
#define EEPROM_MAX_WRITE_BUFFER 32

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

void eeprom::page_write(unsigned int eeaddress, byte *data_arr, int length)
{
    if(length > EEPROM_MAX_WRITE_BUFFER){ // prevent going over limit!!!
        length = EEPROM_MAX_WRITE_BUFFER;
        Serial.println("ERROR! PAGE WRITE ONLY SUPPORTS 32 bytes. Only 32 have been written");
    }
    //Since there are 32,000 addresses in our EEPROM, we have to send the address in two bytes; MSB & LSB
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    for(int i = 0; i<length; i++){
        Wire.write(data_arr[i]);
    }
    Wire.endTransmission();
    delay(5);
    Serial.println("EEPROM Page Write Complete");
}

void eeprom::page_read (unsigned int eeaddress, byte*data_arr, int length) // read to data_arr
{
    byte rdata = 0xFF;
    int i = 0; // current index
    
    Wire.beginTransmission(EEPROM_ADDR);
    //address is sent in two bytes
    Wire.write((int)(eeaddress >> 8));   // MSB 
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    
    Wire.requestFrom(EEPROM_ADDR,length);
    
    while (Wire.available())
    {
        rdata = Wire.read();
        data_arr[i] = rdata;
        i++;
    }
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

/////////////////////////
// config bit operations
/////////////////////////
void eeprom::read_config_raw(){ // testing only! prints to serial
    byte temp_arr[32];
    page_read(0,temp_arr,32);
    Serial.printf("RAW CONFIG BITS: ");
    for(int i = 0; i < 32; i++){
        Serial.printf("0x%X ", temp_arr[i]);
    }
    Serial.println();
}

bool eeprom::write_id(byte id)
{
    write(eeprom_addr_id, id); // write to config bits
    return true;
}

bool eeprom::write_mode(byte mode)
{
    write(eeprom_addr_mode, mode); // write to config bits
    return true;
}

bool eeprom::write_next_address(int next_addr)
{
    byte temp[4]; // converting to bytes
    temp[0] = (int)((next_addr >> 24) & 0xFF);         temp[1] = (int)((next_addr >> 16) & 0xFF);
    temp[2] = (int)((next_addr >> 8) & 0xFF);         temp[3] = (int)((next_addr >> 0) & 0xFF);
    page_write(eeprom_addr_next_addr,temp,4);

    return true;
}

bool eeprom::write_interval(unsigned int interval)
{
    byte temp[4]; // converting to bytes
    temp[0] = (int)((interval >> 24) & 0xFF);         temp[1] = (int)((interval >> 16) & 0xFF);
    temp[2] = (int)((interval >> 8) & 0xFF);         temp[3] = (int)((interval >> 0) & 0xFF);
    page_write(eeprom_addr_interval,temp,4);

    return true;
}
byte eeprom::read_id()
{
    return read(eeprom_addr_id); // write to config bits
}

byte eeprom::read_mode()
{
    return read(eeprom_addr_mode); // write to config bits
}

int eeprom::read_next_address()
{
    byte temp[4];
    page_read(eeprom_addr_next_addr,temp,4);
    return ((temp[0]<<24) + (temp[1]<<16) + (temp[2]<<8) + (temp[3])); // convert bytes to a single int value
}
unsigned int eeprom::read_interval()
{
    byte temp[4];
    page_read(eeprom_addr_interval,temp,4);
    return ((temp[0]<<24) + (temp[1]<<16) + (temp[2]<<8) + (temp[3]));
}