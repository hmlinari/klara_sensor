#ifndef MHZ_SENSOR_DEF
#define MHZ_SENSOR_DEF

#include <Arduino.h>
#include <Mhz19.h>
#include <SoftwareSerial.h>

#define RX_PIN_DEFAULT 13
#define TX_PIN_DEFAULT 12

class MHZ_Sensor
{

    private:
        Mhz19 *sensor;
        SoftwareSerial *softwareSerial;
        int8_t rxPin;
        int8_t txPin; 
        unsigned int preheat_counter;       
    public:
        MHZ_Sensor();
        void setPin(int8_t r, int8_t t){this->rxPin = r; this->txPin=t;};
        void begin();
        bool init();
        int getCarbonDioxide(){return this->sensor->getCarbonDioxide();};
        void preheat(unsigned int *count_sec);
 

};

#endif