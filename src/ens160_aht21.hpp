#ifndef ENS160_AHT21_DEF
#define ENS160_AHT21_DEF

#include "ScioSense_ENS160.h"  // ENS160 library
#include <AHT20.h>

class ENS160_AHT21_Sensor
{

    private:
        ScioSense_ENS160 *ens160;
        AHT20 *aht20;

            
    public:
        ENS160_AHT21_Sensor();
        bool init();
        bool init(int ens160_i2c_addr);
        bool is_ens160_measurement_available(){return this->ens160->available();};
        bool ens160_measureRaw(bool waitForNew){return this->ens160->measureRaw(waitForNew);};
        uint32_t ens160_getHP0(){return ens160->getHP0();};
        uint32_t ens160_getHP1(){return ens160->getHP1();};
        uint32_t ens160_getHP2(){return ens160->getHP2();};
        uint32_t ens160_getHP3(){return ens160->getHP3();};
        
        bool is_aht20_measurement_available(){return this->aht20->available();};
        float aht20_getTemperature(){return this->aht20->getTemperature();};
        float aht20_getHumidity(){return this->aht20->getHumidity();};
        
};

#endif