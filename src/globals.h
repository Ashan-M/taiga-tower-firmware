#include <Arduino.h>
#define CLK_PIN  33

#define CMD_INIT        0x01
#define CMD_READ_DATA   0x02
#define CMD_LED_ON      0x03
#define CMD_LED_OFF     0x04
#define NUM_PODS 6
#define FLOAT_PIN 34
#define WIFI_LED 2

extern const uint8_t slavePins[NUM_PODS];
extern const uint8_t motorPins[NUM_PODS];
extern const uint8_t lightChannels[NUM_PODS];
extern const int numSlaves;

extern bool wifi;
extern bool deviceActive;
extern bool firebase;

extern unsigned long lastPoll;
extern const int pollInterval;

extern bool deviceActive;
extern bool globalLightCmd;
extern bool globalSleepMode;
extern bool globalPumpCmd;

// struct Pod {
//     bool active = false;

//     char podID[20];
//     char podName[10];
//     bool hardwareConnected = false;

//     int slavePin;
//     int pumpPin;
//     int pwmChannel;

//     String plantId;

//     float targetMoisture = 0;
//     float targetLight = 0;
//     int currentMoisture = 0;

//     bool pumpState = false;
//     int lightLevel = 0;
// };
// extern Pod pods[NUM_PODS];



enum WiFiState {
  WIFI_IDLE,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_AP_MODE
};

extern WiFiState wifiState;

extern unsigned long lastBlink;
extern bool ledState;
