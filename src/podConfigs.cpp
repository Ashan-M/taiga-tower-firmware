#include <Arduino.h>
#include <Preferences.h>
#include "pods.h"

Preferences preferences;


// --------------------------------------------------
// Save Pod Configuration
// --------------------------------------------------

bool savePodConfig(
    int slot,
    const Pod& config
    
)
{
    Serial.print("[POD] Received slot: ");
        Serial.println(slot);
    if (slot < 0 || slot > NUM_PODS)
    {
        Serial.print("[POD] Received slot: ");
        Serial.println(slot);
        Serial.println("[NVS] Invalid slot!");
        return false;
    }

    char key[10];

    // Physical slot 0 -> pod_1
    snprintf(
        key,
        sizeof(key),
        "pod_%d",
        slot
    );

    if (!preferences.begin("podConfig", false))
    {
        Serial.println(
            "[NVS] Failed to open namespace!"
        );

        return false;
    }

    size_t written =
        preferences.putBytes(
            key,
            &config,
            sizeof(Pod)
        );

    preferences.end();

    if (written != sizeof(Pod))
    {
        Serial.print(
            "[NVS] Failed to save "
        );

        Serial.println(key);

        return false;
    }

    Serial.print(
        "[NVS] Saved configuration: "
    );

    Serial.println(key);

    return true;
}


// --------------------------------------------------
// Load Pod Configuration
// --------------------------------------------------

bool loadPodConfig(
    int slot,
    Pod& config
)
{
    if (slot < 0 || slot > NUM_PODS)
    {
        Serial.println("[NVS] Invalid slot!");
        return false;
    }

    char key[10];

    snprintf(
        key,
        sizeof(key),
        "pod_%d",
        slot + 1
    );

    if (!preferences.begin("podConfig", true))
    {
        Serial.println(
            "[NVS] Failed to open namespace!"
        );

        return false;
    }

    if (!preferences.isKey(key))
    {
        preferences.end();

        Serial.print(
            "[NVS] No configuration for "
        );

        Serial.println(key);

        return false;
    }

    size_t read =
        preferences.getBytes(
            key,
            &config,
            sizeof(Pod)
        );

    preferences.end();

    if (read != sizeof(Pod))
    {
        Serial.print(
            "[NVS] Invalid data size for "
        );

        Serial.println(key);

        return false;
    }

    Serial.print(
        "[NVS] Loaded configuration: "
    );

    Serial.println(key);

    return true;
}


// --------------------------------------------------
// Remove Pod Configuration
// --------------------------------------------------

bool removePodConfig(
    int slot
)
{
    if (slot < 0 || slot >= NUM_PODS)
    {
        Serial.println("[NVS] Invalid slot!");
        return false;
    }

    char key[10];

    snprintf(
        key,
        sizeof(key),
        "pod_%d",
        slot + 1
    );

    if (!preferences.begin("podConfig", false))
    {
        Serial.println(
            "[NVS] Failed to open namespace!"
        );

        return false;
    }

    bool removed =
        preferences.remove(key);

    preferences.end();

    if (removed)
    {
        Serial.print(
            "[NVS] Removed configuration: "
        );

        Serial.println(key);
    }
    else
    {
        Serial.print(
            "[NVS] Failed to remove: "
        );

        Serial.println(key);
    }

    return removed;
}


void loadActivePods()
{
    for (int i = 0; i < NUM_PODS; i++)
    {
        Pod config;

        if (!loadPodConfig(i, config))
        {
            // No saved pod in this slot
            continue;
        }

        pods[i].active = config.active;
        strcpy(pods[i].podID, config.podID);
        strcpy(pods[i].podName, config.podName);
        strcpy(pods[i].plantID, config.plantID);
        pods[i].targetLight = config.targetLight;
        pods[i].targetMoisture = config.targetMoisture;
        pods[i].slavePin = config.slavePin;
        pods[i].pumpPin = config.pumpPin;
        pods[i].pwmChannel = config.pwmChannel;
        pods[i].hardwareConnected = true;

        Serial.print("[POD] Loaded slot ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println("Pod ID: " + String(pods[i].podID));
        Serial.println("Pod Name: " + String(pods[i].podName));
        Serial.println("Plant ID: " + String(pods[i].plantID));
        Serial.println("Pod Target Light: " + String(pods[i].targetLight));
        Serial.println("Pod Target Moisture: " + String(pods[i].targetMoisture));
        Serial.println("Connected Pins: " + String(pods[i].slavePin) + ", " + String(pods[i].pumpPin) + ", " + String(pods[i].pwmChannel));
        Serial.println();
    }
}

void saveMasterControllers(String controller, bool state) {
    Preferences prefs;
    prefs.begin(controller.c_str(), false);
    prefs.putBool("state", state);
    prefs.end();

}

void loadMasterControllers(String controller, bool &state) {
    Preferences prefs;
    prefs.begin(controller.c_str(), true);
    state = prefs.getBool("state", false);
    prefs.end();
}

int findPodIndexByID(String podID) {
    for (int i = 0; i < NUM_PODS; i++) {
        if (strcmp(pods[i].podID, podID.c_str()) == 0) {
            return i;
        }
    }

    return -1; // Not found
}