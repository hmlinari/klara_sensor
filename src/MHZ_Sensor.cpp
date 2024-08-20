//

#include "MHZ_Sensor.hpp"


MHZ_Sensor::MHZ_Sensor() {
    this->rxPin = RX_PIN_DEFAULT;
    this->txPin = TX_PIN_DEFAULT;
}

void MHZ_Sensor::begin(){

}

bool MHZ_Sensor::init(){
    bool ret;

    //Init software serial
    //SoftwareSerial softwareSerial(13, 12);
    this->softwareSerial = new SoftwareSerial(this->rxPin, this->txPin);
    this->softwareSerial->begin(9600);
    this->sensor = new Mhz19();
    this->sensor->begin(softwareSerial);
  
    ret = this->sensor->setMeasuringRange(Mhz19MeasuringRange::Ppm_5000);  
    if(!ret){
        Serial.println("setMeasuringRange:: Missing Sensor !!!");
        return false;
    }
  
    ret = this->sensor->enableAutoBaseCalibration();
    if(!ret){
        Serial.println("enableAutoBaseCalibration:: Missing Sensor !!!");
        return false;
    }
  return true;
}

void MHZ_Sensor::preheat(unsigned int *count_sec){
    this->preheat_counter = 0;
    *count_sec = 0;
    //Serial.println("Preheating... 3 minutes.");  // Preheating, 3 minutes
    while (!this->sensor->isReady()) {
        delay(1000);
        this->preheat_counter++;
        *count_sec = this->preheat_counter;       
    }
    *count_sec = UINT_MAX;
}