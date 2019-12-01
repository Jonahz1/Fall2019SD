#include "Arduino.h"

class eeprom {
    public: 
        eeprom();
        bool init();
        void write(unsigned int eeaddress, byte data);
        byte read(unsigned int eeaddress);

        void write_temp(int eeaddress, float data);
        float read_temp(int eeaddress);
    private:
        
};