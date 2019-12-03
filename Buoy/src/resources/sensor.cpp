#include <Wire.h>
#include "sensor.h"

//bar02 information
#define MS5837_ADDR 0x76
#define MS5837_RESET 0x1E
#define MS5837_ADC_READ 0x00
#define MS5837_PROM_READ 0xA0
#define MS5837_CONVERT_D1_8192 0x4A
#define MS5837_CONVERT_D2_8192 0x5A

sensor::sensor()
{
}

//init function to initialize and calibrate sensors
//this will be run in the main - setup()
bool sensor::init()
{
    //Reset the Bar02 Pressure Sensor, per datasheet
    Wire.beginTransmission(MS5837_ADDR);
    Wire.write(MS5837_RESET);
    Wire.endTransmission();

    delay(10); //wait for reset to complete

    // Read calibration values and CRC
    for (uint8_t i = 0; i < 7; i++)
    {
        Wire.beginTransmission(MS5837_ADDR);
        Wire.write(MS5837_PROM_READ + i * 2);
        Wire.endTransmission();

        Wire.requestFrom(MS5837_ADDR, 2);
        C[i] = (Wire.read() << 8) | Wire.read();
    }

    // Verify that data is correct with CRC
    uint8_t crcRead = C[0] >> 12;
    uint8_t crcCalculated = crc4(C);

    if (crcCalculated == crcRead)
    {
        return true; // Initialization success
    }

    return false; // CRC fail
}

//method to scan for all i2c devices
void sensor::i2c_scan()
{
    Serial.println("I2C Scan");
    byte error, address;
    int nDevices;
    Serial.println("Scanning...");
    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknow error at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
    {
        Serial.println("No I2C devices found\n");
    }
    else
    {
        Serial.println("done\n");
    }
    delay(100);
}

//method will print the DO levels
float sensor::dissolved_oxygen()
{
    char do_data[20];
    byte in_char = 0;
    byte i = 0;
    Wire.beginTransmission(97); //call the circuit by its ID number.
    Wire.write('r');            //transmit the command that was sent through the serial port. r is read DO
    Wire.endTransmission();

    delay(600); // 600ms delay before our reading data is ready

    Wire.requestFrom(97, 20, 1);
    //int code = Wire.read();

    while (Wire.available())
    {                          //are there bytes to receive.
        in_char = Wire.read(); //receive a byte.
        //Serial.println(in_char);
        do_data[i] = in_char; //load this byte into our array.
        i += 1;               //incur the counter for the array element.
        if (in_char == 0)
        {                           //if we see that we have been sent a null command. (null is 0)
            i = 0;                  //reset the counter i to 0.
            Wire.endTransmission(); //end the I2C data transmission.
            break;                  //exit the while loop.
        }
    }

    // Serial.print(do_data[1]);
    // Serial.print(do_data[2]);
    // Serial.print(do_data[3]);
    // Serial.print(do_data[4]);
    // Serial.print(do_data[5]);
    String data;
    for (int i = 1; i <= 5; i++)
    {
        data += do_data[i];
    }

    return data.toFloat();
}

//bar02 pressure sensor supporting method
uint8_t sensor::crc4(uint16_t n_prom[])
{
    uint16_t n_rem = 0;

    n_prom[0] = ((n_prom[0]) & 0x0FFF);
    n_prom[7] = 0;

    for (uint8_t i = 0; i < 16; i++)
    {
        if (i % 2 == 1)
        {
            n_rem ^= (uint16_t)((n_prom[i >> 1]) & 0x00FF);
        }
        else
        {
            n_rem ^= (uint16_t)(n_prom[i >> 1] >> 8);
        }
        for (uint8_t n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & 0x8000)
            {
                n_rem = (n_rem << 1) ^ 0x3000;
            }
            else
            {
                n_rem = (n_rem << 1);
            }
        }
    }

    n_rem = ((n_rem >> 12) & 0x000F);

    return n_rem ^ 0x00;
}

//bar02 pressure sensor supporting method
void sensor::read()
{
    // Request D1 conversion
    Wire.beginTransmission(MS5837_ADDR);
    Wire.write(MS5837_CONVERT_D1_8192);
    Wire.endTransmission();

    delay(20); // Max conversion time per datasheet

    Wire.beginTransmission(MS5837_ADDR);
    Wire.write(MS5837_ADC_READ);
    Wire.endTransmission();

    Wire.requestFrom(MS5837_ADDR, 3);
    D1 = 0;
    D1 = Wire.read();
    D1 = (D1 << 8) | Wire.read();
    D1 = (D1 << 8) | Wire.read();

    // Request D2 conversion
    Wire.beginTransmission(MS5837_ADDR);
    Wire.write(MS5837_CONVERT_D2_8192);
    Wire.endTransmission();

    delay(20); // Max conversion time per datasheet

    Wire.beginTransmission(MS5837_ADDR);
    Wire.write(MS5837_ADC_READ);
    Wire.endTransmission();

    Wire.requestFrom(MS5837_ADDR, 3);
    D2 = 0;
    D2 = Wire.read();
    D2 = (D2 << 8) | Wire.read();
    D2 = (D2 << 8) | Wire.read();

    calculate();
}

void sensor::calculate()
{
    // Given C1-C6 and D1, D2, calculated TEMP and P
    // Do conversion first and then second order temp compensation

    int32_t dT = 0;
    int64_t SENS = 0;
    int64_t OFF = 0;
    int32_t SENSi = 0;
    int32_t OFFi = 0;
    int32_t Ti = 0;
    int64_t OFF2 = 0;
    int64_t SENS2 = 0;

    // Terms called
    dT = D2 - uint32_t(C[5]) * 256l;

    //bar02 model
    SENS = int64_t(C[1]) * 65536l + (int64_t(C[3]) * dT) / 128l;
    OFF = int64_t(C[2]) * 131072l + (int64_t(C[4]) * dT) / 64l;
    P = (D1 * SENS / (2097152l) - OFF) / (32768l);

    // Temp conversion
    TEMP = 2000l + int64_t(dT) * C[6] / 8388608LL;

    //Second order compensation
    if ((TEMP / 100) < 20)
    { //Low temp
        Ti = (11 * int64_t(dT) * int64_t(dT)) / (34359738368LL);
        OFFi = (31 * (TEMP - 2000) * (TEMP - 2000)) / 8;
        SENSi = (63 * (TEMP - 2000) * (TEMP - 2000)) / 32;
    }

    OFF2 = OFF - OFFi; //Calculate pressure and temp second order
    SENS2 = SENS - SENSi;

    TEMP = (TEMP - Ti);

    P = (((D1 * SENS2) / 2097152l - OFF2) / 32768l);
}

float sensor::pressure(float conversion)
{
    return P * conversion / 100.0f;
}

float sensor::temperature()
{
    return TEMP / 100.0f;
}

float sensor::wind_speed(){
    
    int speed = analogRead(34);
    //float y = map(speed, 320,2418,0,32.1);
    return float(speed);
}
