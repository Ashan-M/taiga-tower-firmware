#include <Arduino.h>
extern bool wifi;

// wifi_AP.cpp
void startAP();
void connectToWifi();
void clearNVS();
void handleClient();

// mqtt.cpp
bool connectMQTT();
bool initMQTT();
void mqttClient();
void startMQTTTaskProcess();

// control.cpp
void initLightControl();
void controlLight(int channel, int brightness);
void controlWaterPump(uint8_t podNo, bool on);
void checkFloater();
void updateLED();
void updateMasterControllers(String controller, bool state);
void pumpTimerCallback(TimerHandle_t timer);
void setupPumpTimers();
void startPumpTimer(uint8_t podNo, uint32_t durationSeconds);

// slaves.cpp
void initSlave(int slot);
void executeCommand(uint8_t cmd, bool expectResponse, int activePin);
void initPodStructs();
void updateFirebasePod(String pod);
void controlPod(int i);
void readSensor(int i);
void handlePodInit(int i);
void activateSleepMode();

// podConfig.cpp
struct PodData
{
  bool isActive;
  char podID[20];
  char podName[10];
  int podPin;
  float moistureLevel;
  float lightIntensity;
};

void saveMasterControllers(String controller, bool state);
void loadMasterControllers(String controller, bool &state);
bool removePodConfig(
    int slot
);
int findPodIndexByID(String podID);

// dataLog.cpp
bool sendPodData();
