#include "Arduino.h"

class eeprom {
    public: 
        eeprom();
        bool init();
        void write(unsigned int eeaddress, byte data);
        byte read(unsigned int eeaddress);
        void page_write(unsigned int eeaddress, byte *data_arr, int length);
        void page_read(unsigned int eeaddress, byte*data_arr, int length); // read to data_arr

        void write_temp(int eeaddress, float data);
        float read_temp(int eeaddress);

        // configuration area of EEPROM
        void read_config_raw(); // testing only! prints to serial
        bool write_id(byte id);
        bool write_mode(byte mode);
        bool write_next_address(int next_addr);
        bool write_interval(unsigned int interval);
        byte read_id();
        byte read_mode();
        int  read_next_address();
        unsigned int read_interval();
    private:
        const unsigned int eeprom_addr_id        = 0x0; // 0
        const unsigned int eeprom_addr_mode      = 0x1; // 1
        const unsigned int eeprom_addr_next_addr = 0x2; // 2
        const unsigned int eeprom_addr_interval  = 0xA; // 10
                                               // 14 would be next
};