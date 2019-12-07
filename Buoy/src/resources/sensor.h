#include "Arduino.h"
#include <Wire.h>


class sensor {
    public: 
        sensor();
        bool init();
        bool init_bar02_above();
        bool init_bar02_below();
        void i2c_scan();
        float get_dissolved_oxygen();
        float get_above_pressure(float conversion = 1.0f);
        float get_above_temperature();
        float get_below_pressure(float conversion = 1.0f);
        float get_below_temperature();
        void read_above_bar02();
        void read_below_bar02();
        float wind_speed();
        
    private:
        //used for bar02
        uint16_t C[8];
	    uint32_t D1, D2;
	    int32_t TEMP;
	    int32_t P;
	    uint8_t _model;
        uint8_t crc4(uint16_t n_prom[]);
        void calculate();

        uint16_t C_b[8];
	    uint32_t D1_b, D2_b;
	    int32_t TEMP_b;
	    int32_t P_b;
	    uint8_t _model_b;
        //uint8_t crc4_b(uint16_t n_prom[]);
        void calculate_b();

        TwoWire above_sensors = TwoWire(0);
        TwoWire below_sensors = TwoWire(1);
        
};