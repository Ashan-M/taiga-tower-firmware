#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "main.h"
#include "podConfigs.h"
#include "globals.h"
#include "pods.h"

const char* MQTT_BROKER = "broker.emqx.io";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "taiga-tower@123!";
const char* MQTT_PASSWORD = "ashan07505825082";

QueueHandle_t mqttQueue;
#define MQTT_QUEUE_SIZE 10

const char* MQTT_TOPIC_HEADER = "taiga-tower/devices/001002/";
// Command topic
const char* MQTT_MASTER_CONTROL_TOPIC =
    "taiga-tower/devices/001002/command";

const char* MQTT_POD_ACTIVATE_TOPIC = 
    "taiga-tower/devices/001002/activatePod";

const char* MQTT_POD_MODE_TOPIC =
    "taiga-tower/devices/001002/mode";

const char* MQTT_POD_COMMAND_TOPIC = 
    "taiga-tower/devices/001002/podCommand";

const char* MQTT_REMOVE_POD_TOPIC = 
    "taiga-tower/devices/001002/removePod";

const char* UPDATE_PLANT_CONFIG_TOPIC = 
    "taiga-tower/plants/update";

const char* MQTT_OTA_UPDATES_TOPIC = 
    "taiga-tower/devices/otaUpdate";

const char* topics[] = {
    MQTT_MASTER_CONTROL_TOPIC,
    MQTT_POD_ACTIVATE_TOPIC,
    MQTT_POD_MODE_TOPIC,
    MQTT_POD_COMMAND_TOPIC,
    MQTT_REMOVE_POD_TOPIC,
    UPDATE_PLANT_CONFIG_TOPIC,
    MQTT_OTA_UPDATES_TOPIC
};

WiFiClient espClient;
PubSubClient client(espClient);

void printAck(
    const String& ackTopic,
    JsonDocument& doc
)
{
    String output;

    output += "\n";
    output += "ACK published!\n";

    output += "Topic: ";
    output += ackTopic;
    output += "\n";

    output += "Payload:\n";

    serializeJsonPretty(
        doc,
        output
    );

    output += "\n";

    Serial.print(output);
}

void sendActivatePodAck( const char* messageID, const char* deviceID, const char* podID)
{
    String ackTopic =
        "taiga-tower/devices/" +
        String(deviceID) +
        "/ack/activatePod";

    JsonDocument doc;

    doc["messageID"] = messageID;
    doc["deviceID"] = deviceID;
    doc["podID"] = podID;
    doc["status"] = "success";
    doc["message"] = "Pod activated successfully";

    String ackPayload;

    serializeJson(doc, ackPayload);


    bool result = client.publish(
        ackTopic.c_str(),
        ackPayload.c_str()
    );


    if (result)
{
    Serial.println("ACK published!");
}
    else
    {
        Serial.println();
        Serial.println("Failed to publish ACK!");

        Serial.print("MQTT state: ");
        Serial.println(client.state());
    }
}


void callback(
    char* topic,
    byte* payload,
    unsigned int length
)
{
    Serial.println();
    Serial.println("================================");
    Serial.println("MQTT MESSAGE RECEIVED");
    Serial.println("================================");

    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Payload length: ");
    Serial.println(length);
    Serial.print("Payload: ");

    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }

    Serial.println();


    JsonDocument doc;

    DeserializationError error =
        deserializeJson(
            doc,
            payload,
            length
        );


    if (error)
    {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());

        return;
    }
    MQTTJob job;
    if (strcmp(topic, topics[0]) == 0) {
        job.type = MQTT_JOB_MASTER_CONTROL;
        job.masterControllers.messageID = doc["messageID"].as<String>();
        job.masterControllers.deviceID = doc["deviceID"].as<String>();
        if(doc["masterLight"].is<bool>()){
            job.masterControllers.masterLight = doc["masterLight"].as<bool>();
            job.masterControllers.hasMasterLight = true;
        }
        if(doc["masterPump"].is<bool>()){
            job.masterControllers.masterPump = doc["masterPump"].as<bool>();
            job.masterControllers.hasMasterPump = true;
        }
        if(doc["sleepMode"].is<bool>()){
            job.masterControllers.sleepMode = doc["sleepMode"].as<bool>();
            job.masterControllers.hasSleepMode = true;
        }
    }

    else if (strcmp(topic, topics[1]) == 0) {
        job.type = MQTT_JOB_POD_ACTIVATE;
        job.podCreation.messageID = doc["messageID"].as<String>();
        job.podCreation.deviceID = doc["deviceID"].as<String>();
        job.podCreation.podName = doc["podName"].as<String>();
        job.podCreation.podID = doc["podID"].as<String>();
        job.podCreation.podSlot = doc["slot"].as<int>();
        job.podCreation.plantID = doc["plantID"].as<String>();
        job.podCreation.mode = doc["mode"].as<String>();
        job.podCreation.defaultMoistureLevel = doc["defaultMoistureLevel"];
        job.podCreation.defaultLightIntensity = doc["defaultLightIntensity"];
        job.podCreation.manualMoistureLevel = doc["maualMoistureLevel"];
        job.podCreation.manualLightIntensity = doc["manualLightIntensity"];
        job.podCreation.podLight = doc["podLight"];
        job.podCreation.podPump = doc["podPump"];
    }

    else if (strcmp(topic, topics[2]) == 0) {
        job.type = MQTT_JOB_POD_MODE;
        job.podMode.messageID = doc["messageID"].as<String>();;
        job.podMode.deviceID = doc["deviceID"].as<String>();;
        job.podMode.podID = doc["podID"].as<String>();;
        job.podMode.mode = doc["mode"].as<String>();;
    }

    else if (strcmp(topic, topics[3]) == 0) {
        job.type = MQTT_JOB_POD_COMMAND;
        job.podControllers.messageID = doc["messageID"].as<String>();;
        job.podControllers.deviceID = doc["deviceID"].as<String>();;
        job.podControllers.podID = doc["podID"].as<String>();
        if(doc["podLight"].is<bool>()){
            job.podControllers.podLight = doc["podLight"].as<bool>();
            job.podControllers.hasPodLight = true;
        }
        if(doc["podPump"].is<bool>()){
            job.podControllers.podPump = doc["podPump"].as<bool>();
            job.podControllers.podPump = true;
        }
        if(doc["manualMoistureLevel"].is<float>()){
            job.podControllers.manualMoistureLevel = doc["manualMoistureLevel"].as<float>();
            job.podControllers.hasmanualMoistureLevel = true;
        }
        if(doc["manualLightIntensity"].is<float>()){
            job.podControllers.manualLightIntensity = doc["manualLightIntensity"].as<float>();
            job.podControllers.hasmanualLightIntensity = true;
        }
        if(doc["podPumpTimer"].is<int>()){
            job.podControllers.podPumpTimer = doc["podPumpTimer"].as<int>();
            job.podControllers.hasPodPumpTimer = true;
        }
    }

    else if (strcmp(topic, topics[4]) == 0){
        job.type = MQTT_JOB_REMOVE_POD;
        job.deletePod.messageID = doc["messageID"].as<String>();
        job.deletePod.deviceID = doc["deviceID"].as<String>();
        job.deletePod.podID = doc["podID"].as<String>();
    }

    else if (strcmp(topic, topics[5]) == 0){
        // job.type = MQTT_JOB_REMOVE_POD;
    }

    if (xQueueSend(
        mqttQueue, &job, 0
    ) != pdPASS){
        Serial.println("MQTT QUEUE FULL");
    } else {
        Serial.println("MQTT JOD ADDED TO QUEUE");
    }

    sendActivatePodAck(
        doc["messageID"],
        doc["deviceID"],
        doc["podID"]
    );
}



bool initMQTT() {
    Serial.println("Initializing MQTT...");
    client.setServer(
        MQTT_BROKER,
        MQTT_PORT
    );
    client.setBufferSize(512);
    client.setCallback(callback);

    mqttQueue = xQueueCreate(MQTT_QUEUE_SIZE, sizeof(MQTTJob));
    if (mqttQueue == NULL)
    {
        Serial.println("Error: Failed to create the MQTT QUEUE");
        return false;
    }
    Serial.println("Created MQTT QUEUE");
    return true;
}

void mqttProcessingTask(void* parameter) {
    MQTTJob job;
    while (true)
    {
        if(
            xQueueReceive(
                mqttQueue, &job, portMAX_DELAY
            ) == pdPASS
        )
        {
            switch (job.type)
            {
            case MQTT_JOB_MASTER_CONTROL:
                Serial.println("Executing MQTT_JOB_MASTER_CONTROL");
                if (job.masterControllers.hasMasterLight){
                    updateMasterControllers("masterLight", job.masterControllers.masterLight);
                } 
                if(job.masterControllers.hasSleepMode){
                    updateMasterControllers("sleepMode", job.masterControllers.sleepMode);
                }

                break;

            case  MQTT_JOB_POD_ACTIVATE:
            {
                Serial.println("Executing MQTT_JOB_POD_ACTIVATE");
                int slot = job.podCreation.podSlot;
                if (slot <1 || slot > NUM_PODS)
                {
                    Serial.print("[POD] Received slot: ");
                    Serial.println(job.podCreation.podSlot);
                    Serial.println("[POD] invalid Slot");
                    break;
                }

                Pod newPod;

                newPod.active = true;
                strncpy(newPod.podID, job.podCreation.podID.c_str(), sizeof(newPod.podID) -1);
                strncpy(newPod.podName, job.podCreation.podName.c_str(), sizeof(newPod.podName) -1);
                strncpy(newPod.plantID, job.podCreation.plantID.c_str(), sizeof(newPod.plantID) -1);
                newPod.targetLight = job.podCreation.defaultLightIntensity;
                newPod.targetMoisture = job.podCreation.defaultMoistureLevel;
                newPod.slavePin = slavePins[slot - 1];
                newPod.pumpPin = motorPins[slot - 1];
                newPod.pwmChannel = lightChannels[slot - 1];
                if (!savePodConfig(slot, newPod))
                {
                    Serial.println("[POD] Failed to save configuration");
                    break;
                }
                int index = slot - 1;
                pods[index].active = true;
                strncpy(pods[index].podID, newPod.podID, sizeof(pods[index].podID) - 1);
                strncpy(pods[index].podName, newPod.podName, sizeof(pods[index].podName) - 1);
                strncpy(pods[index].plantID, newPod.plantID, sizeof(pods[index].plantID) - 1);
                pods[index].targetLight = newPod.targetLight;
                pods[index].targetMoisture = newPod.targetMoisture;
                pods[index].targetLight = newPod.targetLight;

                Serial.print("[POD] Activate slot ");
                Serial.println(slot);
                
                break;
            }
            case MQTT_JOB_POD_COMMAND:
            {
                Serial.println("Executing MQTT_JOB_POD_COMMAND");
                Serial.println("Searching index " + job.podControllers.podID);
                int podIndex = findPodIndexByID(job.podControllers.podID);
                    if(podIndex == -1){
                        Serial.println("Pod ID not found");
                        break;
                    }
                Pod &p = pods[podIndex];

                if(job.podControllers.hasPodPumpTimer) {
                    Serial.print("Controll podPump: ");
                    Serial.println(podIndex);
                    startPumpTimer(podIndex, job.podControllers.podPumpTimer);
                    
                }
                if(job.podControllers.hasmanualLightIntensity) {
                    Serial.println("Sert manual Light Intensity");
                    Serial.println(p.pwmChannel);
                    if(job.podControllers.podLight && globalLightCmd){
                    controlLight(p.pwmChannel, p.targetLight);
                }
                }
                if(job.podControllers.hasmanualMoistureLevel) {
                    Serial.println("Set manual Moisture Level");
                }

                break;
            }

            case MQTT_JOB_REMOVE_POD:
            {
                    Serial.println("Deleting POD: " + job.deletePod.podID);
                    Serial.println("Searching index " + job.deletePod.podID);
                    int podIndex = findPodIndexByID(job.deletePod.podID);
                        if(podIndex == -1){
                            Serial.println("Pod ID not found");
                            break;
                        }
                    removePodConfig(podIndex);
                    break;
                    }
            

            }

            
        }
    }
    
}

void startMQTTTaskProcess() {
    xTaskCreate(
    mqttProcessingTask,
    "MQTTProcessing",
    8192,
    NULL,
    1,
    NULL
    );
    Serial.println("Stating MQTT Process Tasks");
}

bool connectMQTT()
{
    Serial.println();
    Serial.println("Connecting to MQTT...");

    String clientId = "taiga-001002";
    clientId +=
        String(
            (uint32_t)ESP.getEfuseMac(),
            HEX
        );

    Serial.print("Client ID: ");
    Serial.println(clientId);

    if (
        client.connect(
            clientId.c_str(),
            MQTT_USERNAME,
            MQTT_PASSWORD
        )
    )
    {
        Serial.println("MQTT connected!");

        Serial.print("MQTT state: ");
        Serial.println(client.state());

        bool result1 = client.subscribe(topics[0]);
        bool result2 = client.subscribe(topics[1]);
        bool result3 = client.subscribe(topics[2]);
        bool result4 = client.subscribe(topics[3]);
        bool result5 = client.subscribe(topics[4]);
        bool result6 = client.subscribe(topics[5]);
        bool result7 = client.subscribe(topics[6]);


        if(!result1){
            Serial.print("Failed to subscribe ");
            Serial.println(MQTT_MASTER_CONTROL_TOPIC);
        }
        if(!result2){
            Serial.print("Failed to subscribe ");
            Serial.println(MQTT_POD_ACTIVATE_TOPIC);
        }
        if(!result3){
            Serial.print("Failed to subscribe ");
            Serial.println(MQTT_POD_COMMAND_TOPIC);
        }
        if(!result4){
            Serial.print("Failed to subscribe ");
            Serial.println(MQTT_POD_MODE_TOPIC);
        }
        if(!result5){
            Serial.print("Failed to subscribe ");
            Serial.println(MQTT_MASTER_CONTROL_TOPIC);
        }
        if(!result6){
            Serial.print("Failed to subscribe ");
            Serial.println(MQTT_POD_ACTIVATE_TOPIC);
        }
        if(!result7){
            Serial.print("Failed to subscribe ");
            Serial.println(MQTT_POD_COMMAND_TOPIC);
        }



        return true;
    }

    Serial.print("MQTT connection FAILED. State = ");
    Serial.println(client.state());
    return false;
}

void mqttClient() {
    client.loop();
}


