#include "Arduino.h"

class sensor {
    public: 
        sensor();
        bool init();
        void i2c_scan();
        float dissolved_oxygen();
        float pressure(float conversion = 1.0f);
        float temperature();
        void read();


    private:
        //used for bar02
        uint16_t C[8];
	    uint32_t D1, D2;
	    int32_t TEMP;
	    int32_t P;
	    uint8_t _model;
        uint8_t crc4(uint16_t n_prom[]);
        void calculate();
        
};