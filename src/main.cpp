#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Mhz19.h>
#include <SoftwareSerial.h>
#include "MHZ_Sensor.hpp"
#include "ens160_aht21.hpp"

const char* ssid = "WLAN_HM";
const char* password = "homehmtmim";

const char* serverName = "https://script.google.com/macros/s/AKfycbzHvtF-mGM77NQ7OcHfCvdGa0f0o5J5aYwoGex0Tz-l6LNfoA4hgaTLDe1N_7Hb-eTPFw/exec";
#define START_TASK_ENS_ATH 1
#define START_TASK_MHZ 1

MHZ_Sensor mhzSensor;
ENS160_AHT21_Sensor ens_aht_sensor;

struct m_data {
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

m_data share_data;  // Global data structure

void sendToGoogleSheets() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String url = String(serverName) + "?MHZ_CarbonDioxide=" + String(share_data.MHZ_CarbonDioxide) +
                     "&AHT_temperature=" + String(share_data.AHT_temperature, 2) +
                     "&AHT_humidity=" + String(share_data.AHT_humidity, 2) +
                     "&ENS_HP0=" + String(share_data.ENS_HP0) +
                     "&ENS_HP1=" + String(share_data.ENS_HP1) +
                     "&ENS_HP2=" + String(share_data.ENS_HP2) +
                     "&ENS_HP3=" + String(share_data.ENS_HP3);

        http.begin(url.c_str());

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
}

void task_MHZ(void * parameter) {
    m_data *data = (m_data*)parameter;
    data->MHZ_counter = 0;

    Serial.println("Preheating sensor MHZ...");
    mhzSensor.preheat(&data->preheat_count);
    Serial.println("Sensor MHZ Ready...");

    while (1) {
        int carbonDioxide = mhzSensor.getCarbonDioxide();
        if (carbonDioxide >= 0) {
            data->MHZ_CarbonDioxide = carbonDioxide;
            data->MHZ_counter++;
        }
        delay(10000);  // Read every 10 seconds
    }
}

void task_ENS_ATH(void * parameter) {
    m_data *data = (m_data*)parameter;
    data->ENS_counter = 0;
    data->AHT_counter = 0;

    while (1) {
        if (ens_aht_sensor.is_aht20_measurement_available()) {
            data->AHT_temperature = ens_aht_sensor.aht20_getTemperature();
            data->AHT_humidity = ens_aht_sensor.aht20_getHumidity();
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
        delay(2000);  // Read every 2 seconds
    }
}

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Init sensors
    if (!ens_aht_sensor.init()) {
        Serial.println("ERROR: Unable to initialize ENS_AHT sensor");
        while (1) delay(1000);  // Stay here if sensor init fails
    }

    if (!mhzSensor.init()) {
        Serial.println("ERROR: Unable to initialize MHZ sensor");
        while (1) delay(1000);  // Stay here if sensor init fails
    }

    // Create tasks
    xTaskCreate(task_MHZ, "Task MHZ", 4096, (void*)&share_data, 1, NULL);  // Increased stack size
    xTaskCreate(task_ENS_ATH, "Task ENS_ATH", 8192, (void*)&share_data, 1, NULL);  // Increased stack size
}

void loop() {
    static unsigned long lastSendTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastSendTime >= 60000 || lastSendTime == 0) {
        sendToGoogleSheets();
        lastSendTime = currentTime;
    }

    delay(1000);
}
