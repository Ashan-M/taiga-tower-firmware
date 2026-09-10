#include "main.h"
#include "globals.h"
#include "pods.h"
#define THRESHOLD 2

const uint8_t slavePins[NUM_PODS] = {4, 5, 18, 19, 23, 32};
const uint8_t lightChannels[NUM_PODS] = {0,1,2,3,4,5};
char rxBuf[16];
bool globalLightCmd = false;
bool globalSleepMode = false;
bool globalPumpCmd = false;
Pod pods[NUM_PODS];


void initPodStructs() {
    for (int i = 0; i < NUM_PODS; i++) {
        pods[i].slavePin = slavePins[i];
        pods[i].pumpPin = motorPins[i];
        pods[i].pwmChannel = lightChannels[i];

        pinMode(pods[i].pumpPin, OUTPUT);
        digitalWrite(pods[i].pumpPin, LOW);
    }
}

// --- LOW LEVEL BIT BANGING ---

void sendByte(uint8_t data, int activePin) {
  pinMode(activePin, OUTPUT);
  for (int i = 0; i < 8; i++) {
    digitalWrite(activePin, (data & 0x80));
    data <<= 1;
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(50);
    digitalWrite(CLK_PIN, LOW);
    delayMicroseconds(50);
  }
  pinMode(activePin, INPUT_PULLUP); // Release line
}

uint8_t readByte(int activePin) {
  uint8_t value = 0;
  pinMode(activePin, INPUT_PULLUP); 
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(50);
    value <<= 1;
    if (digitalRead(activePin)) value |= 1;
    digitalWrite(CLK_PIN, LOW);
    delayMicroseconds(50);
  }
  return value;
}

// --- PROTOCOL LAYERS ---

void sendString(const char *s, int activePin) {
  while (*s) sendByte(*s++, activePin);
  sendByte('\n', activePin);
}

void readLine(char* buffer, uint8_t maxLen, int activePin) {
  uint8_t idx = 0;
  while (idx < (maxLen - 1)) {
    uint8_t b = readByte(activePin);
    if (b == '\n') break;
    if (b == '\r') continue;
    buffer[idx++] = (char)b;
  }
  buffer[idx] = '\0';
}

bool waitForSlave(uint32_t timeoutMs, int activePin) {
  uint32_t start = millis();
  pinMode(activePin, INPUT_PULLUP);
  while (digitalRead(activePin) == HIGH) {
    if (millis() - start > timeoutMs) return false; 
  }
  digitalWrite(CLK_PIN, HIGH);
  delayMicroseconds(50);
  digitalWrite(CLK_PIN, LOW);
  return true;
}


void initSlave(int slot) {
  int pin = pods[slot].slavePin;
  Serial.printf("\n--- Accessing Slave on Pin %d ---\n", pin);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(100);
  sendByte(CMD_INIT, pin);

  if (waitForSlave(500, pin)) {
    readLine(rxBuf, sizeof(rxBuf), pin);
    
    // Check if the Slave is asking for a new ID
    if (strcmp(rxBuf, "NO_ID") == 0) {
      char newID[16];
      sprintf(newID, "0025-2-%03d", pin);
      sendString(newID, pin); // Send the new ID
      
      // Wait for the Slave to write to EEPROM and echo back the ID
      if (waitForSlave(500, pin)) {
        readLine(rxBuf, sizeof(rxBuf), pin);
        Serial.print("Slave successfully provisioned with ID: ");
        Serial.println(rxBuf);
        strncpy(pods[slot].hardwareID, rxBuf, sizeof(pods[slot].hardwareID) - 1);
        pods[slot].hardwareConnected = true;

      } else {
        Serial.println("Error: Slave timed out during ID saving: ");
        Serial.println(rxBuf);
      }
      
    } else {
      // Slave already had an ID saved
      Serial.print("Slave recognized! ID: ");
      Serial.println(rxBuf);
      strncpy(pods[slot].hardwareID, rxBuf, sizeof(pods[slot].hardwareID) - 1);
      pods[slot].hardwareConnected = true;
    }
  } else {
    Serial.println("Error: Slave Timeout on INIT");
  }
  // pinMode(pin, INPUT_PULLUP);
}



// NEW HIGH-LEVEL COMMAND HANDLER
void executeCommand(uint8_t cmd, bool expectResponse, int activePin) {
  pinMode(activePin, OUTPUT);
  digitalWrite(activePin, LOW);
  delayMicroseconds(100);
  sendByte(cmd, activePin);

  if (expectResponse) {
    if (waitForSlave(500, activePin)) {
      readLine(rxBuf, sizeof(rxBuf), activePin);
      Serial.print("Response: ");
      Serial.println(rxBuf);
    } else {
      Serial.println("Error: Slave Timeout");
    }
  }
  pinMode(activePin, INPUT_PULLUP);
}

void handlePodInit(int i) {

    if (pods[i].active && !pods[i].hardwareConnected) {

        Serial.println("Initializing pod " + String(i+1));

        initSlave(pods[i].slavePin);
        delay(500);
        
        pods[i].hardwareConnected = true;

        Serial.println("Pod " + String(i+1) + " connected");
    }
}

int mapLight(int value) {
    return map(value, 0, 100, 0, 4095);
}

int mapMoisture(int value) {
    return map(value, 0, 100, 310, 770);
}

void readSensor(int i) {
    Serial.println("Reading pin " + String(pods[i].slavePin) + " for pod " + String(i+1));
    executeCommand(CMD_READ_DATA, true, pods[i].slavePin);
    pods[i].currentMoisture = atof(rxBuf);
    pods[i].currentMoisture = mapMoisture(pods[i].currentMoisture);
}



void activateSleepMode() {
    Serial.println("Activating Sleep Mode: Turning off all pods");
    for (int i = 0; i < NUM_PODS; i++) {
        controlLight(pods[i].pwmChannel, 0);
        controlWaterPump(i, LOW);
    }
}

void controlPod(int i) {

    Pod &p = pods[i];

    if (!p.active){
        // Turn off pumpand light for that pod
        controlLight(p.pwmChannel, 0); // turn off light
        controlWaterPump(i, LOW); // turn off pump
        return;
    }

    // ---------- LIGHT ----------
    int desiredLight = 0;

    if (globalLightCmd) {
      desiredLight = mapLight(p.targetLight);
      // Serial.println("Target Light: " + String(p.targetLight));
      controlLight(p.pwmChannel, desiredLight);
         
    }
    else if (!globalLightCmd) {
        desiredLight = 0;
        controlLight(p.pwmChannel, desiredLight);
        
    }

    // ---------- PUMP ----------
    int desiredMoisture = mapMoisture(p.targetMoisture);
    bool pumpOn = (p.currentMoisture > desiredMoisture);

    // Serial.println("Current Moisture: " + String(p.currentMoisture));
    // Serial.println("Target Moisture: " + String(desiredMoisture));

    if (pumpOn) {
        controlWaterPump(i, pumpOn);
        // Serial.println("Pod " + String(i+1) + (pumpOn ? " Pump ON" : " Pump OFF"));
    }
    else{
        controlWaterPump(i, LOW);
    }
    
    // Serial.println("--------------------");
}
