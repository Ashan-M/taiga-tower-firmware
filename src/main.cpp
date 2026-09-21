#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "main.h"
#include "globals.h"
#include "pods.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"

bool wifi = false;
unsigned long lastPoll = 0;
const int pollInterval = 10000;
static int currentPod = 0;
static unsigned long lastPodProcess = 0;

TaskHandle_t sensorHttpTaskHandle = NULL;
TaskHandle_t controlPodTaskHandle = NULL;

void checkOTAInfo() {
    const esp_partition_t* running = esp_ota_get_running_partition();

    Serial.println("=== OTA INFO ===");

    if (running) {
        Serial.println("Firmware: " FIRMWARE_VERSION);
        Serial.printf("Running partition: %s\n", running->label);
        Serial.printf("Address: 0x%X\n", running->address);
        Serial.printf("Size: %u bytes\n", running->size);
    }

    Serial.printf("Free sketch space: %u bytes\n", ESP.getFreeSketchSpace());

    Serial.println("================");
    }

// WiFi
// const char *ssid = "ashan"; // Enter your Wi-Fi name
// const char *password = "12345678";  // Enter Wi-Fi password

bool mqttStat = false;

void collectAndSendPodData()
{
    for (int i = 0; i < NUM_PODS; i++)
    {
        if (!pods[i].active)
            continue;

        readSensor(i);
    }

    sendPodData();
    checkFloater();
}

void sensorHttpTask(void *parameter)
{
    while (true)
    {
        if (wifi && !globalSleepMode)
        {
            collectAndSendPodData();
        }

        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

void controlPodTask(void *parameter){
    while(true) {
        if(!globalSleepMode){
            for(int i=0; i<NUM_PODS; i++){
                controlPod(i);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void activePumpPins() {
    for (int i = 0; i < NUM_PODS; i++) {
        pinMode(motorPins[i], OUTPUT);
        digitalWrite(motorPins[i], LOW);
    }
}

void turnOffAllPWMChannels() {
    for (int i=0; i<6; i++);
}

void setup() {
    Serial.begin(115200);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(FLOAT_PIN, INPUT_PULLUP);
    digitalWrite(CLK_PIN, LOW);
    pinMode(WIFI_LED, OUTPUT);
    digitalWrite(WIFI_LED, LOW);
    checkOTAInfo();
    connectToWifi();
    if(wifi){
        performOTA("http://192.168.8.170:8000/firmware.bin");
    }
//     activePumpPins();
//     setupPumpTimers();
//     connectToWifi();
    
//     delay(2000);
//     if(wifi){
//     bool mqttInit = initMQTT();
//     if(mqttInit) {
//       mqttStat = connectMQTT();
//     }

  
//     loadActivePods();
//     initLightControl();
//     startMQTTTaskProcess();
//     loadMasterControllers("masterLight", globalLightCmd);
//     loadMasterControllers("sleepMode", globalSleepMode);
//     loadMasterControllers("masterPump", globalPumpCmd);
// }

//     xTaskCreatePinnedToCore(
//     sensorHttpTask,
//     "Sensor HTTP Task",
//     8192,
//     NULL,
//     1,
//     &sensorHttpTaskHandle,
//     1
// );
// xTaskCreatePinnedToCore(
//     controlPodTask,
//     "Control Pod Task",
//     1024*2,
//     NULL,
//     3,
//     &controlPodTaskHandle,
//     1
// );

    

}

void handleMqtt() {
  if(!mqttStat && wifi){
      connectMQTT();
    }
    mqttClient();
}


void loop()
{
    // if (wifiState == WIFI_AP_MODE)
    // {
    //     handleClient();
    // }

    // if (!mqttStat && wifi)
    // {
    //     connectMQTT();
    // }

    // mqttClient();

    // // Other very short non-blocking operations
    // updateLED();

    // delay(1);
}