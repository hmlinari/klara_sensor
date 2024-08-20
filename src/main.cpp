#include <Arduino.h>
#include <Mhz19.h>
#include <SoftwareSerial.h>
#include "MHZ_Sensor.hpp"
#include "ens160_aht21.hpp"

#define START_TASK_ENS_ATH 1
#define START_TASK_MHZ 1

MHZ_Sensor mhzSensor;
ENS160_AHT21_Sensor ens_aht_sensor;


struct m_data{
  unsigned int MHZ_counter;
  int MHZ_CarbonDioxide;
  unsigned int preheat_count;

  unsigned int ENS_counter;
  unsigned int AHT_counter;
  float AHT_temperature;
  float AHT_humidity;
  unsigned int ENS_HP0;
  unsigned int ENS_HP1;
  unsigned int ENS_HP2;
  unsigned int ENS_HP3;
};

void task_MHZ(void * parameter){
    bool ret;
    m_data *data;

    data = (m_data*)parameter;

    data->MHZ_counter = 0; //Start state. While counter is 0, sensor iz in init state.

    //Start preheating mode of sensor
    Serial.println("Preheating sensor MHZ...");
    mhzSensor.preheat(&data->preheat_count);
    Serial.println("Sensor MHZ Ready...");

    while(1){
        int carbonDioxide = mhzSensor.getCarbonDioxide();
        if (carbonDioxide >= 0) {
          //Serial.println(String(carbonDioxide) + " ppm");          
          data->MHZ_CarbonDioxide = carbonDioxide;
          data->MHZ_counter++;
        }
        delay(10000); //Sleep 10 sec.
    }
}


void task_ENS_ATH(void * parameter){
  bool ret;
  m_data *data;

  data = (m_data*)parameter;

  Serial.println("0-3");
  data->ENS_counter = 0;
  data->AHT_counter = 0;
  Serial.println("0-4");

  while(1){
    if (ens_aht_sensor.is_aht20_measurement_available() == true){
      //Get the new temperature and humidity value
      float temperature = ens_aht_sensor.aht20_getTemperature();
      float humidity = ens_aht_sensor.aht20_getHumidity();

      data->AHT_humidity = humidity;
      data->AHT_temperature = temperature;
      data->AHT_counter++;
    }

    if (ens_aht_sensor.is_ens160_measurement_available()) {
      ens_aht_sensor.ens160_measureRaw(true);
      data->ENS_HP0 = ens_aht_sensor.ens160_getHP0();
      data->ENS_HP1 = ens_aht_sensor.ens160_getHP1();
      data->ENS_HP2 = ens_aht_sensor.ens160_getHP2();
      data->ENS_HP3 = ens_aht_sensor.ens160_getHP3();
      data->ENS_counter++;    
    }
    delay(2000);
  }

}

void setup() {
  bool ret;
  Serial.begin(115200);

  Serial.println("Start program.");

  //Init ENS AHT senzor
  ret = ens_aht_sensor.init();
  if(!ret){
    Serial.println("ERROR ERROR ERROR !!! unable to initialize ETH_ATH");
    vTaskDelete(NULL);  // Exit and close curent task
  }

  //Init MHZ senzor
  ret = mhzSensor.init();  //Start sensor initialization
  if (!ret){
    Serial.println("ERROR ERROR ERROR !!! Unable tu initialized MHZ Sensor!!!");
    vTaskDelete(NULL);  // Exit and close curent task
  }

}

void loop() {
  m_data share_data;
  bool start_mhz = false;
  bool start_ens = false;
  bool start_ath = false;
  unsigned int last_mhz_counter = 0;
  unsigned int last_ens_counter = 0;
  unsigned int last_ath_counter = 0;
  unsigned int last_preheat_counter = 0;

  share_data.AHT_counter = 0;
  share_data.MHZ_counter = 0;
  share_data.ENS_counter = 0;

  Serial.println("");


#if START_TASK_MHZ == 1
  Serial.println("Start Task MHZ.");
  xTaskCreate(
    task_MHZ,             // Function that should be called
    "Task MHZ",           // Name of the task (for debugging)
    1000,                 // Stack size (bytes)
    (void *) &share_data, // Parameter to pass
    1,                    // Task priority
    NULL                  // Task handle
  );
  Serial.println("Task MHZ created.");
#endif
  
#if START_TASK_ENS_ATH == 1
  Serial.println("Start Task ENS_ATH.");
  xTaskCreate(
    task_ENS_ATH,         // Function that should be called
    "Task ENS_ATH",       // Name of the task (for debugging)
    10000,                 // Stack size (bytes)
    (void *) &share_data, // Parameter to pass
    1,                    // Task priority
    NULL                  // Task handle
  );
  Serial.println("Task ENS_ATH created.");
#endif

  Serial.println("");
  Serial.println("Waiting for sensor to be initalized.");
  while(1){
    //Check if sensor iz initalized
    if(start_mhz == false && share_data.MHZ_counter != 0){
      start_mhz = true;
      Serial.println("Sensor MHZ is INITALIZED.");            
    }
    if(start_ath == false && share_data.AHT_counter != 0){
      start_ath = true;
      Serial.println("Sensor ATH is INITALIZED.");            
    }
    if(start_ens == false && share_data.ENS_counter != 0){
      start_ens = true;
      Serial.println("Sensor MHZ is INITALIZED.");            
    }

    //Print new value is we have new readings
    if (last_ath_counter != share_data.AHT_counter){
      last_ath_counter = share_data.AHT_counter;
      Serial.print("ATH[");
      Serial.print(share_data.AHT_counter) ;
      Serial.print("] - temperature:");
      Serial.print(share_data.AHT_temperature, 2);
      Serial.print(" C\t");
      Serial.print("Humidity: ");
      Serial.print(share_data.AHT_humidity, 2);
      Serial.println("% RH");
    }
    if (last_ens_counter != share_data.ENS_counter){
      last_ens_counter = share_data.ENS_counter;
      Serial.print("ENS["); Serial.print(share_data.ENS_counter); Serial.print("] - ");
      Serial.print("R HP0: ");Serial.print(ens_aht_sensor.ens160_getHP0());Serial.print("Ohm\t");
      Serial.print("R HP1: ");Serial.print(ens_aht_sensor.ens160_getHP1());Serial.print("Ohm\t");
      Serial.print("R HP2: ");Serial.print(ens_aht_sensor.ens160_getHP2());Serial.print("Ohm\t");
      Serial.print("R HP3: ");Serial.print(ens_aht_sensor.ens160_getHP3());Serial.println("Ohm");
    }
    if (last_mhz_counter != share_data.MHZ_counter && share_data.preheat_count == UINT_MAX){
      last_mhz_counter = share_data.MHZ_counter;
      Serial.print("MHZ["); Serial.print(share_data.MHZ_counter); Serial.print("] - ");
      Serial.println(String(share_data.MHZ_CarbonDioxide) + " ppm");
    }

    if (share_data.preheat_count != UINT_MAX && last_preheat_counter != share_data.preheat_count){
      last_preheat_counter = share_data.preheat_count;
      if ((last_preheat_counter % 10) == 0){
        Serial.println("EMS[preheating - " + String(share_data.preheat_count)+" sec.]");
      }
    }

  }
  delay(500);
}
