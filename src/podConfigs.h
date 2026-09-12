#pragma once

#include <Arduino.h>


struct Defaults
{
    String messageID;
    String deviceID;
};


struct PodCreation : Defaults
{
    String podID;
    String podName;
    String mode;
    String plantID;
    int podSlot;

    int defaultMoistureLevel = 0;
    int defaultLightIntensity = 0;

    int manualMoistureLevel = 0;
    int manualLightIntensity = 0;

    bool podPump = false;
    bool podLight = false;
};

struct PodDeletation : Defaults
{
    String podID;
};


struct MasterControllers : Defaults
{
    bool masterLight = false;
    bool masterPump = false;
    bool sleepMode = false;

    bool hasMasterLight = false;
    bool hasMasterPump = false;
    bool hasSleepMode = false;
};


struct PodMode : Defaults
{
    String podID;
    String mode;
};

struct PlantConfig
{
    String plantID;
    float moistureLevel;
    float lightIntensity;
};

struct PodControllers : Defaults
{
    String podID;

    bool podPump = false;
    bool podLight = false;

    float manualLightIntensity = 0;
    float manualMoistureLevel = 0;
    int podPumpTimer = 0;

    bool hasPodPump = false;
    bool hasPodLight = false;
    bool hasmanualLightIntensity = false;
    bool hasmanualMoistureLevel = false;
    bool hasPodPumpTimer = false;

};

enum MQTTJobType
{
    MQTT_JOB_MASTER_CONTROL,
    MQTT_JOB_POD_ACTIVATE,
    MQTT_JOB_POD_MODE,
    MQTT_JOB_POD_COMMAND,
    MQTT_JOB_REMOVE_POD,
    MQTT_JOB_UPDDATE_PLANT_CONFIG,
    MQTT_JOB_OTA_UPDATE
};

struct MQTTJob
{
    MQTTJobType type;

    MasterControllers masterControllers;
    PodCreation podCreation;
    PodMode podMode;
    PodControllers podControllers;
    PodDeletation deletePod;
    PlantConfig plantConfig;
};