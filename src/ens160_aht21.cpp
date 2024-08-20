#include "ens160_aht21.hpp"


ENS160_AHT21_Sensor::ENS160_AHT21_Sensor(){

}

bool ENS160_AHT21_Sensor::init(int ens160_i2c_addr){
      
    Serial.println("------------------------------------------------------------");
    Serial.println("ENS160 - Digital air quality sensor");
    Serial.println();
    Serial.println("Sensor readout in custom mode");
    Serial.println();
    Serial.println("------------------------------------------------------------");
    
    this->ens160 = new ScioSense_ENS160(ens160_i2c_addr);
    this->aht20 = new AHT20();

    delay(1000);
    Serial.print("ENS160...");

    ens160->setI2C(21, 22);
    bool ok = ens160->begin();

    if(!ok) return false;

    if (aht20->begin() == false)
    {
        Serial.println("AHT20 not detected. Please check wiring. Freezing.");
        return false;
    }
    Serial.println("AHT20 acknowledged.");

    Serial.println(ens160->available() ? "done." : "failed!");
    if (ens160->available()) {
        // Print ENS160 versions
        Serial.print("\tRev: "); Serial.print(ens160->getMajorRev());
        Serial.print("."); Serial.print(ens160->getMinorRev());
        Serial.print("."); Serial.println(ens160->getBuild());

        Serial.print("\tCustom mode ");
        ens160->initCustomMode(3);                                     // example has 3 steps, max. 20 steps possible
    
        // Step time is a multiple of 24ms and must not be smaller than 48ms
        ens160->addCustomStep(48, 0, 0, 0, 0, 80, 80, 80, 80);         // Step 1: 48ms, no measurments, all hotplates at low temperatures 
        ens160->addCustomStep(196, 0, 0, 0, 0, 160, 215, 215, 200);    // Step 2: 196ms, no measurments, all hotplates at medium temperatures 
        ens160->addCustomStep(600, 1, 1, 1, 1, 250, 350, 350, 325);    // Step 3: 600ms, measurments done, all hotplates at high temperatures 
        Serial.println(ens160->setMode(ENS160_OPMODE_CUSTOM) ? "done." : "failed!");
        return true;
    }else{
        return false;
    }
}

bool ENS160_AHT21_Sensor::init(){
    return(this->init(ENS160_I2CADDR_1));
}

