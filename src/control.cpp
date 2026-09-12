#include "main.h"
#include "globals.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

TimerHandle_t pumpTimers[NUM_PODS];
unsigned long lastFloatCheck = 0;
const int floatCheckInterval = 10000;
String lastFloaterState = "";

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

//const uint8_t motorPins[NUM_PODS] = {14, 12, 13, 25, 26, 27};
const uint8_t motorPins[NUM_PODS] = {25, 26, 27, 14, 12, 13};
volatile bool lastMasterLightState = false;
volatile bool lastMasterPumpState = false;


void initLightControl() {
    pwm.begin();
    pwm.setPWMFreq(1000); // 1kHz for AL8861
    Serial.println("Light PWM Initialized");
}

void controlLight(int channel, int brightness) {
    pwm.setPWM(channel, 0, brightness);
    // Serial.println("Set light channel " + String(channel) + " to brightness " + String(brightness));
    // delay(500);
}

void controlWaterPump(uint8_t podNo, bool on) {
    digitalWrite(motorPins[podNo], on);
    // Serial.println("Set water pump " + String(podNo) + " to " + (on));
    // delay(500);
}

void checkFloater() {
  Serial.println("Checking Floater");

    if (millis() - lastFloatCheck > floatCheckInterval) {

        lastFloatCheck = millis();  // ✅ move inside

        int state = digitalRead(FLOAT_PIN);
       // int state = LOW;

        String currentState;

        if (state == HIGH) {
            currentState = "LOW";  // Tank empty
            Serial.println("Water LOW level (Tank Empty)");
        } else {
            currentState = "HIGH"; // Tank full
            Serial.println("Water HIGH level (Tank Full)");
        }

        // ✅ Only update if changed
        if (currentState != lastFloaterState) {

            Serial.println("🔄 Floater changed → updating Firebase");

            lastFloaterState = currentState;
            bool stat = (currentState == "HIGH") ? true : false;

            sendFloaterStat(stat);
        }
    }
}

void updateLED() {
  unsigned long now = millis();

  switch (wifiState) {

    case WIFI_CONNECTED:
      digitalWrite(WIFI_LED, HIGH);
      break;

    case WIFI_CONNECTING:
      if (now - lastBlink > 200) { // fast blink
        ledState = !ledState;
        digitalWrite(WIFI_LED, ledState);
        lastBlink = now;
      }
      break;

    case WIFI_AP_MODE:
      if (now - lastBlink > 1000) { // slow blink
        ledState = !ledState;
        digitalWrite(WIFI_LED, ledState);
        lastBlink = now;
      }
      break;

    default:
      digitalWrite(WIFI_LED, LOW);
      break;
  }
}

void updateMasterControllers(String controller, bool state){
  if(controller == "masterLight"){
    globalLightCmd = state;
    saveMasterControllers("masterLight", state);
    Serial.println("Updating Master Light: " + String(state ? "ON" : "OFF"));
  } else if (controller == "masterPump") { 
    globalPumpCmd = state;
    saveMasterControllers("masterPump", state);
    Serial.println("Updating Master Pump: " + String(state ? "ON" : "OFF"));
  } else if(controller == "sleepMode"){
    globalSleepMode = state;
    saveMasterControllers("sleepMode", state);
    Serial.println("Updating Sleep Mode: " + String(state ? "ON" : "OFF"));
  }
}

void pumpTimerCallback(TimerHandle_t timer){
  uint8_t podNo = (uint8_t)(uintptr_t)pvTimerGetTimerID(timer);
  Serial.printf("Pump timer finished for POD %d\n", podNo);
  controlWaterPump(podNo, false);

}

void setupPumpTimers(){
  for (uint8_t i=0; i<NUM_PODS; i++){
    pumpTimers[i] = xTimerCreate(
      "PumpTimer", pdMS_TO_TICKS(1000), pdFALSE, (void *)(uintptr_t)i, pumpTimerCallback
    );
    Serial.printf("Create pump timers");
    if(pumpTimers[i] == NULL) {
      Serial.printf("Failed to create pump timers");
    }
  }
}

void startPumpTimer(uint8_t podNo, uint32_t durationSeconds){
  TimerHandle_t timer = pumpTimers[podNo];
  if(timer == NULL) {
    return;
  }
  xTimerStop(timer, 0);
  controlWaterPump(podNo, true);
  xTimerChangePeriod(timer, pdMS_TO_TICKS(durationSeconds * 1000), 0);
  xTimerStart(timer, 0);
  Serial.printf("Pump timer started for POD %d\n", podNo);
}
