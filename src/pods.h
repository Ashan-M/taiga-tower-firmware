#pragma once
#include <Arduino.h>

#define NUM_PODS 6

struct PodConfig
{
    char podID[20]="";
    char podName[10]="";
    char plantID[10]="";
    char hardwareID[16]="";

    float targetMoisture=0.0;
    float targetLight=0.0;

    bool active=false;
};

struct Pod
{
    bool active=false;
    bool hardwareConnected = false;
    char hardwareID[16] = "";

    char podID[20] = "";
    char podName[32] = "";
    char plantID[20] = "";

    float targetMoisture = 0.0;
    float targetLight = 0.0;

    // Hardware mapping
    int slavePin;
    int pumpPin;
    int pwmChannel;

    // Runtime values
    int currentMoisture = 0;

    bool pumpState = false;
    bool lightState = false;
    int lightLevel = 0;


};

struct PodRuntime {
    bool pumpTimerActive;
    unsigned long pumpStartTime;
    unsigned long pumpDuration;
};

extern PodRuntime podRuntime[NUM_PODS];
extern Pod pods[NUM_PODS];

bool savePodConfig (int slot, const Pod& config);

bool loadPodConfig(int slot, Pod& config);

bool removePodConfig(int slot);
void loadActivePods();